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

/// return 'next' pointer in the given segment (given by root)
inline TObjId nextPtrFromSeg(const SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(VT_ABSTRACT != sh.valTarget(seg));

    const BindingOff &off = sh.segBinding(seg);
    const TValId addr = const_cast<SymHeap &>(sh).valByOffset(seg, off.next);
    return const_cast<SymHeap &>(sh).ptrAt(addr);
}

/// return 'prev' pointer in the given segment (given by root)
inline TObjId prevPtrFromSeg(const SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(VT_ABSTRACT != sh.valTarget(seg));

    const BindingOff &off = sh.segBinding(seg);
    const TValId addr = const_cast<SymHeap &>(sh).valByOffset(seg, off.prev);
    return const_cast<SymHeap &>(sh).ptrAt(addr);
}

/// return the value of 'next' in the given segment (given by root)
inline TValId nextValFromSeg(SymHeap &sh, TValId seg) {
    const TObjId ptrNext = nextPtrFromSeg(sh, seg);
    const TValId valNext = sh.valueOf(ptrNext);
    sh.objReleaseId(ptrNext);
    return valNext;
}

/// return DLS peer object of the given DLS
inline TValId dlSegPeer(const SymHeap &sh, TValId dls) {
    CL_BREAK_IF(sh.valOffset(dls));
    CL_BREAK_IF(OK_DLS != sh.valTargetKind(dls));
    const BindingOff &off = sh.segBinding(dls);
    const TValId peer = valOfPtrAt(const_cast<SymHeap &>(sh), dls, off.prev);
    return sh.valRoot(peer);
}

/// return DLS peer object in case of DLS, the given value otherwise
inline TValId segPeer(const SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(!isAbstract(sh.valTarget(seg)));
    return (OK_DLS == sh.valTargetKind(seg))
        ? dlSegPeer(sh, seg)
        : seg;
}

/// return address of segment's head (useful mainly for Linux lists)
inline TValId segHeadAt(const SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));
    CL_BREAK_IF(VT_ABSTRACT != sh.valTarget(seg));

    const BindingOff &off = sh.segBinding(seg);
    return const_cast<SymHeap &>(sh).valByOffset(seg, off.head);
}

/// we do NOT require root to be a segment
inline TValId segNextRootObj(SymHeap &sh, TValId at, TOffset offNext) {
    CL_BREAK_IF(sh.valOffset(at));
    if (OK_DLS == sh.valTargetKind(at))
        // jump to peer in case of DLS
        at = dlSegPeer(sh, at);

    return nextRootObj(sh, at, offNext);
}

/// we DO require the root to be a segment
inline TValId segNextRootObj(SymHeap &sh, TValId root) {
    CL_BREAK_IF(sh.valOffset(root));

    const BindingOff off = sh.segBinding(root);
    const TOffset offNext = (OK_DLS == sh.valTargetKind(root))
        ? off.prev
        : off.next;

    return segNextRootObj(sh, root, offNext);
}

inline unsigned objMinLength(const SymHeap &sh, TValId root) {
    CL_BREAK_IF(sh.valOffset(root));

    const EValueTarget code = sh.valTarget(root);
    if (isAbstract(code))
        // abstract target
        return sh.segMinLength(root);

    else if (isPossibleToDeref(code))
        // concrete target
        return 1;

    else
        // no target here
        return 0;
}

/// same as SymHeap::objSetProto(), but takes care of DLS peers
void segSetProto(SymHeap &sh, TValId seg, bool isProto);

TValId segClone(SymHeap &sh, const TValId seg);

/// destroy the given list segment object (including DLS peer in case of DLS)
void segDestroy(SymHeap &sh, TObjId seg);

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
        SymHeap                 &sh,
        const TValId            at,
        const BindingOff        &off)
{
    const TObjId next = sh.ptrAt(sh.valByOffset(at, off.next));
    if (OBJ_INVALID != next)
        ignoreList.insert(next);

    const TObjId prev = sh.ptrAt(sh.valByOffset(at, off.prev));
    if (OBJ_INVALID != prev)
        ignoreList.insert(prev);
}

/**
 * returns true if all DLS in the given symbolic heap are consistent
 * @note this runs in debug build only
 */
bool dlSegCheckConsistency(const SymHeap &sh);

#endif /* H_GUARD_SYMSEG_H */
