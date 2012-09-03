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

bool matchOffsets(
        const SymHeapCore       &sh1,
        const SymHeapCore       &sh2,
        const TValId            v1,
        const TValId            v2)
{
    const EValueTarget code1 = sh1.valTarget(v1);
    const EValueTarget code2 = sh2.valTarget(v2);

    const bool isRange = (VT_RANGE == code1);
    if (isRange != (VT_RANGE == code2))
        // range vs. scalar offset
        return false;

    if (isRange) {
        // compare offset ranges
        const IR::Range offRange1 = sh1.valOffsetRange(v1);
        const IR::Range offRange2 = sh2.valOffsetRange(v2);
        return (offRange1 == offRange2);
    }
    else {
        // compare scalar offset
        const TOffset off1 = sh1.valOffset(v1);
        const TOffset off2 = sh2.valOffset(v2);
        return (off1 == off2);
    }
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

    if (!matchOffsets(sh1, sh2, v1, v2))
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

    SymHeap &sh1Writable = const_cast<SymHeap &>(sh1);
    SymHeap &sh2Writable = const_cast<SymHeap &>(sh2);

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap1) {
        UniformBlock bl2(item.second);
        bl2.tplValue = translateValProto(sh2Writable, sh1, bl2.tplValue);

        TUniBlockMap cov2;
        if (!sh2.findCoveringUniBlocks(&cov2, root2, bl2))
            return false;
    }

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap2) {
        UniformBlock bl1(item.second);
        bl1.tplValue = translateValProto(sh1Writable, sh2, bl1.tplValue);

        TUniBlockMap cov1;
        if (!sh1.findCoveringUniBlocks(&cov1, root1, bl1))
            return false;
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
    const TSizeRange size1 = sh1.valSizeOfTarget(root1);
    const TSizeRange size2 = sh2.valSizeOfTarget(root2);
    if (size1 != size2)
        // target size mismatch
        return false;

    const TProtoLevel level1 = sh1.valTargetProtoLevel(root1);
    const TProtoLevel level2 = sh2.valTargetProtoLevel(root2);
    if (level1 != level2)
        // prototype level mismatch
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

    const TMinLen len1 = sh1.segMinLength(root1);
    const TMinLen len2 = sh2.segMinLength(root2);
    if (len1 != len2)
        // minimal length mismatch
        return false;

    if (OK_OBJ_OR_NULL == kind1 /* == kind2 */)
        // this kind has no binding
        return true;

    // compare binding fields
    const BindingOff &bf1 = sh1.segBinding(root1);
    const BindingOff &bf2 = sh2.segBinding(root2);
    return (bf1 == bf2);
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

    if (!matchOffsets(sh1, sh2, v1, v2))
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

typedef std::queue<TValPair>                        TSched;
typedef WorkList<TValPair,TSched>                   TWorkList;

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

        bool operator()(ObjHandle item[2]) {
            const TValId v1 = item[0].value();
            const TValId v2 = item[1].value();

            bool follow;
            if (!cmpValues(&follow, vMap_, sh1_, sh2_, v1, v2))
                // value mismatch
                return false;

            if (follow) {
                // schedule roots for next wheel
                const TValId root1 = sh1_.valRoot(v1);
                const TValId root2 = sh2_.valRoot(v2);
                wl_.schedule(TValPair(root1, root2));
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
            const TValPair vp(roots[0], roots[1]);
            wl_.schedule(vp);
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
        const TValPair vp(VAL_ADDR_OF_RET, VAL_ADDR_OF_RET);
        wl.schedule(vp);
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
