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

#include <cl/storage.hh>

#include "symbt.hh"
#include "symheap.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <boost/foreach.hpp>

bool numFromVal(long *pDst, const SymHeap &sh, const TValId val) {
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

    CustomValue cv = sh.valUnwrapCustom(val);
    if (CV_INT != cv.code)
        return false;

    *pDst = cv.data.num;
    return true;
}

bool rangeFromVal(IntRange *pDst, const SymHeap &sh, const TValId val) {
    long num;
    if (numFromVal(&num, sh, val)) {
        // a single number
        pDst->lo = num;
        pDst->hi = num;
        return true;
    }

    if (VT_CUSTOM != sh.valTarget(val))
        // not a custom value
        return false;

    CustomValue cv = sh.valUnwrapCustom(val);
    if (CV_INT_RANGE != cv.code)
        // not an integral range
        return false;

    *pDst = cv.data.rng;
    return true;
}

bool anyRangeFromVal(
        IntRange                    *pDst,
        const SymHeap               &sh,
        const TValId                 val)
{
    const EValueTarget code = sh.valTarget(val);
    if (VT_CUSTOM == code)
        // try to extract an integral range
        return rangeFromVal(pDst, sh, val);

    if (VAL_NULL == val || isAnyDataArea(code)) {
        // extract offset range
        *pDst = sh.valOffsetRange(val);
        return true;
    }

    // FIXME: this way we are asking for overflow (build vs. host arch mismatch)
    if (VT_UNKNOWN == code) {
        *pDst = IntRangeDomain;
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
    if (CV_STRING != cv.code)
        // not a string literal
        return false;

    *pDst = cv.data.str;
    CL_BREAK_IF(!*pDst);
    return true;
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
    switch (code) {
        case VT_STATIC:
        case VT_ON_STACK:
        case VT_ON_HEAP:
        case VT_CUSTOM:
        case VT_COMPOSITE:
        case VT_LOST:
        case VT_DELETED:
            return;

        case VT_RANGE:
        case VT_ABSTRACT:
        case VT_INVALID:
        case VT_UNKNOWN:
            break;
    }

    const TValId tmp = valA;
    valA = valB;
    valB = tmp;
}

bool valInsideSafeRange(const SymHeapCore &sh, TValId val) {
    return isKnownObject(sh.valTarget(val))
        && (0 < sh.valSizeOfTarget(val));
}

bool canWriteDataPtrAt(const SymHeapCore &sh, TValId val) {
    if (!isPossibleToDeref(sh.valTarget(val)))
        return false;

    static TOffset ptrSize;
    if (!ptrSize)
        ptrSize = sh.stor().types.dataPtrSizeof();

    return (ptrSize <= sh.valSizeOfTarget(val));
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
        const IntRange &off = src.valOffsetRange(valSrc);
        *pVal = dst.valByRange(rootDst, off);
    }

    // match
    return true;
}

void translateValProto(
        TValId                  *pValProto,
        SymHeap                 &dst,
        const SymHeap           &src)
{
    if (*pValProto <= 0)
        // do not translate special values
        return;

    // read properties from src
    const EValueTarget code = src.valTarget(*pValProto);
    const EValueOrigin origin = src.valOrigin(*pValProto);

    // create an equivalent unknown value in dst
    CL_BREAK_IF(VT_UNKNOWN != code);
    *pValProto = dst.valCreate(code, origin);
}

void initGlVar(SymHeap &sh, const CVar &cv) {
    CL_BREAK_IF(cv.inst);
    CL_BREAK_IF(isVarAlive(sh, cv));

    SymBackTrace dummyBt(sh.stor());
    SymProc proc(sh, &dummyBt);
    (void) proc.varAt(cv);
}

void getPtrValues(TValList &dst, SymHeap &sh, TValId at) {
    ObjList ptrs;
    sh.gatherLivePointers(ptrs, at);
    BOOST_FOREACH(const ObjHandle &obj, ptrs) {
        const TValId val = obj.value();
        if (0 < val)
            dst.push_back(val);
    }
}

void redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo,
        const TOffset           offHead)
{
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
            IntRange offRange = sh.valOffsetRange(nowAt);
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
    }
}
