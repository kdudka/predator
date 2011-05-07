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

#ifndef H_GUARD_SYMUTIL_H
#define H_GUARD_SYMUTIL_H

/**
 * @file symutil.hh
 * some generic utilities working on top of a symbolic heap
 * @todo API documentation
 */

#include "config.h"

#include <cl/code_listener.h>
#include <cl/clutil.hh>

#include "symheap.hh"

#include <stack>

#include <boost/foreach.hpp>

class LocationWriter;

namespace CodeStorage {
    struct Var;
}

inline TValId boolToVal(const bool b) {
    return (b)
        ? VAL_TRUE
        : VAL_FALSE;
}

void moveKnownValueToLeft(const SymHeapCore &sh, TValId &valA, TValId &valB);

inline TValId valOfPtrAt(SymHeap &sh, TValId at) {
    const TObjId ptr = sh.ptrAt(at);
    return sh.valueOf(ptr);
}

inline TValId valOfPtrAt(SymHeap &sh, TValId at, TOffset off) {
    const TValId ptrAt = sh.valByOffset(at, off);
    return valOfPtrAt(sh, ptrAt);
}

// TODO: remove this
TObjId objRoot(const SymHeap &sh, TObjId obj);

// TODO: remove this
inline TObjId objRootByVal(const SymHeap &sh, TValId val) {
    if (val <= 0)
        return OBJ_INVALID;

    const TValId rootAt = sh.valRoot(val);
    return sh.pointsTo(rootAt);
}

// TODO: remove this
inline TObjId objRootByPtr(const SymHeap &sh, TObjId ptr) {
    const TValId val = sh.valueOf(ptr);
    return objRootByVal(sh, val);
}

// TODO: remove this
inline bool objIsSeg(const SymHeap &sh, TObjId obj) {
    const TValId addr = sh.placedAt(obj);
    if (addr <= 0)
        return false;

    return SymHeap::isAbstract(sh.valTarget(addr));
}

// TODO: remove this
inline EObjKind objKind(const SymHeap &sh, TObjId obj) {
    const TValId addr = sh.placedAt(obj);
    if (addr <= 0)
        // not really
        return OK_CONCRETE;

    return sh.valTargetKind(addr);
}

void getPtrValues(TValList &dst, const SymHeap &heap, TValId at);

inline TValId nextRootObj(SymHeap &sh, TValId root, TOffset offNext) {
    CL_BREAK_IF(sh.valOffset(root));
    const TObjId nextPtr = sh.ptrAt(sh.valByOffset(root, offNext));
    return sh.valRoot(sh.valueOf(nextPtr));
}

void initVariable(SymHeap                       &sh,
                  TObjId                        obj,
                  const CodeStorage::Var        &var);

typedef std::pair<TObjId, TObjId> TObjPair;

#ifndef BUILDING_DOX
// helper template for traverseSubObjs()
template <class TItem> struct TraverseSubObjsHelper { };

// specialisation suitable for traversing two composite objects simultaneously
template <> struct TraverseSubObjsHelper<TObjPair> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjPair item) {
        const struct cl_type *clt = sh.objType(item.first);
        CL_BREAK_IF(!clt || *clt != *sh.objType(item.second));
        return clt;
    }
    static TObjPair getNextItem(const SymHeap &sh, TObjPair item, int nth) {
        item.first  = sh.subObj(item.first,  nth);
        item.second = sh.subObj(item.second, nth);
        return item;
    }
};
#endif

/// take the given visitor through all live pointers
template <class THeap, class TVisitor, typename TMethod>
bool /* complete */ traverseCore(
        THeap                       &sh,
        const TValId                at,
        TVisitor                    &visitor,
        TMethod                     method)
{
    // check that we got a valid root object
    CL_BREAK_IF(const_cast<SymHeap &>(sh).objAt(at) <= 0);
    const TValId rootAt = sh.valRoot(at);
    const TOffset offRoot = sh.valOffset(at);

    TObjList objs;
    (sh.*method)(objs, rootAt);
    BOOST_FOREACH(const TObjId obj, objs) {
        const TOffset off = sh.valOffset(sh.placedAt(obj));
        if (off < offRoot)
            // do not go above the starting point
            continue;

        if (!visitor(sh, obj))
            // traversal cancelled by visitor
            return false;
    }

    // done
    return true;
}

/// take the given visitor through all live pointers
template <class THeap, class TVisitor>
bool /* complete */ traverseLivePtrs(
        THeap                       &sh,
        const TValId                rootAt,
        TVisitor                    &visitor)
{
    return traverseCore(sh, rootAt, visitor, &SymHeap::gatherLivePointers);
}

/// take the given visitor through all live objects
template <class THeap, class TVisitor>
bool /* complete */ traverseLiveObjs(
        THeap                       &sh,
        const TValId                rootAt,
        TVisitor                    &visitor)
{
    return traverseCore(sh, rootAt, visitor, &SymHeap::gatherLiveObjects);
}

/// take the given visitor through a composite object (or whatever you pass in)
template <class THeap, class TVisitor, class TItem = TObjId>
bool /* complete */ traverseSubObjs(THeap &sh, TItem item, TVisitor &visitor,
                                    bool leavesOnly)
{
    std::stack<TItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        typedef TraverseSubObjsHelper<TItem> THelper;
        const struct cl_type *clt = THelper::getItemClt(sh, item);
        CL_BREAK_IF(!clt || !isComposite(clt));

        for (int i = 0; i < clt->item_cnt; ++i) {
            const TItem next = THelper::getNextItem(sh, item, i);

            const struct cl_type *subClt = THelper::getItemClt(sh, next);
            if (subClt && isComposite(subClt)) {
                todo.push(next);

                if (leavesOnly)
                    // do not call the visitor for internal nodes, if requested
                    continue;
            }

            if (!/* continue */visitor(sh, next))
                return false;
        }
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

// TODO: remove this
TObjId subObjByChain(const SymHeap &sh, TObjId obj, TFieldIdxChain ic);

/// (VAL_INVALID != pointingFrom) means 'pointing from anywhere'
void redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo);

#endif /* H_GUARD_SYMUTIL_H */
