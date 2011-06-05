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

#include "symheap.hh"
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

        case VT_ABSTRACT:
        case VT_INVALID:
        case VT_UNKNOWN:
            break;
    }

    const TValId tmp = valA;
    valA = valB;
    valB = tmp;
}

void getPtrValues(TValList &dst, const SymHeap &sh, TValId at) {
    TObjList ptrs;
    sh.gatherLivePointers(ptrs, at);
    BOOST_FOREACH(const TObjId obj, ptrs) {
        const TValId val = sh.valueOf(obj);
        if (0 < val)
            dst.push_back(val);
    }
}

class PointingObjectsFinder {
    public:
        // we have to use std::set, a vector is not sufficient in all cases
        typedef std::set<TObjId> TResults;

    private:
        TResults results_;

    public:
        const TResults& results() const { return results_; }

        bool operator()(const SymHeap &sh, TObjId obj) {
            const TValId addr = sh.placedAt(obj);
            CL_BREAK_IF(addr <= 0);

            TObjList refs;
            sh.usedBy(refs, addr);
            std::copy(refs.begin(), refs.end(),
                      std::inserter(results_, results_.begin()));

            return /* continue */ true;
        }
};

void redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo)
{
    // go through all objects pointing at/inside pointingTo
    TObjList refs;
    sh.pointedBy(refs, pointingTo);
    BOOST_FOREACH(const TObjId obj, refs) {
        const TValId referrerAt = sh.valRoot(sh.placedAt(obj));
        if (VAL_INVALID != pointingFrom && pointingFrom != referrerAt)
            // pointed from elsewhere, keep going
            continue;

        // check the current link
        const TValId nowAt = sh.valueOf(obj);
        const TOffset offToRoot = sh.valOffset(nowAt);
        CL_BREAK_IF(sh.valOffset(redirectTo));

        // redirect accordingly
        const TValId result = sh.valByOffset(redirectTo, offToRoot);
        sh.objSetValue(obj, result);
    }
}
