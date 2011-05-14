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
        const TValId            v2,
        const bool              symHeapNG)
{
    if (!checkNonPosValues(v1, v2))
        // null vs. non-null, etc.
        return false;

    else if (v1 <= VAL_NULL)
        // no need to save mapping of special values, they're fixed anyway
        return true;

    const bool isValidPtr1 = isPossibleToDeref(sh1.valTarget(v1));
    const bool isValidPtr2 = isPossibleToDeref(sh2.valTarget(v2));
    if (!isValidPtr1 || !isValidPtr2)
        // we need to be careful with magic values
        return matchPlainValuesCore(valMapping, v1, v2);

    const TOffset off1 = sh1.valOffset(v1);
    const TOffset off2 = sh2.valOffset(v2);
    if (off1 != off2)
        // offset mismatch
        return false;

    const TValId root1 = sh1.valRoot(v1);
    const TValId root2 = sh2.valRoot(v2);
    if (!matchPlainValuesCore(valMapping, root1, root2))
        // root mismatch
        return false;

    // TODO: throw this away as soon as symcmp/symjoin is ported to symheap-ng
    return symHeapNG
        || matchPlainValuesCore(valMapping, v1, v2);
}

// FIXME: this needs some cleanup and refactoring
bool matchValues(
        TValMapBidir            &vMap,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            v1,
        const TValId            v2)
{
    if (!matchPlainValues(vMap, sh1, sh2, v1, v2, /* symHeapNG */ true))
        return false;

    // check for special values
    const bool isSpecial = (v1 <= 0);
    CL_BREAK_IF(isSpecial && 0 < v2);
    if (isSpecial)
        // already checked by matchPlainValues()/checkNonPosValues()
        return true;

    // check for unknown values
    const EValueTarget code1 = sh1.valTarget(v1);
    const EValueTarget code2 = sh2.valTarget(v2);
    if (code1 != code2)
        // mismatch in kind of values
        return false;

    if (!isPossibleToDeref(code1) && VT_CUSTOM != code1)
        // do not follow unknown values
        return true;

    // check custom values state
    const int cVal1 = sh1.valGetCustom(v1);
    const int cVal2 = sh2.valGetCustom(v2);
    if ((-1 == cVal1) != (-1 == cVal2))
        return false;

    if (-1 != cVal1)
        // match pair of custom values
        return (cVal1 == cVal2);

    const bool isProto1 = sh1.valTargetIsProto(v1);
    const bool isProto2 = sh2.valTargetIsProto(v2);
    if (isProto1 != isProto2)
        // prototype vs. shared object while called from areEqual()
        return false;

    const TOffset off1 = sh1.valOffset(v1);
    const TOffset off2 = sh2.valOffset(v2);
    if (off1 != off2)
        // value offset mismatch
        return false;

    const EObjKind kind1 = sh1.valTargetKind(v1);
    const EObjKind kind2 = sh2.valTargetKind(v2);
    if (kind1 != kind2)
        // kind of object mismatch
        return false;

    if (OK_CONCRETE != kind1) {
        // compare binding fields
        const BindingOff &bf1 = sh1.segBinding(v1);
        const BindingOff &bf2 = sh2.segBinding(v2);
        if (bf1 != bf2)
            return false;
    }

    // follow all other values
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
    const EValueTarget code1 = sh1.valTarget(v1);
    const EValueTarget code2 = sh2.valTarget(v2);
    if (code1 != code2)
        // target kind mismatch
        return false;

    if (!matchValues(vMap, sh1, sh2, v1, v2))
        // value mismatch
        return false;

    *pNeedFollow = isPossibleToDeref(code1);
    return true;
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
    typename TWorkList::value_type item;
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

bool areEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        TValMap                 *srcToDst,
        TValMap                 *dstToSrc)
{
    CL_BREAK_IF(srcToDst && !srcToDst->empty());
    CL_BREAK_IF(dstToSrc && !dstToSrc->empty());

    // DFS stack
    WorkList<TValPair> wl;

    // value substitution (isomorphism)
    TValMapBidir vMap;

    // FIXME: suboptimal interface of SymHeap::gatherCVars()
    TCVarList cVars1, cVars2;
    sh1.gatherCVars(cVars1);
    sh2.gatherCVars(cVars2);
    if (cVars1 != cVars2)
        // different program variables
        return false;

    SymHeap &sh1Writable = const_cast<SymHeap &>(sh1);
    SymHeap &sh2Writable = const_cast<SymHeap &>(sh2);

    // start with program variables
    BOOST_FOREACH(CVar cv, cVars1) {
        const TValId v1 = sh1Writable.addrOfVar(cv);
        const TValId v2 = sh2Writable.addrOfVar(cv);
        wl.schedule(v1, v2);
    }

    // run DFS
    if (!dfsCmp(wl, vMap, sh1Writable, sh2Writable))
        return false;

    // finally match heap predicates
    if (!sh1.matchPreds(sh2, vMap[0]) || !sh2.matchPreds(sh1, vMap[1]))
        return false;

    if (srcToDst)
        *srcToDst = vMap[/* ltr */ 0];

    if (dstToSrc)
        *dstToSrc = vMap[/* rtl */ 1];

    // full match!
    return true;
}
