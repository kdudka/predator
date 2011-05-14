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

    const EValueTarget code = heap.valTarget(*ptrVal);
    if (!isOnHeap(code))
        // non-heap objects cannot be JUNK
        return false;

    // only root objects can be destroyed
    *ptrVal = heap.valRoot(*ptrVal);

    WorkList<TObjId> wl;
    digPointingObjects(wl, heap, *ptrVal);

    TObjId obj;
    while (wl.next(obj)) {
        const TValId val = heap.placedAt(obj);
        if (!isOnHeap(heap.valTarget(val)))
            // non-heap object simply can't be JUNK
            return false;

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

            // gather all values inside the junk object
            const TValId root = sh.valRoot(val);
            std::vector<TValId> ptrs;
            getPtrValues(ptrs, sh, root);

            // destroy junk
            if (lw)
                CL_WARN_MSG(lw, "killing junk");
            if (!sh.valDestroyTarget(root))
                CL_BREAK_IF("failed to kill junk");

            // schedule just created junk candidates for next wheel
            BOOST_FOREACH(TValId ptrVal, ptrs) {
                todo.push(ptrVal);
            }
        }
    }

    return detected;
}
