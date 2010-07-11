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

void objReplace(SymHeap &sh, TObjId oldObj, TObjId newObj) {
    if (OBJ_INVALID != sh.objParent(oldObj)
            || OBJ_INVALID != sh.objParent(newObj))
        // attempt to replace a sub-object
        TRAP;

    // resolve object addresses
    const TValueId oldAddr = sh.placedAt(oldObj);
    const TValueId newAddr = sh.placedAt(newObj);
    if (oldAddr <= 0 || newAddr <= 0)
        TRAP;

    // update all references
    sh.valReplace(oldAddr, newAddr);

    // now destroy the old object
    sh.objDestroy(oldObj);
}

void skipObj(const SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext) {
    const TObjId objPtrNext = subObjByChain(sh, *pObj, icNext);
    const TValueId valNext = sh.valueOf(objPtrNext);
    const TObjId objNext = sh.pointsTo(valNext);
    if (OBJ_INVALID == objNext)
        TRAP;

    // move to the next object
    *pObj = objNext;
}

TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg) {
    if (OK_CONCRETE == sh.objKind(seg))
        // invalid call of nextPtrFromSeg()
        TRAP;

    const TFieldIdxChain icNext = sh.objNextField(seg);
    return subObjByChain(sh, seg, icNext);
}

TObjId dlSegPeer(const SymHeap &sh, TObjId dls) {
    if (OK_DLS != sh.objKind(dls))
        // invalid call of dlSegPeer()
        TRAP;

    TObjId peer = dls;
    skipObj(sh, &peer, sh.objPeerField(dls));
    return peer;
}

namespace {
    bool segProveNeq(const SymHeap &sh, TValueId v1, TValueId v2) {
        bool eq;
        if (!sh.proveEq(&eq, v1, v2))
            return /* no idea */ false;

        if (eq)
            // equal ... basically means 'invalid segment'
            TRAP;

        return /* not equal */ true;
    }
}

bool dlSegNotEmpty(const SymHeap &sh, TObjId dls) {
    if (OK_DLS != sh.objKind(dls))
        // invalid call of dlSegNotEmpty()
        TRAP;

    const TObjId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // red the values (addresses of the surround)
    const TValueId val1 = sh.valueOf(next1);
    const TValueId val2 = sh.valueOf(next2);

    // attempt to prove both
    const bool ne1 = segProveNeq(sh, val1, sh.placedAt(peer));
    const bool ne2 = segProveNeq(sh, val2, sh.placedAt(dls));
    if (ne1 && ne2)
        return /* not empty */ true;

    if (!ne1 && !ne2)
        return /* possibly empty */ false;

    // the given DLS is guaranteed to be non empty in one direction, but not
    // vice versa --> such a DLS is considered as mutant and should not be
    // passed through
    TRAP;
    return false;
}

bool segNotEmpty(const SymHeap &sh, TObjId seg) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of segNotEmpty()
            TRAP;

        case OK_SLS:
            break;

        case OK_DLS:
            return dlSegNotEmpty(sh, seg);
    }

    const TObjId next = nextPtrFromSeg(sh, seg);
    const TValueId nextVal = sh.valueOf(next);
    const TValueId addr = sh.placedAt(seg);
    return /* not empty */ segProveNeq(sh, addr, nextVal);
}

bool segEqual(const SymHeap &sh, TValueId v1, TValueId v2) {
    const TObjId o1 = sh.pointsTo(v1);
    const TObjId o2 = sh.pointsTo(v2);
    if (o1 <= 0 || o2 <= 0)
        TRAP;

    const EObjKind kind = sh.objKind(o1);
    if (sh.objKind(o2) != kind)
        return false;

    TObjId peer1 = o1;
    TObjId peer2 = o2;
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of segEqual()
            TRAP;

        case OK_DLS:
            if (sh.objPeerField(o1) != sh.objPeerField(o2))
                // 'peer' selector mismatch
                return false;

            peer1 = dlSegPeer(sh, o1);
            peer2 = dlSegPeer(sh, o2);
            // fall through!

        case OK_SLS:
            if (sh.objNextField(o1) != sh.objNextField(o2))
                // 'next' selector mismatch
                return false;
    }

    // so far equal, now compare the 'next' values
    const TObjId next1 = nextPtrFromSeg(sh, peer1);
    const TObjId next2 = nextPtrFromSeg(sh, peer2);
    return (sh.valueOf(next1) == sh.valueOf(next2));
}

void segDestroy(SymHeap &sh, TObjId seg) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of segDestroy()
            TRAP;

        case OK_DLS:
            sh.objDestroy(dlSegPeer(sh, seg));
            // fall through!

        case OK_SLS:
            sh.objDestroy(seg);
    }
}

