/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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
#include "symabstract.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "util.hh"

#include <stack>

namespace {

bool doesAnyonePointToInside(const SymHeap1 &sh, TObjId obj) {
    // traverse the given composite object recursively
    std::stack<TObjId> todo;
    todo.push(obj);
    while (!todo.empty()) {
        obj = todo.top();
        todo.pop();

        const struct cl_type *clt = sh.objType(obj);
        if (!clt || clt->code != CL_TYPE_STRUCT)
            TRAP;

        for (int i = 0; i < clt->item_cnt; ++i) {
            const TObjId sub = sh.subObj(obj, i);
            const TValueId subAddr = sh.placedAt(sub);
            if (sh.usedByCount(subAddr))
                return true;

            const struct cl_type *subClt = sh.objType(sub);
            if (subClt && subClt->code == CL_TYPE_STRUCT)
                todo.push(sub);
        }
    }

    return false;
}

void objReplace(SymHeap1 &sh, TObjId oldObj, TObjId newObj) {
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

    // now destroy the object
    sh.objDestroy(oldObj);
}

EObjKind discover(const SymHeapEx &sh, TObjId obj, TFieldIdxChain &icBind,
                  TFieldIdxChain &icPeer)
{
    const struct cl_type *clt = sh.objType(obj);
    if (CL_TYPE_STRUCT != clt->code)
        TRAP;

    // TODO: search recursively
    int nth = -1;
    for (int i = 0; i < clt->item_cnt; ++i) {
        const TObjId objPtrNext = sh.subObj(obj, i);
        const TValueId valNext = sh.valueOf(objPtrNext);
        if (valNext <= 0)
            continue;

        if (sh.valType(valNext) != clt)
            continue;

        if (UV_KNOWN != sh.valGetUnknown(valNext))
            continue;

        if (-1 == nth)
            nth = i;
        else
            CL_DEBUG("discover() is taking first selector of suitable type as "
                    "'next', but there are more of such candidates!");
    }

    if (-1 == nth)
        // no match
        return OK_CONCRETE;

    icBind.push_back(nth);

    // TODO: DLS
    (void) icPeer;
    return OK_SLS;
}

void abstract(SymHeapEx &sh, TObjId obj) {
#if SE_DISABLE_ABSTRACT
    return;
#endif
    if (doesAnyonePointToInside(sh, obj))
        return;

    // a temporary solution preventing as from an infinite loop
    std::set<TObjId> done;
    while (!hasKey(done, obj)) {
        done.insert(obj);
        if (1 < sh.objAbstractLevel(obj))
            // not supported for now
            TRAP;

        TFieldIdxChain icBind;
        TFieldIdxChain icPeer;
        const EObjKind kind = discover(sh, obj, icBind, icPeer);
        if (OK_CONCRETE == kind)
            return;

        if (OK_SLS != kind)
            // something more than OK_SLS
            TRAP;

        const TObjId objPtrNext = subObjByChain(sh, obj, icBind);
        const TValueId valNext = sh.valueOf(objPtrNext);
        if (valNext <= 0)
            // this looks like a failure of discover()
            TRAP;

        if (1 != sh.usedByCount(valNext))
            // the next object is pointed, giving up...
            return;

        // resolve the 'next' ptr
        const TObjId objNext = sh.pointsTo(valNext);
        if (doesAnyonePointToInside(sh, objNext))
            // somone points to a field of the target object, giving up...
            return;

        if (OK_CONCRETE == sh.objKind(objNext)) {
            // abstract the _next_ object
            sh.objAbstract(objNext, kind, icBind, icPeer);

            // we're constructing the abstract object from a concrete one --> it
            // implies non-empty LS at this point
            const TValueId addrNext = sh.placedAt(objNext);
            const TObjId objNextNextPtr = subObjByChain(sh, objNext, icBind);
            const TValueId valNextNext = sh.valueOf(objNextNextPtr);
            if (addrNext <= 0 || valNextNext < /* we allow NULL here */ 0)
                TRAP;
            sh.addNeq(addrNext, valNextNext);
        }

        if (OK_SLS != sh.objKind(objNext))
            TRAP;

        // replace self by the next object
        objReplace(sh, obj, objNext);

        // move to the next object
        obj = objNext;
    }
}

} // namespace

void abstractIfNeeded(SymHeap &sh) {
    // FIXME: brute force method
    // TODO: start from cVars instead
    int i;
    for(i=0; i<=sh.lastObjId(); ++i) {
        TObjId o = static_cast<TObjId>(i);
        TValueId addr = sh.placedAt(o);
        if(addr==VAL_INVALID)
            continue;   // no address value => invalid object id

        const struct cl_type *clt = sh.objType(o);
        if(!clt)
            continue;   // anonymous object of known size

        if(clt->code != CL_TYPE_STRUCT)
            continue;   // we can abstract structs only

        if(sh.usedByCount(addr)==1)
            // a candidate for abstraction
            abstract(sh, o);

    } // for each object-id
}

void concretizeObj(SymHeap &sh, TObjId ao, TSymHeapList &todo) {
    const TFieldIdxChain ciBind = sh.objBinderField(ao);
    const TObjId objPtrNext = subObjByChain(sh, ao, ciBind);
    const TValueId valNext = sh.valueOf(objPtrNext);
    const TObjId objNext = sh.pointsTo(valNext);
    const TValueId addr = sh.placedAt(ao);

    // check if the LS may be empty
    bool eq;
    if (!sh.proveEq(&eq, addr, valNext)) {
        // possibly empty LS
        SymHeap sh0(sh);
        if (OBJ_INVALID == objNext) {
            // 'next' pointer does not point to a valid object
            sh0.valReplace(addr, valNext);
            sh0.objDestroy(ao);
        }
        else
            objReplace(sh0, ao, objNext);
        todo.push_back(sh0);
    }

    if (eq)
        // self loop?
        TRAP;

    // duplicate self as abstract object
    const TObjId aoDup = sh.objDup(ao);

    // concretize self
    sh.objConcretize(ao);

    // now chain it all together
    sh.objSetValue(objPtrNext, sh.placedAt(aoDup));
}
