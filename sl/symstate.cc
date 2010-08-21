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
#include "symstate.hh"

#include <cl/cl_msg.hh>
#include <cl/code_listener.h>

#include "util.hh"
#include "symutil.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy_if
#include <functional>           // for std::bind
#include <iomanip>
#include <map>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymState implementation
namespace {
    bool checkNonPosValues(int a, int b) {
        if (0 < a && 0 < b)
            // we'll need to properly compare positive values
            return false;

        // non-positive values always have to match, bail out otherwise
        return a != b;
    }

    template <class TSubst>
    bool matchValues(TSubst &subst, TValueId v1, TValueId v2) {
        if (checkNonPosValues(v1, v2))
            // null vs. non-null, etc.
            return false;

        else if (v1 <= VAL_NULL)
            // no need to save mapping of special values, they're fixed anyway
            return true;

        // we need to have the values always in the same order to guarantee
        // the substitution to be bijective ... there used to be a nasty bug
        // at this point, leading to the following nonsense:
        //   [17] = 17
        //   [18] = 18
        //   [35] = 17
        //   [36] = 18
        sortValues(v1, v2);

        typename TSubst::iterator iter = subst.find(v1);
        if (iter != subst.end())
            // substitution already defined, check if it applies seamlessly
            return iter->second == v2;

        // define a new substitution
        subst[v1] = v2;
        return true;
    }

    template <class TSubst, class THeap>
    bool matchValues(TSubst &subst, const THeap &heap1, const THeap &heap2,
                     TValueId v1, TValueId v2)
    {
        if (v1 <= 0 || v2 <= 0)
            // this can't be a pair of custom or unknown values
            return matchValues(subst, v1, v2);

        // do we know the values?
        const EUnknownValue uv1 = heap1.valGetUnknown(v1);
        const EUnknownValue uv2 = heap2.valGetUnknown(v2);
        if (uv1 != uv2)
            // mismatch in kind of unknown values
            return false;

        // FIXME: should we check also the target type-info?
        const int cVal1 = heap1.valGetCustom(0, v1);
        const int cVal2 = heap2.valGetCustom(0, v2);
        if (OBJ_INVALID == cVal1 && OBJ_INVALID == cVal2)
            // this can't be a pair of custom values
            return matchValues(subst, v1, v2);

        // custom and non-custom values can't to be compared
        SE_BREAK_IF(OBJ_INVALID == cVal1 || OBJ_INVALID == cVal2);

        // match custom values
        return (cVal1 == cVal2);
    }

    template <class THeap>
    bool skipValue(const THeap &heap, TValueId value) {
        if (OBJ_INVALID != heap.valGetCompositeObj(value))
            // compare composite objects recursively
            return false;

        if (value <= 0)
            // no need for next wheel (special values already handled)
            return true;

        if (-1 != heap.valGetCustom(0, value))
            // don't follow fnc pointers (and other custom values) by pointsTo()
            return true;

        const EUnknownValue code = heap.valGetUnknown(value);
        switch (code) {
            case UV_KNOWN:
            case UV_ABSTRACT:
                return false;

            default:
                // don't follow uknown values
                return true;
        }
    }

    template<class THeap>
    bool isComposite(const THeap &heap1, const THeap &heap2,
                     TValueId value1, TValueId value2)
    {
        const TObjId cObj1 = heap1.valGetCompositeObj(value1);
        const TObjId cObj2 = heap2.valGetCompositeObj(value2);
        if (OBJ_INVALID == cObj1 && OBJ_INVALID == cObj2)
            return false;

        // types has to match (scalar vs. composite can't be compared)
        SE_BREAK_IF(OBJ_INVALID == cObj1 || OBJ_INVALID == cObj2);

        return true;
    }

    template<class TWL, class THeap>
    bool digComposite(TWL &wl, const THeap &heap1, const THeap &heap2,
                      TValueId value1, TValueId value2)
    {
        const TObjId cObj1 = heap1.valGetCompositeObj(value1);
        const TObjId cObj2 = heap2.valGetCompositeObj(value2);
        // cObj1 and cObj2 are supposed to be valid at this point, see
        // isComposite()

        typedef std::pair<TObjId, TObjId> TItem;
        std::stack<TItem> todo;
        push(todo, cObj1, cObj2);
        while (!todo.empty()) {
            TObjId o1, o2;
            boost::tie(o1, o2) = todo.top();
            todo.pop();

            const struct cl_type *clt = heap1.objType(o1);
            if (clt != heap2.objType(o2))
                // type mismatch
                return false;

            const enum cl_type_e code = (clt)
                ? clt->code
                : /* anonymous object of known size */ CL_TYPE_PTR;     // segments?

            switch (code) {
                case CL_TYPE_PTR: {
                    const TValueId val1 = heap1.valueOf(o1);
                    const TValueId val2 = heap2.valueOf(o2);
                    wl.schedule(val1, val2);
                    break;
                }

                case CL_TYPE_STRUCT:
                    for (int i = 0; i < clt->item_cnt; ++i) {
                        const TObjId sub1 = heap1.subObj(o1, i);
                        const TObjId sub2 = heap2.subObj(o2, i);
                        SE_BREAK_IF(sub1 < 0 || sub2 < 0);

                        push(todo, sub1, sub2);
                    }
                    break;

                case CL_TYPE_INT:
                    break;

                default:
                    // other type of values should be safe to ignore here
                    // but worth to check by a debugger at least once anyway
#if SE_SELF_TEST
                    SE_TRAP;
#endif
                    break;
            }
        }
        return true;
    }

} // namespace 

template <class TWL>
bool cmpAbstractObjects(TWL &wl, const SymHeap &sh1, const SymHeap &sh2,
                        TObjId o1, TObjId o2)
{
    const EObjKind kind = sh1.objKind(o1);
    if (sh2.objKind(o2) != kind)
        // kind of object mismatch
        return false;

    if (OK_CONCRETE == kind || OK_PART == kind)
        // no abstract objects comparison
        return true;

    // compare binding fields
    const SegBindingFields bf = sh1.objBinding(o1);
    if (sh2.objBinding(o2) != bf)
        return false;

    if (OK_HEAD != kind)
        return true;

    // jump to root objects
    o1 = subObjByInvChain(sh1, o1, bf.head);
    o2 = subObjByInvChain(sh2, o2, bf.head);

    // schedule roots for the next wheel
    const TValueId v1 = sh1.placedAt(o1);
    const TValueId v2 = sh2.placedAt(o2);
    wl.schedule(v1, v2);
    return true;
}

// wrapper on top of SymHeapCore::gatherRelatedValues() that filters out
// unused values
template <class TDst>
void gatherRelatedValues(TDst &dst, const SymHeap &sh, TValueId ref) {
    using namespace boost::lambda;

    TDst tmp;
    sh.gatherRelatedValues(tmp, ref);

    // NOTE: beware of the std::cref(sh) usage here -- if you use just 'sh'
    //       instead of std::cref(sh), you decrease the over all performance
    //       10x (!!!), no matter how aggressive optimization you pass to
    //       the compiler in that case
    std::copy_if(tmp.begin(), tmp.end(), std::back_inserter(dst),
            bind(&SymHeapCore::usedByCount, std::cref(sh), _1));
}

template <class TWL, class TSubst>
bool matchPreds(TWL             &wl,
                TSubst          &valSubst,
                const SymHeap   &sh1,
                const SymHeap   &sh2,
                TValueId        v1,
                TValueId        v2)
{
    SymHeap::TContValue rel1, rel2;
    gatherRelatedValues(rel1, sh1, v1);
    gatherRelatedValues(rel2, sh2, v2);

    const unsigned cnt = rel1.size();
    if (rel2.size() != cnt)
        return false;

    // We will probably need to extend the interface of SymHeap in order
    // to compare predicates enough efficiently...
#if 0
    // TODO
    if (cnt)
        TRAP;
#else
    (void) wl;
    (void) valSubst;
#endif

    return true;
}

template <class TWL, class TSubst>
bool dfsCmp(TWL             &wl,
            TSubst          &valSubst,
            const SymHeap   &heap1,
            const SymHeap   &heap2)
{
    // DFS loop
    typename TWL::value_type item;
    while (wl.next(item)) {
        TValueId value1, value2;
        boost::tie(value1, value2) = item;

        if (!matchValues(valSubst, heap1, heap2, value1, value2))
            // value mismatch
            return false;

        if (!matchPreds(wl, valSubst, heap1, heap2, value1, value2))
            // predicate mismatch
            return false;

        if (skipValue(heap1, value1))
            // no need for next wheel
            continue;

        if (isComposite(heap1, heap2, value1, value2)) {
            if (!digComposite(wl, heap1, heap2, value1, value2))
                // object type mismatch (something nasty in the analyzed code)
                return false;

            // compare composite objects recursively
            continue;
        }

        const TObjId obj1 = heap1.pointsTo(value1);
        const TObjId obj2 = heap2.pointsTo(value2);
        if (checkNonPosValues(obj1, obj2))
            // variable mismatch
            return false;

        if (!cmpAbstractObjects(wl, heap1, heap2, obj1, obj2))
            // abstract objects are not equeal
            return false;

        value1 = heap1.valueOf(obj1);
        value2 = heap2.valueOf(obj2);

        // schedule values for next wheel
        wl.schedule(value1, value2);
    }

    // heaps are equal (isomorphism)
    return true;
}

bool operator== (const SymHeap &heap1, const SymHeap &heap2) {
    // DFS stack
    typedef std::pair<TValueId, TValueId> TValuePair;
    WorkList<TValuePair> wl;

    // value substitution (isomorphism)
    typedef std::map<TValueId, TValueId> TSubst;
    TSubst valSubst;

    // FIXME: suboptimal interface of SymHeap::gatherCVars()
    SymHeap::TContCVar cVars1, cVars2;
    heap1.gatherCVars(cVars1);
    heap1.gatherCVars(cVars2);
    if (cVars1.size() != cVars2.size())
        // different count of program variables
        // --> no chance the heaps are equal up to isomorphism
        return false;

    // merge cVars
    std::set<CVar> all;
    std::copy(cVars1.begin(), cVars1.end(), std::inserter(all, all.begin()));
    std::copy(cVars2.begin(), cVars2.end(), std::inserter(all, all.begin()));

    BOOST_FOREACH(CVar cv, all) {
        const TObjId var1 = heap1.objByCVar(cv);
        const TObjId var2 = heap2.objByCVar(cv);
        if (var1 < 0 || var2 < 0)
            // static variable mismatch
            return false;

        // retrieve values of static variables
        const TValueId value1 = heap1.valueOf(var1);
        const TValueId value2 = heap2.valueOf(var2);

        // schedule for DFS
        wl.schedule(value1, value2);
    }

    // run DFS
    return dfsCmp(wl, valSubst, heap1, heap2);
}

int SymState::lookup(const SymHeap &heap) const {
    const int cnt = this->size();
    for(int idx = 0; idx < cnt; ++idx) {
        if (heap == heaps_[idx])
            return idx;
    }

    // not found
    return -1;
}

void SymState::insert(const SymHeap &heap) {
    BOOST_FOREACH(const SymHeap &current, heaps_) {
        // TODO: check for entailment instead
        if (heap == current)
            return;
    }

    // add given heap to union
    this->insertNew(heap);
}

void SymState::insert(const SymState &huni) {
    BOOST_FOREACH(const SymHeap &current, huni) {
        this->insert(current);
    }
}

// vim: tw=80
