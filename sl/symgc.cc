/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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
#include "symgc.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symheap.hh"
#include "symutil.hh"
#include "worklist.hh"

#include <stack>
#include <vector>

#include <boost/foreach.hpp>

template <class TWL>
void digPointingObjects(TWL &wl, const SymHeap &sh, TValId val) {
    // go through all objects having the value
    TObjList cont;
    sh.usedBy(cont, val);
    BOOST_FOREACH(TObjId obj, cont) {
        wl.schedule(obj);
    }

    // seek object's root
    const TValId root = sh.valRoot(val);
    if (root < 0)
        return;

    // traverse all subobjects
    TObjList refs;
    sh.pointedBy(refs, root);
    BOOST_FOREACH(const TObjId obj, refs) {
        wl.schedule(obj);
    }
}

bool digJunk(const SymHeap &heap, TValId *ptrVal) {
    if (*ptrVal <= 0)
        return false;

    const EUnknownValue code = heap.valGetUnknown(*ptrVal);
    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            break;

        default:
            return false;
    }

    if (VAL_INVALID != heap.valGetCustom(*ptrVal))
        // ignore custom values (e.g. fnc pointers)
        return false;

    TObjId obj = heap.pointsTo(*ptrVal);
    if (!isHeapObject(heap, obj))
        // non-heap object simply can't be JUNK
        return false;

    // only root objects can be destroyed
    *ptrVal = heap.valRoot(*ptrVal);

    WorkList<TObjId> wl;
    digPointingObjects(wl, heap, *ptrVal);
    while (wl.next(obj)) {
        if (!isHeapObject(heap, obj))
            return false;

        const TValId val = heap.placedAt(obj);
        CL_BREAK_IF(val <= 0);

        digPointingObjects(wl, heap, val);
    }

    return true;
}

bool collectJunk(SymHeap &sh, TValId val, const struct cl_loc *lw) {
    bool detected = false;

    std::stack<TValId> todo;
    todo.push(val);
    while (!todo.empty()) {
        TValId val = todo.top();
        todo.pop();

        if (digJunk(sh, &val)) {
            detected = true;
            const TObjId obj = objRootByVal(sh, val);
            CL_BREAK_IF(obj <= 0);

            // gather all values inside the junk object
            std::vector<TValId> ptrs;
            getPtrValues(ptrs, sh, obj);

            // destroy junk
            if (lw)
                CL_WARN_MSG(lw, "killing junk");
            if (!sh.valDestroyTarget(sh.placedAt(obj)))
                CL_BREAK_IF("failed to kill junk");

            // schedule just created junk candidates for next wheel
            BOOST_FOREACH(TValId ptrVal, ptrs) {
                todo.push(ptrVal);
            }
        }
    }

    return detected;
}
