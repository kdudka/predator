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

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symbt.hh"
#include "symheap.hh"
#include "symproc.hh"
#include "util.hh"

#include <stack>

#include <boost/foreach.hpp>

void moveKnownValueToLeft(
        const SymHeapCore           &sh,
        TValId                      &valA,
        TValId                      &valB)
{
    sortValues(valA, valB);

    if ((0 < valA) && UV_KNOWN != sh.valGetUnknown(valA)) {
        const TValId tmp = valA;
        valA = valB;
        valB = tmp;
    }
}

// a wrapper for legacy code; this will go away once we switch to symheap-ng
TObjId objDup(SymHeap &sh, const TObjId obj) {
    const TValId addr = sh.placedAt(obj);
    const TValId dupAt = sh.valClone(addr);
    return sh.pointsTo(dupAt);
}

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
        CL_BREAK_IF(chkStack.empty());
        if (chkStack.top() != ic[i])
            // field mismatch
            return OBJ_INVALID;

        chkStack.pop();
    }
    CL_BREAK_IF(!chkStack.empty());

    return obj;
}

bool isHeapObject(const SymHeap &sh, TObjId obj) {
    if (obj <= 0)
        return false;

    const TValId at = sh.placedAt(obj);
    return SymHeap::isOnHeap(sh.valTarget(at));
}

TObjId /* root */ objRoot(const SymHeap &sh, TObjId obj) {
    if (obj <= 0)
        return obj;

    const TValId addr = sh.placedAt(obj);
    const TValId rootAt = sh.valRoot(addr);
    const TObjId root = const_cast<SymHeap &>(sh).objAt(rootAt);
    if (OBJ_UNKNOWN == root)
        // FIXME: a dangling object??? (try test-0093 with a debugger)
        return obj;

    return root;
}

void getPtrValues(TValList &dst, const SymHeap &heap, TObjId obj) {
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
                const TValId val = heap.valueOf(obj);
                if (0 < val)
                    dst.push_back(val);

                break;
            }

            case CL_TYPE_STRUCT:
            case CL_TYPE_UNION:
                for (int i = 0; i < clt->item_cnt; ++i) {
                    const TObjId subObj = heap.subObj(obj, i);
                    CL_BREAK_IF(subObj < 0);

                    todo.push(subObj);
                }
                break;

            case CL_TYPE_ENUM:
            case CL_TYPE_ARRAY:
            case CL_TYPE_CHAR:
            case CL_TYPE_BOOL:
            case CL_TYPE_INT:
                break;

            default:
                // other types of value should be safe to ignore here
                // but worth to check by a debugger at least once anyway
#ifndef NDEBUG
                CL_TRAP;
#endif
                break;
        }
    }
}

void skipObj(const SymHeap &sh, TObjId *pObj, TOffset offNext)
{
    const TObjId objPtrNext = ptrObjByOffset(sh, *pObj, offNext);
    const TObjId objNext = objRootByPtr(sh, objPtrNext);

    // move to the next object
    *pObj = objNext;
}

typedef std::pair<TObjId, const cl_initializer *> TInitialItem;

// specialization of TraverseSubObjsHelper suitable for gl initializers
template <> struct TraverseSubObjsHelper<TInitialItem> {
    static const struct cl_type* getItemClt(const SymHeap           &sh,
                                            const TInitialItem      &item)
    {
        const struct cl_type *clt = sh.objType(item.first);
        CL_BREAK_IF(item.second && (!clt || *clt != *item.second->type));
        return clt;
    }

    static TInitialItem getNextItem(const SymHeap                   &sh,
                                    TInitialItem                    item,
                                    int                             nth)
    {
        item.first = sh.subObj(item.first, nth);

        const struct cl_initializer *&initial = item.second;
        if (initial)
            initial = initial->data.nested_initials[nth];

        return item;
    }
};

bool initSingleVariable(SymHeap &sh, const TInitialItem &item) {
    const TObjId obj = item.first;
    const struct cl_type *clt = sh.objType(obj);
    CL_BREAK_IF(!clt);

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_ARRAY:
            CL_DEBUG("CL_TYPE_ARRAY is not supported by VarInitializer");
            return /* continue */ true;

        case CL_TYPE_UNION:
        case CL_TYPE_STRUCT:
            CL_TRAP;

        default:
            break;
    }

    const struct cl_initializer *initial = item.second;
    if (!initial) {
        // no initializer given, nullify the variable
        sh.objSetValue(obj, /* also equal to VAL_FALSE */ VAL_NULL);
        return /* continue */ true;
    }

    // FIXME: we're asking for troubles this way
    const CodeStorage::Storage *null = 0;
    SymBackTrace dummyBt(*null);
    SymProc proc(sh, &dummyBt);

    // resolve initial value
    const struct cl_operand *op = initial->data.value;
    const TValId val = proc.heapValFromOperand(*op);
    CL_DEBUG("using explicit initializer: obj #"
            << static_cast<int>(obj) << " <-- val #"
            << static_cast<int>(val));

    // set the initial value
    CL_BREAK_IF(VAL_INVALID == val);
    sh.objSetValue(obj, val);

    return /* continue */ true;
}

void initVariable(SymHeap                       &sh,
                  TObjId                        obj,
                  const CodeStorage::Var        &var)
{
    const TInitialItem item(obj, var.initial);

    if (isComposite(var.clt))
        traverseSubObjs(sh, item, initSingleVariable, /* leavesOnly */ true);
    else
        initSingleVariable(sh, item);
}

class PointingObjectsFinder {
    public:
        // we have to use std::set, a vector is not sufficient in all cases
        typedef std::set<TObjId> TResults;

    private:
        TResults results_;

    public:
        const TResults& results() const { return results_; }

        bool operator()(const SymHeap &sh, TObjId obj) {
            const TValId addr = sh.placedAt(obj);
            CL_BREAK_IF(addr <= 0);

            TObjList refs;
            sh.usedBy(refs, addr);
            std::copy(refs.begin(), refs.end(),
                      std::inserter(results_, results_.begin()));

            return /* continue */ true;
        }
};

void gatherPointingObjects(const SymHeap            &sh,
                           TObjList                 &dst,
                           const TObjId             root,
                           bool                     toInsideOnly)
{
    PointingObjectsFinder visitor;
    if (!toInsideOnly)
        visitor(sh, root);

    if (!isComposite(sh.objType(root)))
        // nothing to traverse here
        return;

    traverseSubObjs(sh, root, visitor, /* leavesOnly */ false);
    std::copy(visitor.results().begin(), visitor.results().end(),
              std::back_inserter(dst));
}

TObjId subSeekByOffset(
        const SymHeap               &sh,
        const TObjId                obj,
        const TOffset               offToSeek,
        const struct cl_type        *clt,
        const enum cl_type_e        code)
{
    if (obj < 0)
        return obj;

    SymHeap &shNonConst = const_cast<SymHeap &>(sh);
    const TValId addr = sh.placedAt(obj);
    const TValId subAddr = shNonConst.valByOffset(addr, offToSeek);
    CL_BREAK_IF(subAddr <= 0);

    if (clt)
        return shNonConst.objAt(subAddr, clt);
    else
        return shNonConst.objAt(subAddr, code);
}

void seekRoot(const SymHeap &sh, TObjId *pRoot, TOffset *pOff) {
    const TObjId obj = *pRoot;
    if (OBJ_INVALID == obj)
        return;

    const TObjId root = objRoot(sh, obj);
    (*pOff) += subOffsetIn(sh, root, obj);
    (*pRoot) = root;
}

TObjId ptrObjByOffset(const SymHeap &sh, TObjId obj, TOffset off) {
    seekRoot(sh, &obj, &off);
    return subSeekByOffset(sh, obj, off, /* clt */ 0, CL_TYPE_PTR);
}

TObjId compObjByOffset(const SymHeap &sh, TObjId obj, TOffset off) {
    seekRoot(sh, &obj, &off);
    return subSeekByOffset(sh, obj, off, /* clt */ 0, CL_TYPE_STRUCT);
}

TValId addrQueryByOffset(
        SymHeap                 &sh,
        const TObjId            target,
        const TOffset           offRequested,
        const struct cl_type    *cltPtr,
        const struct cl_loc     *lw)
{
    // seek root object while cumulating the offset
    TObjId obj = target;
    TObjId parent;
    TOffset off = offRequested;
    int nth;
    while (OBJ_INVALID != (parent = sh.objParent(obj, &nth))) {
        const struct cl_type *cltParent = sh.objType(parent);
        CL_BREAK_IF(cltParent->item_cnt <= nth);

        off += cltParent->items[nth].offset;
        obj = parent;
    }

    const struct cl_type *cltRoot = sh.objType(obj);
    if (!cltRoot || cltRoot->code != CL_TYPE_STRUCT) {
        if (lw)
            CL_ERROR_MSG(lw, "unsupported target type for pointer plus");

        return sh.valCreateUnknown(UV_UNKNOWN);
    }

    if (off < 0)
        // we need to create an off-value
        return sh.valByOffset(sh.placedAt(obj), off);

    // jump to _target_ type
    const struct cl_type *clt = targetTypeOfPtr(cltPtr);

    const TObjId sub = subSeekByOffset(sh, obj, off, clt);
    if (sub <= 0) {
        if (!off)
            // we have already reached zero offset
            // --> it should be safe to just return the address
            return sh.placedAt(obj);

        // fall-back to off-value, but now related to the original target,
        // instead of root
        return sh.valByOffset(sh.placedAt(target), offRequested);
    }

    return sh.placedAt(sub);
}

void redirectInboundEdges(
        SymHeap                 &sh,
        const TObjId            pointingFrom,
        const TObjId            pointingTo,
        const TObjId            redirectTo)
{
#ifndef NDEBUG
    const struct cl_type *clt1 = sh.objType(pointingTo);
    const struct cl_type *clt2 = sh.objType(redirectTo);
    CL_BREAK_IF(!clt1 || !clt2 || *clt1 != *clt2);
#endif

    // go through all objects pointing at/inside pointingTo
    TObjList refs;
    gatherPointingObjects(sh, refs, pointingTo, /* toInsideOnly */ false);
    BOOST_FOREACH(const TObjId obj, refs) {
        if (OBJ_INVALID != pointingFrom && pointingFrom != objRoot(sh, obj))
            // pointed from elsewhere, keep going
            continue;

        TObjId parent = sh.pointsTo(sh.valueOf(obj));
        CL_BREAK_IF(parent <= 0);

        // seek obj's root
        int nth;
        TFieldIdxChain invIc;
        while (OBJ_INVALID != (parent = sh.objParent(parent, &nth)))
            invIc.push_back(nth);

        // now take the selector chain reversely
        TObjId target = redirectTo;
        BOOST_REVERSE_FOREACH(int nth, invIc) {
            target = sh.subObj(target, nth);
            CL_BREAK_IF(OBJ_INVALID == target);
        }

        // redirect!
        sh.objSetValue(obj, sh.placedAt(target));
    }
}

