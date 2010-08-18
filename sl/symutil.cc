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

TObjId subObjByInvChain(const SymHeap &sh, TObjId obj, TFieldIdxChain ic) {
    std::stack<int> chkStack;

    // just slowly go to the root
    for (unsigned i = 0; i < ic.size(); ++i) {
        int nth;
        const TObjId parent = sh.objParent(obj, &nth);
        if (OBJ_INVALID == parent)
            // count mismatch
            return OBJ_INVALID;

        chkStack.push(nth);
        obj = parent;
    }

    // now check if the captured selector sequence matches the given one
    for (unsigned i = 0; i < ic.size(); ++i) {
        SE_BREAK_IF(chkStack.empty());
        if (chkStack.top() != ic[i])
            // field mismatch
            return OBJ_INVALID;

        chkStack.pop();
    }
    SE_BREAK_IF(!chkStack.empty());

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
    SE_BREAK_IF(obj < 0);

    TObjId parent;
    while (OBJ_INVALID != (parent = heap.objParent(obj)))
        obj = parent;

    TValueId val = heap.placedAt(obj);
    SE_BREAK_IF(val <= 0);

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
                    SE_BREAK_IF(subObj < 0);

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
#if SE_SELF_TEST
                SE_TRAP;
#endif
                break;
        }
    }
}

void objReplace(SymHeap &sh, TObjId oldObj, TObjId newObj) {
    // check for possible replacement of sub-object
    SE_BREAK_IF(OBJ_INVALID != sh.objParent(oldObj));
    SE_BREAK_IF(OBJ_INVALID != sh.objParent(newObj));

    // resolve object addresses
    const TValueId oldAddr = sh.placedAt(oldObj);
    const TValueId newAddr = sh.placedAt(newObj);
    SE_BREAK_IF(oldAddr <= 0 || newAddr <= 0);

    // update all references
    sh.valReplace(oldAddr, newAddr);

    // now destroy the old object
    sh.objDestroy(oldObj);
}

void skipObj(const SymHeap &sh, TObjId *pObj, TFieldIdxChain icHead,
             TFieldIdxChain icNext)
{
    const TObjId objPtrNext = subObjByChain(sh, *pObj, icNext);
    const TObjId headNext = sh.pointsTo(sh.valueOf(objPtrNext));
    const TObjId objNext = subObjByInvChain(sh, headNext, icHead);

    // move to the next object
    *pObj = objNext;
}
