/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYMSEG_H
#define H_GUARD_SYMSEG_H

/**
 * @file symseg.hh
 * some generic utilities operating on list segments (SLS, DLS, Linux lists)
 */

#include "config.h"
#include "symheap.hh"
#include "symutil.hh"

/**
 * return true if there is a list segment among the given pair of values
 * @param sh an instance of symbolic heap
 * @param atAddr address of the heap object for consideration
 * @param pointingTo target address of the given potential list segment
 * @param kind kind of list segment to look for
 */
bool haveSeg(const SymHeap &sh, TValId atAddr, TValId pointingTo,
             const EObjKind kind);

/**
 * return true if there is a DLS (Doubly-linked List Segment) among the given
 * pair of values
 * @param sh an instance of symbolic heap
 * @param atAddr address of the heap object for consideration
 * @param peerAddr potential address of DLS peer object
 */
bool haveDlSegAt(const SymHeap &sh, TValId atAddr, TValId peerAddr);

/// TODO: remove this
TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg);

/// TODO: remove this
TObjId peerPtrFromSeg(const SymHeap &sh, TObjId seg);

/// TODO: remove this
TObjId dlSegPeer(const SymHeap &sh, TObjId dls);

/// return 'next' pointer in the given segment (given by root)
inline TObjId nextPtrFromSeg(SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(VT_ABSTRACT != sh.valTarget(seg));

    const BindingOff &off = sh.segBinding(seg);
    const TValId addr = sh.valByOffset(seg, off.next);
    return sh.ptrAt(addr);
}

/// return 'prev' pointer in the given segment (given by root)
inline TObjId prevPtrFromSeg(SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(VT_ABSTRACT != sh.valTarget(seg));

    const BindingOff &off = sh.segBinding(seg);
    const TValId addr = sh.valByOffset(seg, off.prev);
    return sh.ptrAt(addr);
}

/// return DLS peer object of the given DLS
inline TValId dlSegPeer(SymHeap &sh, TValId dls) {
    CL_BREAK_IF(sh.valOffset(dls));
    CL_BREAK_IF(OK_DLS != sh.valTargetKind(dls));
    const BindingOff &off = sh.segBinding(dls);
    const TObjId ptr = sh.ptrAt(sh.valByOffset(dls, off.prev));
    return sh.valRoot(sh.valueOf(ptr));
}

/// return DLS peer object in case of DLS, the given value otherwise
inline TValId segPeer(SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(!SymHeap::isAbstract(sh.valTarget(seg)));
    return (OK_DLS == sh.valTargetKind(seg))
        ? dlSegPeer(sh, seg)
        : seg;
}

/// return address of segment's head (useful mainly for Linux lists)
inline TValId segHeadAt(SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(VT_ABSTRACT != sh.valTarget(seg));

    const BindingOff &off = sh.segBinding(seg);
    return sh.valByOffset(seg, off.head);
}

/// we do NOT require root to be a segment
inline TValId segNextRootObj(SymHeap &sh, TValId at, TOffset offNext) {
    CL_BREAK_IF(sh.valOffset(at));
    if (OK_DLS == sh.valTargetKind(at))
        // jump to peer in case of DLS
        at = dlSegPeer(sh, at);

    return nextRootObj(sh, at, offNext);
}

/// return lower estimation of DLS length
unsigned dlSegMinLength(const SymHeap &sh, TValId dls);

void dlSegSetMinLength(SymHeap &sh, TValId dls, unsigned len);

/// return lower estimation of segment length
unsigned segMinLength(const SymHeap &sh, TValId seg);

inline unsigned objMinLength(const SymHeap &sh, TValId at) {
    if (const_cast<SymHeap &>(sh).objAt(at) <= 0)
        // XXX
        return 0;

    return (SymHeap::isAbstract(sh.valTarget(at)))
        ? segMinLength(sh, at)
        : /* OK_CONCRETE */ 1;
}

void segSetMinLength(SymHeap &sh, TValId seg, unsigned len);

inline const BindingOff& segBinding(const SymHeap &sh, TObjId obj) {
    const TValId addr = sh.placedAt(obj);
    CL_BREAK_IF(addr <= 0);
    return sh.segBinding(addr);
}

inline void objSetAbstract(
        SymHeap                     &sh,
        const TObjId                obj,
        const EObjKind              kind,
        const BindingOff            &off)
{
    const TValId addr = sh.placedAt(obj);
    CL_BREAK_IF(addr <= 0);
    sh.valTargetSetAbstract(addr, kind, off);
}

inline void objSetConcrete(
        SymHeap                     &sh,
        const TObjId                obj)
{
    const TValId addr = sh.placedAt(obj);
    CL_BREAK_IF(addr <= 0);
    sh.valTargetSetConcrete(addr);
}

/// same as SymHeap::objSetProto(), but takes care of DLS peers
void segSetProto(SymHeap &sh, TValId seg, bool isProto);

TValId segClone(SymHeap &sh, const TValId seg);

/// destroy the given list segment object (including DLS peer in case of DLS)
void segDestroy(SymHeap &sh, TObjId seg);

// TODO: remove this
inline TValId segHeadAddr(const SymHeap &sh, TObjId seg) {
    return segHeadAt(const_cast<SymHeap &>(sh), sh.placedAt(seg));
}

template <class TIgnoreList>
void buildIgnoreList(
        TIgnoreList             &ignoreList,
        const SymHeap           &sh,
        const TValId            at)
{
    SymHeap &writable = const_cast<SymHeap &>(sh);
    TOffset off;
    TObjId tmp;

    const EObjKind kind = sh.valTargetKind(at);
    switch (kind) {
        case OK_CONCRETE:
            return;

        case OK_DLS:
            // preserve 'peer' field
            off = sh.segBinding(at).prev;
            tmp = writable.ptrAt(writable.valByOffset(at, off));
            ignoreList.insert(tmp);
            // fall through!

        case OK_SLS:
        case OK_MAY_EXIST:
            // preserve 'next' field
            off = sh.segBinding(at).next;
            tmp = writable.ptrAt(writable.valByOffset(at, off));
            ignoreList.insert(tmp);
    }
}

template <class TIgnoreList>
void buildIgnoreList(
        TIgnoreList             &ignoreList,
        const SymHeap           &sh,
        const TObjId            obj,
        const BindingOff        &off)
{
    // TODO: remove this
    const TValId at = sh.placedAt(obj);
    SymHeap &writable = const_cast<SymHeap &>(sh);

    const TObjId next = writable.ptrAt(writable.valByOffset(at, off.next));
    if (OBJ_INVALID != next)
        ignoreList.insert(next);

    const TObjId prev = writable.ptrAt(writable.valByOffset(at, off.prev));
    if (OBJ_INVALID != prev)
        ignoreList.insert(prev);
}

/**
 * returns true if all DLS in the given symbolic heap are consistent
 * @note this runs in debug build only
 */
bool dlSegCheckConsistency(const SymHeap &sh);

#endif /* H_GUARD_SYMSEG_H */
