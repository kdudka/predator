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

#define SC_DEBUG(...) do {                                                  \
    if (::debugSymCmp)                                                      \
        CL_DEBUG("SymCmp: " << __VA_ARGS__);                                \
} while (0)

#define SC_DUMP_ID(sh, id) \
    "dump_id((SymHeap *)" << &(sh) << ", " << (id) << ")"

#define SC_DUMP_V1_V2(sh1, sh2, v1, v2)                                     \
    ", v1 = " << SC_DUMP_ID(sh1, v1) <<                                     \
    ", v2 = " << SC_DUMP_ID(sh2, v2)

#define SC_DEBUG_VAL_SCHEDULE(who, sh1, sh2, v1, v2)                        \
    SC_DEBUG("+++ " << who                                                  \
            << SC_DUMP_V1_V2(sh1, sh2, v1, v2))

#define SC_DEBUG_VAL_SCHEDULE_BY(who, o1, o2, sh1, sh2, v1, v2)             \
    SC_DEBUG_VAL_SCHEDULE(who << "(" << o1 << ", " << o2 << ")",            \
                          sh1, sh2, v1, v2)

#define SC_DEBUG_VAL_MISMATCH(...)                                          \
    SC_DEBUG("<-- "                                                         \
            << __VA_ARGS__                                                  \
            << SC_DUMP_V1_V2(sh1, sh2, v1, v2))

// set to 'true' if you wonder why SymCmp matches states as it does (noisy)
static bool debugSymCmp = static_cast<bool>(DEBUG_SYMCMP);

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

    const bool isValidPtr1 = SymHeap::isPossibleToDeref(sh1.valTarget(v1));
    const bool isValidPtr2 = SymHeap::isPossibleToDeref(sh2.valTarget(v2));
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

template <class TMapping>
bool matchValues(
        TMapping                &valMapping,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            v1,
        const TValId            v2)
{
    if (!matchPlainValues(valMapping, sh1, sh2, v1, v2, /* symHeapNG */ true))
        return false;

    // check for special values
    const bool isSpecial = (v1 <= 0);
    CL_BREAK_IF(isSpecial && 0 < v2);
    if (isSpecial)
        // already checked by matchPlainValues()/checkNonPosValues()
        return true;

    // check for unknown values
    const EUnknownValue code = sh1.valGetUnknown(v1);
    if (code != sh2.valGetUnknown(v2))
        // mismatch in kind of unknown values
        return false;

    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            break;

        case UV_UNKNOWN:
        case UV_DONT_CARE:
        case UV_UNINITIALIZED:
            // do not follow unknown values
            return true;
    }

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
    if (off1 != off2) {
        SC_DEBUG_VAL_MISMATCH("value offset mismatch");
        return false;
    }

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

template<class TWorkList>
bool digComposite(
        bool                    *isComp,
        TWorkList               &wl,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            v1,
        const TValId            v2)
{
    *isComp = false;
    const TObjId cObj1 = sh1.valGetCompositeObj(v1);
    const TObjId cObj2 = sh2.valGetCompositeObj(v2);

    const bool isComp1 = (OBJ_INVALID != sh1.valGetCompositeObj(v1));
    const bool isComp2 = (OBJ_INVALID != sh2.valGetCompositeObj(v2));
    if (isComp1 != isComp2)
        // scalar vs. composite objects, the heaps can't be equal
        return false;

    if (isComp1)
        *isComp = true;
    else
        return true;

    // we _have_ to jump to the roots at this point as long as we admit
    // to see through multi-level Linux lists
    const TObjId root1 = objRoot(sh1, cObj1);
    const TObjId root2 = objRoot(sh2, cObj2);
    CL_BREAK_IF(OBJ_INVALID == root1 || OBJ_INVALID == root2);

    typedef std::pair<TObjId, TObjId> TItem;
    std::stack<TItem> todo;
    push(todo, root1, root2);
    while (!todo.empty()) {
        TObjId o1, o2;
        boost::tie(o1, o2) = todo.top();
        todo.pop();

        const struct cl_type *clt = sh1.objType(o1);
        if (clt != sh2.objType(o2))
            // type mismatch
            return false;

        if (isComposite(clt)) {
            for (int i = 0; i < clt->item_cnt; ++i) {
                const TObjId sub1 = sh1.subObj(o1, i);
                const TObjId sub2 = sh2.subObj(o2, i);
                CL_BREAK_IF(sub1 < 0 || sub2 < 0);

                push(todo, sub1, sub2);
            }

            continue;
        }

        const TValId val1 = sh1.valueOf(o1);
        const TValId val2 = sh2.valueOf(o2);
        if (wl.schedule(val1, val2)) {
            SC_DEBUG_VAL_SCHEDULE_BY("digComposite", o1, o2,
                                  sh1, sh2, val1, val2);
        }
    }
    return true;
}

template <class TWorkList, class TMapping>
bool dfsCmp(
        TWorkList               &wl,
        TMapping                &valMapping,
        SymHeap                 &sh1,
        SymHeap                 &sh2)
{
    // DFS loop
    typename TWorkList::value_type item;
    while (wl.next(item)) {
        TValId v1, v2;
        boost::tie(v1, v2) = item;

        if (!matchValues(valMapping, sh1, sh2, v1, v2)) {
            SC_DEBUG_VAL_MISMATCH("value mismatch");
            return false;
        }

        bool isComp;
        if (!digComposite(&isComp, wl, sh1, sh2, v1, v2)) {
            SC_DEBUG_VAL_MISMATCH("object composition mismatch");
            return false;
        }

        if (isComp)
            continue;

        const bool follow1 = SymHeap::isPossibleToDeref(sh1.valTarget(v1));
        const bool follow2 = SymHeap::isPossibleToDeref(sh2.valTarget(v2));
        if (follow1 != follow2)
            return false;

        if (!follow1)
            continue;

        const TOffset off = sh1.valOffset(v1);
        CL_BREAK_IF(off != sh2.valOffset(v2));
        if (off < 0) {
            // XXX
            v1 = sh1.valRoot(v1);
            v2 = sh2.valRoot(v2);
            if (wl.schedule(v1, v2))
                SC_DEBUG_VAL_SCHEDULE("offVal", sh1, sh2, v1, v2);
        }

        const TObjId obj1 = sh1.pointsTo(v1);
        const TObjId obj2 = sh2.pointsTo(v2);
        if (!checkNonPosValues(obj1, obj2)) {
            SC_DEBUG("non-matched targets");
            return false;
        }

        v1 = sh1.valueOf(obj1);
        v2 = sh2.valueOf(obj2);

        // schedule values for next wheel
        if (wl.schedule(v1, v2))
            SC_DEBUG_VAL_SCHEDULE_BY("dfsCmp", obj1, obj2, sh1, sh2, v1, v2);
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
    TValMapBidir valMapping;

    // FIXME: suboptimal interface of SymHeap::gatherCVars()
    TCVarList cVars1, cVars2;
    sh1.gatherCVars(cVars1);
    sh2.gatherCVars(cVars2);
    if (cVars1 != cVars2) {
        SC_DEBUG("<-- different program variables");
        return false;
    }

    SymHeap &sh1Writable = const_cast<SymHeap &>(sh1);
    SymHeap &sh2Writable = const_cast<SymHeap &>(sh2);

    // start with program variables
    BOOST_FOREACH(CVar cv, cVars1) {
        const TValId v1 = sh1Writable.addrOfVar(cv);
        const TValId v2 = sh2Writable.addrOfVar(cv);
        if (wl.schedule(v1, v2))
            SC_DEBUG_VAL_SCHEDULE("cVar(" << cv.uid << ")", sh1, sh2, v1, v2);
    }

    // run DFS
    if (!dfsCmp(wl, valMapping, sh1Writable, sh2Writable))
        return false;

    // finally match heap predicates
    if (!sh1.matchPreds(sh2, valMapping[/* ltr */ 0])
            || !sh2.matchPreds(sh1, valMapping[/* rtl */ 1]))
    {
        SC_DEBUG("<-- failed to match heap predicates");
        return false;
    }

    if (srcToDst)
        *srcToDst = valMapping[/* ltr */ 0];

    if (dstToSrc)
        *dstToSrc = valMapping[/* rtl */ 1];

    // full match!
    return true;
}
