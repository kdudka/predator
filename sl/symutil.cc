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
#include "symutil.hh"

#include <cl/storage.hh>

#include "symheap.hh"

#include <stack>

#include <boost/foreach.hpp>

TObjId subObjByChain(const SymHeap &sh, TObjId obj, TFieldIdxChain ic) {
    BOOST_FOREACH(const int nth, ic) {
        obj = sh.subObj(obj, nth);
        if (OBJ_INVALID == obj)
            break;
    }

    return obj;
}

bool isHeapObject(const SymHeap &heap, TObjId obj) {
    if (obj <= 0)
        return false;

    for (; OBJ_INVALID != obj; obj = heap.objParent(obj))
        if (heap.cVar(0, obj))
            return false;

    return true;
}

void digRootObject(const SymHeap &heap, TValueId *pValue) {
    TObjId obj = heap.pointsTo(*pValue);
    if (obj < 0)
        TRAP;

    TObjId parent;
    while (OBJ_INVALID != (parent = heap.objParent(obj)))
        obj = parent;

    TValueId val = heap.placedAt(obj);
    if (val <= 0)
        TRAP;

    *pValue = val;
}

void getPtrValues(SymHeapCore::TContValue &dst, const SymHeap &heap,
                  TObjId obj)
{
    std::stack<TObjId> todo;
    todo.push(obj);
    while (!todo.empty()) {
        const TObjId obj = todo.top();
        todo.pop();

        const struct cl_type *clt = heap.objType(obj);
        const enum cl_type_e code = (clt)
            ? clt->code
            : /* anonymous object of known size */ CL_TYPE_PTR;

        switch (code) {
            case CL_TYPE_PTR: {
                const TValueId val = heap.valueOf(obj);
                if (0 < val)
                    dst.push_back(val);

                break;
            }

            case CL_TYPE_STRUCT:
                for (int i = 0; i < clt->item_cnt; ++i) {
                    const TObjId subObj = heap.subObj(obj, i);
                    if (subObj < 0)
                        TRAP;

                    todo.push(subObj);
                }
                break;

            case CL_TYPE_ARRAY:
            case CL_TYPE_CHAR:
            case CL_TYPE_BOOL:
            case CL_TYPE_INT:
                break;

            default:
                // other types of value should be safe to ignore here
                // but worth to check by a debugger at least once anyway
                TRAP;
        }
    }
}
