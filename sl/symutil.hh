/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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

#include <set>

#include <boost/foreach.hpp>

typedef std::set<CVar> TCVarSet;

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

void getPtrValues(TValList &dst, const SymHeap &heap, TValId at);

inline TValId nextRootObj(SymHeap &sh, TValId root, TOffset offNext) {
    CL_BREAK_IF(sh.valOffset(root));
    const TObjId nextPtr = sh.ptrAt(sh.valByOffset(root, offNext));
    return sh.valRoot(sh.valueOf(nextPtr));
}

template <class TDst, typename TInserter>
void gatherProgramVarsCore(TDst &dst, const SymHeap &sh, TInserter ins) {
    TValList live;
    sh.gatherRootObjects(live, isProgramVar);

    BOOST_FOREACH(const TValId root, live)
        (dst.*ins)(sh.cVarByRoot(root));
}

inline void gatherProgramVars(TCVarList &dst, const SymHeap &sh) {
    void (TCVarList::*ins)(const CVar &) = &TCVarList::push_back;
    gatherProgramVarsCore(dst, sh, ins);
}

inline void gatherProgramVars(TCVarSet &dst, const SymHeap &sh) {
    typedef std::pair<TCVarSet::iterator, bool> TRet;
    TRet (TCVarSet::*ins)(const CVar &) = &TCVarSet::insert;
    gatherProgramVarsCore(dst, sh, ins);
}

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

/// take the given visitor through all live objects object-wise
template <unsigned N, class THeap, class TVisitor>
bool /* complete */ traverseLiveObjsGeneric(
        THeap                *const heaps[N],
        const TValId                at[N],
        TVisitor                    &visitor)
{
    // collect the starting points
    TValId roots[N];
    TOffset offs[N];
    for (unsigned i = 0; i < N; ++i) {
        SymHeap &sh = *heaps[i];

        const TValId addr = at[i];
        roots[i] = sh.valRoot(addr);
        offs[i] = sh.valOffset(addr);
    }

    // collect all live objects from everywhere
    typedef std::pair<TOffset, TObjType> TItem;
    std::set<TItem> all;
    for (unsigned i = 0; i < N; ++i) {
        SymHeap &sh = *heaps[i];
        const TValId root = roots[i];
        if (root < 0)
            continue;

        TObjList objs;
        sh.gatherLiveObjects(objs, root);
        BOOST_FOREACH(const TObjId obj, objs) {
            const TValId objAt = sh.placedAt(obj);
            const TOffset off = sh.valOffset(objAt) - offs[i];
            if (off < 0)
                // do not go above the starting point
                continue;

            const TObjType clt = sh.objType(obj);
            const TItem item(off, clt);
            all.insert(item);
        }
    }

    // go through all live objects
    BOOST_FOREACH(const TItem &item, all) {
        const TOffset  off = item.first;
        const TObjType clt = item.second;

        TObjId objs[N];
        for (unsigned i = 0; i < N; ++i) {
            SymHeap &sh = *heaps[i];

            const TValId addr = sh.valByOffset(roots[i], offs[i] + off);
            objs[i] = sh.objAt(addr, clt);
        }

        if (!visitor(objs))
            // traversal cancelled by visitor
            return false;
    }

    // done
    return true;
}

/// take the given visitor through all live objects object-wise
template <unsigned N, class THeap, class TVisitor>
bool /* complete */ traverseLiveObjs(
        THeap                       &sh,
        const TValId                at[N],
        TVisitor                    &visitor)
{
    THeap *heaps[N];
    for (unsigned i = 0; i < N; ++i)
        heaps[i] = &sh;

    return traverseLiveObjsGeneric<N>(heaps, at, visitor);
}

/// (VAL_INVALID != pointingFrom) means 'pointing from anywhere'
void redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo);

#endif /* H_GUARD_SYMUTIL_H */
