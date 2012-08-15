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

#include <boost/foreach.hpp>

bool numFromVal(IR::TInt *pDst, const SymHeapCore &sh, const TValId val) {
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

bool rngFromVal(IR::Range *pDst, const SymHeapCore &sh, const TValId val) {
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

bool stringFromVal(const char **pDst, const SymHeap &sh, const TValId val) {
    if (VT_CUSTOM != sh.valTarget(val))
        // not a custom value
        return false;

    CustomValue cv = sh.valUnwrapCustom(val);
    if (CV_STRING != cv.code())
        // not a string literal
        return false;

    *pDst = cv.str().c_str();
    CL_BREAK_IF(!*pDst);
    return true;
}

const IR::Range& rngFromCustom(const CustomValue &cv) {
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

void moveKnownValueToLeft(
        const SymHeapCore           &sh,
        TValId                      &valA,
        TValId                      &valB)
{
    sortValues(valA, valB);
    if (valA <= 0)
        return;

    const EValueTarget code = sh.valTarget(valA);
    if (isKnownObject(code))
        return;

    const TValId tmp = valA;
    valA = valB;
    valB = tmp;
}

bool valInsideSafeRange(const SymHeapCore &sh, TValId val) {
    const EValueTarget code = sh.valTarget(val);
    if (!isKnownObject(code))
        return false;

    const TSizeRange size = sh.valSizeOfTarget(val);
    return (IR::Int0 < size.lo);
}

bool canWriteDataPtrAt(const SymHeapCore &sh, TValId val) {
    if (!isPossibleToDeref(sh.valTarget(val)))
        return false;

    static TSizeOf ptrSize;
    if (!ptrSize)
        ptrSize = sh.stor().types.dataPtrSizeof();

    const TSizeRange size = sh.valSizeOfTarget(val);
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
        SymHeap                 &dst,
        const SymHeap           &src,
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

void initGlVar(SymHeap &sh, const CVar &cv) {
    CL_BREAK_IF(cv.inst);
    CL_BREAK_IF(isVarAlive(sh, cv));

    SymBackTrace dummyBt(sh.stor());
    SymProc proc(sh, &dummyBt);
    (void) proc.varAt(cv);
}

bool /* anyChange */ redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo,
        const TOffset           offHead)
{
    bool anyChange = false;

    // go through all objects pointing at/inside pointingTo
    ObjList refs;
    sh.pointedBy(refs, pointingTo);
    BOOST_FOREACH(const ObjHandle &obj, refs) {
        if (VAL_INVALID != pointingFrom) {
            const TValId referrerAt = sh.valRoot(obj.placedAt());
            if (pointingFrom != referrerAt)
                // pointed from elsewhere, keep going
                continue;
        }

        // check the current link
        const TValId nowAt = obj.value();
        TValId result;

        const EValueTarget code = sh.valTarget(nowAt);
        if (VT_RANGE == code) {
            // redirect a value with range offset
            IR::Range offRange = sh.valOffsetRange(nowAt);
            offRange.lo -= offHead;
            offRange.hi -= offHead;
            result = sh.valByRange(redirectTo, offRange);
        }
        else {
            // redirect a value with scalar offset
            const TOffset offToRoot = sh.valOffset(nowAt);
            result = sh.valByOffset(redirectTo, offToRoot - offHead);
        }

        // store the redirected value
        obj.setValue(result);
        anyChange = true;
    }

    return anyChange;
}

bool proveNeq(const SymHeapCore &sh, TValId ref, TValId val) {
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
    const EValueTarget code = sh.valTarget(val);
    if (VAL_NULL == ref && (isKnownObject(code) || isGone(code)))
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
