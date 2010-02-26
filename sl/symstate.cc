/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "symstate.hh"

#include "cl_private.hh"
#include "util.hh"
#include "worklist.hh"

#include <map>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymHeapUnion implementation
namespace {
    bool checkNonPosValues(int a, int b) {
        if (0 < a && 0 < b)
            // we'll need to properly compare positive values
            return false;

        // non-positive values always have to match, bail out otherwise
        return a != b;
    }

    template <class TSubst>
    bool matchValues(TSubst &subst, int v1, int v2) {
        if (checkNonPosValues(v1, v2))
            // null vs. non-null, etc.
            return false;

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
                     int v1, int v2)
    {
        if (v1 <= 0 || v2 <= 0)
            // this can't be a pair of custom or unknown values
            return matchValues(subst, v1, v2);

        // do we know the values?
        const EUnknownValue uv1 = heap1.valGetUnknown(v1);
        const EUnknownValue uv2 = heap2.valGetUnknown(v2);
        if (UV_KNOWN != uv1 || UV_KNOWN != uv2)
            // compare types of uknown values
            return (uv1 == uv2);

        const int cVal1 = heap1.valGetCustom(0, v1);
        const int cVal2 = heap2.valGetCustom(0, v2);
        if (OBJ_INVALID == cVal1 && OBJ_INVALID == cVal2)
            // this can't be a pair of custom values
            return matchValues(subst, v1, v2);

        if (OBJ_INVALID == cVal1 || OBJ_INVALID == cVal2)
            // custom and non-custom values are going to be compared
            TRAP;

        // match custom values
        return (cVal1 == cVal2);
    }

    template <class THeap>
    bool skipValue(const THeap &heap, int value) {
        if (OBJ_INVALID != heap.valGetCompositeObj(value))
            // compare composite objects recursively
            return false;

        if (value <= 0)
            // no need for next wheel (special values already handled)
            return true;

        if (VAL_INVALID != heap.valGetCustom(0, value))
            // don't follow fnc pointers (and other custom values) by pointsTo()
            return true;

        if (UV_KNOWN != heap.valGetUnknown(value))
            // don't follow uknown values
            return true;

        return false;
    }

    template<class TWL, class THeap>
    bool digComposite(TWL &wl, const THeap &heap1, const THeap &heap2,
                      int value1, int value2)
    {
        const int cObj1 = heap1.valGetCompositeObj(value1);
        const int cObj2 = heap2.valGetCompositeObj(value2);
        if (OBJ_INVALID == cObj1 && OBJ_INVALID == cObj2)
            return false;

        if (OBJ_INVALID == cObj1 || OBJ_INVALID == cObj2)
            // type mismatch (scalar vs. composite ought to be compared)
            TRAP;

        // FIXME: the following block of code is sort of copy-pasted
        //        from symexec.cc
        typedef std::pair<int /* obj */, int /* obj */> TItem;
        std::stack<TItem> todo;
        push(todo, cObj1, cObj2);
        while (!todo.empty()) {
            int o1, o2;
            boost::tie(o1, o2) = todo.top();
            todo.pop();

            const struct cl_type *clt = heap1.objType(o1);
            if (clt != heap2.objType(o2))
                // type mismatch
                TRAP;

            const enum cl_type_e code = (clt)
                ? clt->code
                : /* anonymous object of known size */ CL_TYPE_PTR;

            switch (code) {
                case CL_TYPE_PTR: {
                    const int val1 = heap1.valueOf(o1);
                    const int val2 = heap2.valueOf(o2);
                    wl.schedule(val1, val2);
                    break;
                }

                case CL_TYPE_STRUCT:
                    for (int i = 0; i < clt->item_cnt; ++i) {
                        const int sub1 = heap1.subVar(o1, i);
                        const int sub2 = heap2.subVar(o2, i);
                        if (sub1 < 0 || sub2 < 0)
                            TRAP;

                        push(todo, sub1, sub2);
                    }
                    break;

                case CL_TYPE_INT:
                    break;

                default:
                    // other type of values should be safe to ignore here
                    // but worth to check by a debugger at least once anyway
                    TRAP;
            }
        }
        return true;
    }
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
        int value1, value2;
        boost::tie(value1, value2) = item;

        if (digComposite(wl, heap1, heap2, value1, value2))
            // compare composite objects recursively
            continue;

        // FIXME: this appears twice because of digComposite
        if (!matchValues(valSubst, heap1, heap2, value1, value2))
            // value mismatch
            return false;

        // FIXME: this appears twice because of digComposite
        if (skipValue(heap1, value1))
            // do not follow unknown value
            continue;

        // TODO: distinguish among SLS and single dynamic variables here
        const int obj1 = heap1.pointsTo(value1);
        const int obj2 = heap2.pointsTo(value2);
        if (checkNonPosValues(obj1, obj2))
            // variable mismatch
            return false;

        value1 = heap1.valueOf(obj1);
        value2 = heap2.valueOf(obj2);
        if (!matchValues(valSubst, heap1, heap2, value1, value2))
            // value mismatch
            return false;

        if (skipValue(heap1, value1))
            // no need for next wheel
            continue;

        // schedule values for next wheel
        wl.schedule(value1, value2);
    }

    // heaps are equal (isomorphism)
    return true;
}

bool operator== (const SymHeap &heap1, const SymHeap &heap2) {
    // DFS stack
    typedef std::pair<int, int> TValuePair;
    WorkList<TValuePair> wl;

    // value substitution (isomorphism)
    typedef std::map<int, int> TSubst;
    TSubst valSubst;

    // NOTE: we do not check cVars themselves among heaps
    // they are *supposed* to be the same
    SymHeap::TCont cVars;
    heap1.gatherCVars(cVars);
    BOOST_FOREACH(int uid, cVars) {
        const int var1 = heap1.varByCVar(uid);
        const int var2 = heap2.varByCVar(uid);
        if (var1 < 0 || var2 < 0)
            // heap corruption detected
            TRAP;

        // retrieve values of static variables
        const int value1 = heap1.valueOf(var1);
        const int value2 = heap2.valueOf(var2);
        if (!matchValues(valSubst, heap1, heap2, value1, value2))
            // value mismatch, bail out now
            return false;

        if (skipValue(heap1, value1))
            // no need for next wheel
            continue;

        // schedule for DFS
        wl.schedule(value1, value2);
    }

    // bad luck, we need to run DFS
    return dfsCmp(wl, valSubst, heap1, heap2);
}

void SymHeapUnion::insert(const SymHeap &heap) {
    // FIXME: not very efficient implementation of union :-)
    // TODO: implement the container as either hash or tree data structure
    BOOST_FOREACH(const SymHeap &current, heaps_) {
        // TODO: check for entailment instead
        if (heap == current)
            return;
    }

    // add given heap to union
    heaps_.push_back(heap);
}

void SymHeapUnion::insert(const SymHeapUnion &huni) {
    BOOST_FOREACH(const SymHeap &current, huni) {
        this->insert(current);
    }
}
