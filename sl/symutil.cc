/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "symutil.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symbt.hh"
#include "symheap.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <algorithm>                /* for std::find() */

#include <boost/foreach.hpp>

bool numFromVal(IR::TInt *pDst, const SymHeapCore &sh, const TValId val)
{
    switch (val) {
        case VAL_NULL:
            *pDst = 0L;
            return true;

        case VAL_TRUE:
            *pDst = 1L;
            return true;

        default:
            if (VT_CUSTOM == sh.valTarget(val))
                break;

            // not a custom value, which integral constants are supposed to be
            return false;
    }

    const CustomValue cv = sh.valUnwrapCustom(val);
    if (CV_INT_RANGE != cv.code())
        return false;

    const IR::Range &rng = cv.rng();
    if (!isSingular(rng))
        // we are asked to return a scalar, but only integral range is available
        return false;

    *pDst = rng.lo;
    return true;
}

bool rngFromVal(IR::Range *pDst, const SymHeapCore &sh, const TValId val)
{
    IR::TInt num;
    if (numFromVal(&num, sh, val)) {
        // a single number
        *pDst = IR::rngFromNum(num);
        return true;
    }

    if (VAL_NULL == sh.valRoot(val)) {
        // extract offset range of a NULL value
        *pDst = sh.valOffsetRange(val);
        return true;
    }

    if (VT_CUSTOM != sh.valTarget(val))
        // not a custom value
        return false;

    CustomValue cv = sh.valUnwrapCustom(val);
    if (CV_INT_RANGE != cv.code())
        // not an integral range
        return false;

    *pDst = cv.rng();
    return true;
}

bool anyRangeFromVal(
        IR::Range                   *pDst,
        const SymHeap               &sh,
        const TValId                 val)
{
    // try to extract an integral range
    if (rngFromVal(pDst, sh, val))
        return true;

    const EValueTarget code = sh.valTarget(val);
    if (isAnyDataArea(code)) {
        // extract offset range
        *pDst = sh.valOffsetRange(val);
        return true;
    }

    // FIXME: this way we are asking for overflow (build vs. host arch mismatch)
    if (VT_UNKNOWN == code) {
        *pDst = IR::FullRange;
        return true;
    }

    // there is no range we could extract
    return false;
}

bool stringFromVal(std::string *pDst, const SymHeap &sh, const TValId val)
{
    if (VT_CUSTOM != sh.valTarget(val))
        // not a custom value
        return false;

    CustomValue cv = sh.valUnwrapCustom(val);
    if (CV_STRING != cv.code())
        // not a string literal
        return false;

    *pDst = cv.str();
    return true;
}

const IR::Range& rngFromCustom(const CustomValue &cv)
{
    const ECustomValue code = cv.code();
    switch (code) {
        case CV_INT_RANGE:
            return cv.rng();
            break;

        default:
            CL_BREAK_IF("invalid call of rngFromVal()");
            return IR::FullRange;
    }
}

TSizeRange valSizeOfTarget(const SymHeapCore &sh, const TValId at)
{
    if (!isPossibleToDeref(sh, at))
        // no writable target around here
        return IR::rngFromNum(IR::Int0);

    const IR::Range off = sh.valOffsetRange(at);
    if (off.lo < 0)
        // if the offset can be negative, we can safely write nothing
        return IR::rngFromNum(IR::Int0);

    const TObjId obj = sh.objByAddr(at);
    if (!sh.isValid(obj))
        // we cannot safely write to an invalid object
        return IR::rngFromNum(IR::Int0);

    const IR::Range size = sh.objSize(obj);
    return size - off;
}

bool canPointToFront(const ETargetSpecifier ts)
{
    switch (ts) {
        case TS_REGION:
        case TS_FIRST:
            return true;

        case TS_INVALID:
        case TS_LAST:
        case TS_ALL:
            break;
    }

    return false;
}

bool canPointToBack(const ETargetSpecifier ts)
{
    switch (ts) {
        case TS_REGION:
        case TS_LAST:
            return true;

        case TS_INVALID:
        case TS_FIRST:
        case TS_ALL:
            break;
    }

    return false;
}

bool compareIntRanges(
        bool                        *pDst,
        const enum cl_binop_e       code,
        const IR::Range             &range1,
        const IR::Range             &range2)
{
    CmpOpTraits ct;
    if (!describeCmpOp(&ct, code))
        return false;

    if (isAligned(range1) || isAligned(range2)) {
        CL_DEBUG("compareIntRanges() does not support alignment yet");
        return false;
    }

    // check for interval overlapping (strict)
    const bool ltr = (range1.hi < range2.lo);
    const bool rtl = (range2.hi < range1.lo);

    if (ct.preserveEq && ct.preserveNeq) {
        // either == or !=

        if (ltr || rtl) {
            // no overlaps on the given intervals
            *pDst = ct.negative;
            return true;
        }

        if (isSingular(range1) && isSingular(range2)) {
            // we got two integral constants and both are equal
            CL_BREAK_IF(range1.lo != range2.hi);
            *pDst = !ct.negative;
            return true;
        }

        // we got something ambiguous
        return false;
    }

    if (ct.negative) {
        // either < or >

        if ((ltr     && ct.leftToRight) || (rtl     && ct.rightToLeft)) {
            *pDst = true;
            return true;
        }
    }
    else {
        // either <= or >=

        if ((rtl     && ct.leftToRight) || (ltr     && ct.rightToLeft)) {
            *pDst = false;
            return true;
        }
    }

    // check for interval overlapping (weak)
    const bool ltrWeak = (range1.hi <= range2.lo);
    const bool rtlWeak = (range2.hi <= range1.lo);

    if (ct.negative) {
        // either < or >

        if ((rtlWeak && ct.leftToRight) || (ltrWeak && ct.rightToLeft)) {
            *pDst = false;
            return true;
        }
    }
    else {
        // either <= or >=

        if ((ltrWeak && ct.leftToRight) || (rtlWeak && ct.rightToLeft)) {
            *pDst = true;
            return true;
        }
    }

    // we got something ambiguous
    return false;
}

bool isKnownObjectAt(
        const SymHeap              &sh,
        const TValId                val,
        const bool                  allowInvalid = false)
{
    const EValueTarget code = sh.valTarget(val);
    if (VT_RANGE == code)
        // address with offset ranges are not allowed to be dreferenced for now
        return false;

    const TObjId obj = sh.objByAddr(val);
    if (allowInvalid) {
        if (OBJ_INVALID == obj)
            return false;
    }
    else {
        if (!isPossibleToDeref(sh, val))
            return false;
    }

    return !isAbstractObject(sh, obj);
}

void moveKnownValueToLeft(
        const SymHeap               &sh,
        TValId                      &valA,
        TValId                      &valB)
{
    sortValues(valA, valB);
    if (valA <= 0)
        return;

    if (isKnownObjectAt(sh, valA))
        return;

    const TValId tmp = valA;
    valA = valB;
    valB = tmp;
}

bool valInsideSafeRange(const SymHeap &sh, TValId val)
{
    if (!isKnownObjectAt(sh, val))
        return false;

    const TSizeRange size = valSizeOfTarget(sh, val);
    return (IR::Int0 < size.lo);
}

bool canWriteDataPtrAt(const SymHeapCore &sh, TValId val)
{
    if (!isPossibleToDeref(sh, val))
        return false;

    static TSizeOf ptrSize;
    if (!ptrSize)
        ptrSize = sh.stor().types.dataPtrSizeof();

    const TSizeRange size = valSizeOfTarget(sh, val);
    return (ptrSize <= size.lo);
}

bool translateValId(
        TValId                  *pVal,
        SymHeapCore             &dst,
        const SymHeapCore       &src,
        const TValMap           &valMap)
{
    const TValId valSrc = *pVal;
    if (valSrc <= VAL_NULL)
        // special values always match, no need for mapping
        return true;

    const TValId rootSrc = src.valRoot(valSrc);
    const TValId rootDst = roMapLookup(valMap, rootSrc);
    if (VAL_INVALID == rootDst)
        // rootSrc not found in valMap
        return false;

    if (rootSrc == valSrc) {
        // no offset used
        *pVal = rootDst;
    }
    else {
        // translate the lookup result by the original offset
        const IR::Range &off = src.valOffsetRange(valSrc);
        *pVal = dst.valByRange(rootDst, off);
    }

    // match
    return true;
}

TValId translateValProto(
        SymHeapCore             &dst,
        const SymHeapCore       &src,
        const TValId             valProto)
{
    if (valProto <= 0)
        // do not translate special values
        return valProto;

    // read properties from src
    const EValueTarget code = src.valTarget(valProto);
    const EValueOrigin origin = src.valOrigin(valProto);

    // create an equivalent unknown value in dst
    CL_BREAK_IF(VT_UNKNOWN != code);
    return dst.valCreate(code, origin);
}

void initGlVar(SymHeap &sh, const CVar &cv)
{
    CL_BREAK_IF(cv.inst);

    const bool alreadyAlive = isVarAlive(sh, cv);

    SymBackTrace dummyBt(sh.stor());
    SymProc proc(sh, &dummyBt);
    (void) proc.objByVar(cv, /* initOnly */ alreadyAlive);
}

bool /* anyChange */ redirectRefs(
        SymHeap                &sh,
        const TObjId            pointingFrom,
        const TObjId            pointingTo,
        const ETargetSpecifier  pointingWith,
        const TObjId            redirectTo,
        const ETargetSpecifier  redirectWith,
        const TOffset           shiftBy)
{
    bool anyChange = false;

    // go through all objects pointing at/inside pointingTo
    FldList refs;
    sh.pointedBy(refs, pointingTo);
    BOOST_FOREACH(const FldHandle &fld, refs) {
        if (OBJ_INVALID != pointingFrom) {
            const TObjId refObj = fld.obj();
            if (pointingFrom != refObj)
                // pointed from elsewhere, keep going
                continue;
        }

        // check the current link
        const TValId nowAt = fld.value();

        ETargetSpecifier ts = sh.targetSpec(nowAt);
        if (TS_INVALID != pointingWith && pointingWith != ts)
            // target specifier mismatch
            continue;

        if (TS_INVALID != redirectWith)
            // use the given target specifier
            ts = redirectWith;

        // first redirect the target
        const TValId baseAddr = sh.addrOfTarget(redirectTo, ts);
        TValId result;

        const EValueTarget code = sh.valTarget(nowAt);
        if (VT_RANGE == code) {
            // shift the base address by range offset
            IR::Range offRange = sh.valOffsetRange(nowAt);
            offRange.lo += shiftBy;
            offRange.hi += shiftBy;
            result = sh.valByRange(baseAddr, offRange);
        }
        else {
            // shift the base address by calar offset
            const TOffset offToRoot = sh.valOffset(nowAt);
            result = sh.valByOffset(baseAddr, offToRoot + shiftBy);
        }

        // store the redirected value
        fld.setValue(result);
        anyChange = true;
    }

    return anyChange;
}

void redirectRefsNotFrom(
        SymHeap                &sh,
        const TObjSet          &pointingNotFrom,
        const TObjId            pointingTo,
        const TObjId            redirectTo,
        const ETargetSpecifier  redirectWith,
        bool                  (*tsFilter)(ETargetSpecifier))
{
    // go through all objects pointing at/inside pointingTo
    FldList refs;
    sh.pointedBy(refs, pointingTo);
    BOOST_FOREACH(const FldHandle &fld, refs) {
        const TObjId refObj = fld.obj();
        if (hasKey(pointingNotFrom, refObj))
            continue;

        const TValId nowAt = fld.value();

        if (tsFilter) {
            const ETargetSpecifier ts = sh.targetSpec(nowAt);
            if (!tsFilter(ts))
                continue;
        }

        // resolve the base address
        const TValId baseAddr = sh.addrOfTarget(redirectTo, redirectWith);

        // shift the base address by the offset range (if any)
        const IR::Range off = sh.valOffsetRange(nowAt);
        const TValId result = sh.valByRange(baseAddr, off);

        // store the redirected value
        fld.setValue(result);
    }
}

void transferOutgoingEdges(
        SymHeap                &sh,
        const TObjId            ofObj,
        const TObjId            toObj)
{
    CL_BREAK_IF(sh.objSize(ofObj) != sh.objSize(toObj));

    FldList fields;
    sh.gatherLiveFields(fields, ofObj);
    BOOST_FOREACH(const FldHandle &fldOld, fields) {
        const TValId val = fldOld.value();
        const FldHandle fldNew(sh, toObj, fldOld.type(), fldOld.offset());
        fldNew.setValue(val);
    }
}

bool proveNeq(const SymHeap &sh, TValId ref, TValId val)
{
    // check for invalid values
    if (VAL_INVALID == ref || VAL_INVALID == val)
        return false;

    // check for identical values
    if (ref == val)
        return false;

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(sh, ref, val);

    // check for known bool values
    // NOTE: only an optimization to eliminate unnecessary calls of rngFromVal()
    if (VAL_TRUE == ref)
        return (VAL_FALSE == val);

    // we presume (0 <= ref) and (0 < val) at this point
    if (VAL_NULL == ref && isKnownObjectAt(sh, val, /* allowInvalid */ true))
        // all addresses of objects have to be non-zero
        return true;

    if (valInsideSafeRange(sh, ref) && valInsideSafeRange(sh, val))
        // NOTE: we know (ref != val) at this point, look above
        return true;

    IR::Range rng1, rng2;
    if (rngFromVal(&rng1, sh, ref) && rngFromVal(&rng2, sh, val)) {
        // both values are integral ranges (
        bool result;
        return (compareIntRanges(&result, CL_BINOP_NE, rng1, rng2) && result);
    }

    // check for an explicit Neq predicate
    return sh.chkNeq(ref, val);
}
