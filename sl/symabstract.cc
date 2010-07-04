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

#include <boost/foreach.hpp>

typedef std::pair<TObjId, TObjId> TObjPair;

// helper template for traverseSubObjs()
template <class TItem> struct TraverseSubObjsHelper { };

// specialisation for TObjId, which means basic implementation of the traversal
template <> struct TraverseSubObjsHelper<TObjId> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjId obj) {
        return sh.objType(obj);
    }
    static TObjId getNextItem(const SymHeap &sh, TObjId obj, int nth) {
        return sh.subObj(obj, nth);
    }
};

// specialisation suitable for traversing two composite objects simultaneously
template <> struct TraverseSubObjsHelper<TObjPair> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjPair item) {
        const struct cl_type *clt1 = sh.objType(item.first);
        const struct cl_type *clt2 = sh.objType(item.second);
        if (clt1 != clt2)
            TRAP;

        return clt1;
    }
    static TObjPair getNextItem(const SymHeap &sh, TObjPair item, int nth) {
        item.first  = sh.subObj(item.first,  nth);
        item.second = sh.subObj(item.second, nth);
        return item;
    }
};

// take the given visitor through a composite object (or whatever you pass in)
template <class THeap, typename TVisitor, class TItem = TObjId>
bool /* complete */ traverseSubObjs(THeap &sh, TItem item, TVisitor visitor) {
    std::stack<TItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        typedef TraverseSubObjsHelper<TItem> THelper;
        const struct cl_type *clt = THelper::getItemClt(sh, item);
        if (!clt || clt->code != CL_TYPE_STRUCT)
            TRAP;

        for (int i = 0; i < clt->item_cnt; ++i) {
            const TItem next = THelper::getNextItem(sh, item, i);
            if (!/* continue */visitor(sh, next))
                return false;

            const struct cl_type *subClt = THelper::getItemClt(sh, next);
            if (subClt && subClt->code == CL_TYPE_STRUCT)
                todo.push(next);
        }
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

namespace {

bool doesAnyonePointToInsideVisitor(const SymHeap &sh, TObjId sub) {
    const TValueId subAddr = sh.placedAt(sub);
    return /* continue */ !sh.usedByCount(subAddr);
}

bool doesAnyonePointToInside(const SymHeap &sh, TObjId obj) {
    return !traverseSubObjs(sh, obj, doesAnyonePointToInsideVisitor);
}

bool abstractNonMatchingValuesVisitor(SymHeap &sh, TObjPair item) {
    const TObjId dst = item.second;
    const TValueId valSrc = sh.valueOf(item.first);
    const TValueId valDst = sh.valueOf(dst);
    bool eq;
    if (sh.proveEq(&eq, valSrc, valDst) && eq)
        // values are equal
        return /* continue */ true;

    // attempt to dig some type-info for the new unknown value
    const struct cl_type *clt = sh.valType(valSrc);
    const struct cl_type *cltDst = sh.valType(valDst);
    if (!clt)
        clt = cltDst;
    else if (cltDst && cltDst != clt)
        // should be safe to ignore
        TRAP;

    // create a new unknown value as a placeholder
    const TValueId valNew = sh.valCreateUnknown(UV_UNKNOWN, clt);
    sh.objSetValue(dst, valNew);
    return /* continue */ true;
}

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(SymHeap &sh, TObjId src, TObjId dst) {
    if (OK_CONCRETE == sh.objKind(dst))
        // invalid call of abstractNonMatchingValues()
        TRAP;

    // wait, first preserve the value of binder
    const TObjId objBind = subObjByChain(sh, dst, sh.objBinderField(dst));
    const TValueId valBind = sh.valueOf(objBind);
    if (!sh.objPeerField(dst).empty())
        // TODO: do the same for the 'peer' field, DLS are involved
        TRAP;

    const TObjPair item(src, dst);
    traverseSubObjs(sh, item, abstractNonMatchingValuesVisitor);

    // now restore the possibly smashed value of binder
    sh.objSetValue(objBind, valBind);
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

EObjKind discover(const SymHeap &sh, TObjId obj, TFieldIdxChain &icBind,
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

class ProbeVisitor {
    private:
        TValueId                addr_;
        const struct cl_type    *clt_;

    public:
        ProbeVisitor(const SymHeap &sh, TObjId obj) {
            addr_ = sh.placedAt(obj);
            clt_  = sh.objType(obj);
            if (!addr_ || !clt_ || CL_TYPE_STRUCT != clt_->code)
                TRAP;
        }

    bool operator()(SymHeap &sh, TObjId obj) const {
        const TValueId valNext = sh.valueOf(obj);
        if (valNext <= 0 || sh.valType(valNext) != clt_)
            return /* continue */ true;

        if (valNext == addr_ || UV_KNOWN != sh.valGetUnknown(valNext))
            return /* continue */ true;

        const TObjId target = sh.pointsTo(valNext);
        return doesAnyonePointToInside(sh, target);
    }
};

bool probe(SymHeap &sh, TObjId obj) {
    if (doesAnyonePointToInside(sh, obj))
        return false;

    const ProbeVisitor visitor(sh, obj);
    return !traverseSubObjs(sh, obj, visitor);
}

void abstract(SymHeap &sh, TObjId obj) {
#if !SE_DISABLE_ABSTRACT
    if (!probe(sh, obj))
#endif
        return;

    CL_DEBUG("abstract: initial probe was successful, now "
            "digging selectors...");

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
        abstractNonMatchingValues(sh, obj, objNext);
        objReplace(sh, obj, objNext);

        // move to the next object
        obj = objNext;
    }
}

} // namespace

void abstractIfNeeded(SymHeap &sh) {
    SymHeapCore::TContObj roots;
    sh.gatherRootObjs(roots);
    BOOST_FOREACH(const TObjId obj, roots) {
        const TValueId addr = sh.placedAt(obj);
        const unsigned uses = sh.usedByCount(addr);
        switch (uses) {
            case 0:
                CL_WARN("abstractIfNeeded() encountered an unused root object #"
                        << obj);
                // fall through!

            default:
                continue;

            case 1:
                // a candidate for abstraction
                abstract(sh, obj);
        }
    }
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
