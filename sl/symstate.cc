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

#include "symstate.hh"

#include "cl_private.hh"

#include <map>
#include <set>
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

        typename TSubst::iterator iter = subst.find(v1);
        if (iter != subst.end())
            // substitution already defined, check if it applies seamlessly
            return iter->second == v2;

        // define a new substitution
        subst[v1] = v2;
        return true;
    }

    template <class THeap>
    bool skipValue(const THeap &heap, int value) {
        if (value <= 0)
            // no need for next wheel (special values already handled)
            return true;

        if (SymbolicHeap::VAL_INVALID != heap.valGetCustom(0, value))
            // we can't follow fnc pointers by pointsTo() since they are
            // sort of virtual from this ascpect (and we of course do
            // not need to follow them)
            return true;

        return false;
    }
}

namespace SymbolicHeap {
    template <class TStack, class TSubst>
    bool dfsCmp(TStack &todo,
                TSubst &valSubst,
                const SymHeap &heap1,
                const SymHeap &heap2)
    {
        // FIXME: not very efficient implementation of DFS
        std::set<int> done;

        // DFS loop
        while (!todo.empty()) {
            int value1, value2;
            boost::tie(value1, value2) = todo.top();
            todo.pop();
            done.insert(value1);

            // TODO: distinguish among SLS and single dynamic variables here
            const int obj1 = heap1.pointsTo(value1);
            const int obj2 = heap2.pointsTo(value2);
            if (checkNonPosValues(obj1, obj2))
                // variable mismatch
                return false;

            // TODO: here handle structured variables
            value1 = heap1.valueOf(obj1);
            value2 = heap2.valueOf(obj2);
            if (!matchValues(valSubst, value1, value2))
                // value mismatch
                return false;

            if (skipValue(heap1, value1))
                // no need for next wheel
                continue;

            if (!hasKey(done, value1))
                // schedule values for next wheel
                todo.push(value1, value2);
        }

        // heaps are equal (isomorphism)
        return true;
    }

    bool operator== (const SymHeap &heap1, const SymHeap &heap2) {
        // DFS stack
        typedef std::pair<int, int> TValuePair;
        typedef std::stack<TValuePair> TValueStack;
        TValueStack dfsStack;

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
            if (!matchValues(valSubst, value1, value2))
                // value mismatch, bail out now
                return false;

            if (skipValue(heap1, value1))
                // no need for next wheel
                continue;

            // schedule for DFS
            dfsStack.push(value1, value2);
        }

        // bad luck, we need to run DFS
        return dfsCmp(dfsStack, valSubst, heap1, heap2);
    }
}

void SymHeapUnion::insert(const SymbolicHeap::SymHeap &heap) {
    using SymbolicHeap::SymHeap;

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
