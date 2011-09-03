/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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
#include "symcmp.hh"

#include <cl/cl_msg.hh>

#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

bool matchPlainValuesCore(
        TValMapBidir            valMapping,
        const TValId            v1,
        const TValId            v2)
{
    // left-to-right check
    TValMap &ltr = valMapping[/* ltr */ 0];
    TValMap::iterator iter1 = ltr.find(v1);
    if (iter1 != ltr.end())
        // substitution already defined, check if it applies seamlessly
        return iter1->second == v2;

    // right-to-left check
    TValMap &rtl = valMapping[/* rtl */ 1];
    TValMap::iterator iter2 = rtl.find(v2);
    if (iter2 != rtl.end())
        // substitution already defined, check if it applies seamlessly
        return iter2->second == v1;

    // not found --> define a new substitution
    ltr[v1] = v2;
    rtl[v2] = v1;
    return true;
}

bool matchPlainValues(
        TValMapBidir            valMapping,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            v1,
        const TValId            v2)
{
    if (!checkNonPosValues(v1, v2))
        // null vs. non-null, etc.
        return false;

    else if (v1 <= VAL_NULL)
        // no need to save mapping of special values, they're fixed anyway
        return true;

    const TOffset off1 = sh1.valOffset(v1);
    const TOffset off2 = sh2.valOffset(v2);
    if (off1 != off2)
        // offset mismatch
        return false;

    // check the mapping of roots
    const TValId root1 = sh1.valRoot(v1);
    const TValId root2 = sh2.valRoot(v2);
    return matchPlainValuesCore(valMapping, root1, root2);
}

bool matchUniBlocks(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            root1,
        const TValId            root2)
{
    TUniBlockMap bMap1, bMap2;
    sh1.gatherUniformBlocks(bMap1, root1);
    sh2.gatherUniformBlocks(bMap2, root2);

    if (bMap1.size() != bMap2.size())
        // count of blocks does not match
        return false;

    TUniBlockMap::const_iterator i1 = bMap1.begin();
    TUniBlockMap::const_iterator i2 = bMap2.begin();
    const TUniBlockMap::const_iterator t1 = bMap1.end();

    for (; i1 != t1; ++i1, ++i2) {
        CL_BREAK_IF(i2 == bMap2.end());
        const UniformBlock &bl1 = i1->second;
        const UniformBlock &bl2 = i2->second;
        if (!areUniBlocksEqual(sh1, sh2, bl1, bl2))
            return false;

        CL_BREAK_IF(i1->first != i2->first);
    }

    // full match!
    return true;
}

bool matchRoots(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            root1,
        const TValId            root2,
        const EValueTarget      code)
{
    const int size1 = sh1.valSizeOfTarget(root1);
    const int size2 = sh2.valSizeOfTarget(root2);
    if (size1 != size2)
        // target size mismatch
        return false;

    const bool isProto1 = sh1.valTargetIsProto(root1);
    const bool isProto2 = sh2.valTargetIsProto(root2);
    if (isProto1 != isProto2)
        // prototype vs. shared object while called from areEqual()
        return false;

    if (!matchUniBlocks(sh1, sh2, root1, root2))
        // root canvas mismatch
        return false;

    if (!isAbstract(code))
        // not an abstract object
        return true;

    const EObjKind kind1 = sh1.valTargetKind(root1);
    const EObjKind kind2 = sh2.valTargetKind(root2);
    if (kind1 != kind2)
        // kind of object mismatch
        return false;

    // compare binding fields
    const BindingOff &bf1 = sh1.segBinding(root1);
    const BindingOff &bf2 = sh2.segBinding(root2);
    if (bf1 != bf2)
        return false;

    const unsigned len1 = sh1.segMinLength(root1);
    const unsigned len2 = sh2.segMinLength(root2);
    if (len1 != len2)
        // minimal length mismatch
        return false;

    // abstract objects are equal
    return true;
}

bool cmpValues(
        bool                    *pNeedFollow,
        TValMapBidir            &vMap,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            v1,
        const TValId            v2)
{
    *pNeedFollow = false;
    if (!matchPlainValues(vMap, sh1, sh2, v1, v2))
        return false;

    // check for special values
    const bool isSpecial = (v1 <= 0);
    CL_BREAK_IF(isSpecial && 0 < v2);
    if (isSpecial)
        // already checked by matchPlainValues()/checkNonPosValues()
        return true;

    const TOffset off1 = sh1.valOffset(v1);
    const TOffset off2 = sh2.valOffset(v2);
    if (off1 != off2)
        // value offset mismatch
        return false;

    const EValueTarget code = sh1.valTarget(v1);
    if (code != sh2.valTarget(v2))
        // target kind mismatch
        return false;

    if (VT_CUSTOM == code) {
        // match pair of custom values
        const CustomValue cVal1 = sh1.valUnwrapCustom(v1);
        const CustomValue cVal2 = sh2.valUnwrapCustom(v2);
        return (cVal1 == cVal2);
    }

    if (isPossibleToDeref(code))
        *pNeedFollow = true;
    else
        // no valid target
        return true;

    // match roots
    const TValId root1 = sh1.valRoot(v1);
    const TValId root2 = sh2.valRoot(v2);
    return matchRoots(sh1, sh2, root1, root2, code);
}

typedef WorkList<TValPair> TWorkList;

class ValueComparator {
    private:
        TWorkList       &wl_;
        TValMapBidir    &vMap_;
        SymHeap         &sh1_;
        SymHeap         &sh2_;

    public:
        ValueComparator(
                TWorkList       &wl,
                TValMapBidir    &vMap,
                SymHeap         &sh1,
                SymHeap         &sh2):
            wl_(wl),
            vMap_(vMap),
            sh1_(sh1),
            sh2_(sh2)
        {
        }

        bool operator()(TObjId item[2]) {
            const TValId v1 = sh1_.valueOf(item[0]);
            const TValId v2 = sh2_.valueOf(item[1]);

            bool follow;
            if (!cmpValues(&follow, vMap_, sh1_, sh2_, v1, v2))
                // value mismatch
                return false;

            if (follow) {
                // schedule roots for next wheel
                const TValId root1 = sh1_.valRoot(v1);
                const TValId root2 = sh2_.valRoot(v2);
                wl_.schedule(root1, root2);
            }

            return /* continue */ true;
        }
};

bool dfsCmp(
        TWorkList               &wl,
        TValMapBidir            &vMap,
        SymHeap                 &sh1,
        SymHeap                 &sh2)
{
    // DFS loop
    TWorkList::value_type item;
    while (wl.next(item)) {
        TValId v1, v2;
        boost::tie(v1, v2) = item;

        bool follow;
        if (!cmpValues(&follow, vMap, sh1, sh2, v1, v2))
            // value mismatch
            return false;

        if (!follow)
            // nothing to follow here
            continue;

        // set up a visitor
        SymHeap *const heaps[] = { &sh1, &sh2 };
        TValId roots[] = { v1, v2 };
        ValueComparator visitor(wl, vMap, sh1, sh2);

        // guide it through a pair of root objects
        if (!traverseLiveObjsGeneric<2>(heaps, roots, visitor))
            return false;
    }

    // heaps are equal up to isomorphism
    return true;
}

class VarScheduleVisitor {
    private:
        TWorkList &wl_;

    public:
        VarScheduleVisitor(TWorkList &wl):
            wl_(wl)
        {
        }

        bool operator()(const TValId roots[2]) {
            wl_.schedule(roots[0], roots[1]);
            return /* continue */ true;
        }
};

bool areEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2)
{
    SymHeap &sh1Writable = const_cast<SymHeap &>(sh1);
    SymHeap &sh2Writable = const_cast<SymHeap &>(sh2);

    SymHeap *const heaps[] = {
        &sh1Writable,
        &sh2Writable
    };

    TWorkList wl;
    if (sh1.valLastKnownTypeOfTarget(VAL_ADDR_OF_RET)
            || sh2.valLastKnownTypeOfTarget(VAL_ADDR_OF_RET))
    {
        // schedule return values
        wl.schedule(VAL_ADDR_OF_RET, VAL_ADDR_OF_RET);
    }

    // start with program variables
    VarScheduleVisitor visitor(wl);
    if (!traverseProgramVarsGeneric<0, /* N_SRC */ 2>(heaps, visitor))
        return false;

    // check isomorphism
    TValMapBidir vMap;
    if (!dfsCmp(wl, vMap, sh1Writable, sh2Writable))
        return false;

    // finally match heap predicates
    return sh1.matchPreds(sh2, vMap[0])
        && sh2.matchPreds(sh1, vMap[1]);
}
