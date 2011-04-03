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
bool haveSeg(const SymHeap &sh, TValueId atAddr, TValueId pointingTo,
             const EObjKind kind);

/**
 * return true if there is a DLS (Doubly-linked List Segment) among the given
 * pair of values
 * @param sh an instance of symbolic heap
 * @param atAddr address of the heap object for consideration
 * @param peerAddr potential address of DLS peer object
 */
bool haveDlSegAt(const SymHeap &sh, TValueId atAddr, TValueId peerAddr);

/// return 'next' pointer of the given list segment as a heap object
TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg);

/// return 'peer' pointer of the given DLS
TObjId peerPtrFromSeg(const SymHeap &sh, TObjId seg);

/// return DLS peer object of the given DLS
TObjId dlSegPeer(const SymHeap &sh, TObjId dls);

/// return lower estimation of DLS length
unsigned dlSegMinLength(const SymHeap &sh, TObjId dls);

void dlSegSetMinLength(SymHeap &sh, TObjId dls, unsigned len);

/// return lower estimation of segment length
unsigned segMinLength(const SymHeap &sh, TObjId seg);

inline unsigned objMinLength(const SymHeap &sh, TObjId obj) {
    if (obj <= 0)
        return 0;

    return (objIsSeg(sh, obj))
        ? segMinLength(sh, obj)
        : /* OK_CONCRETE */ 1;
}

void segSetMinLength(SymHeap &sh, TObjId seg, unsigned len);

/// same as SymHeap::objSetProto(), but takes care of DLS peers
void segSetProto(SymHeap &sh, TObjId seg, bool isProto);

TObjId segClone(SymHeap &sh, const TObjId seg);

/// destroy the given list segment object (including DLS peer in case of DLS)
void segDestroy(SymHeap &sh, TObjId seg);

/**
 * return the @b head object of the given list segment
 * @note it may return the segment itself in case of regular list segment
 * @note this is mostly useful as soon as @b Linux @b lists are involved
 */
inline TObjId segHead(const SymHeap &sh, TObjId seg) {
    const int offHead = sh.objBinding(seg).head;
    return compObjByOffset(sh, seg, offHead);
}

/**
 * return address of the @b head object of the given list segment
 * @note it may return address of the segment itself in case of regular list
 * segment
 * @note this is mostly useful as soon as @b Linux @b lists are involved
 */
inline TValueId segHeadAddr(const SymHeap &sh, TObjId seg) {
    const TObjId head = segHead(sh, seg);
    return sh.placedAt(head);
}

template <class TIgnoreList>
void buildIgnoreList(
        TIgnoreList             &ignoreList,
        const SymHeap           &sh,
        const TObjId            obj)
{
    TObjId tmp;

    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_HEAD:
        case OK_PART:
            CL_BREAK_IF("invalid call of buildIgnoreList()");
            // fall through!

        case OK_CONCRETE:
            return;

        case OK_DLS:
            // preserve 'peer' field
            tmp = ptrObjByOffset(sh, obj, sh.objBinding(obj).prev);
            ignoreList.insert(tmp);
            // fall through!

        case OK_SLS:
        case OK_MAY_EXIST:
            // preserve 'next' field
            tmp = ptrObjByOffset(sh, obj, sh.objBinding(obj).next);
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
    const TObjId next = ptrObjByOffset(sh, obj, off.next);
    if (OBJ_INVALID != next)
        ignoreList.insert(next);

    const TObjId prev = ptrObjByOffset(sh, obj, off.prev);
    if (OBJ_INVALID != prev)
        ignoreList.insert(prev);
}

/**
 * returns true if all DLS in the given symbolic heap are consistent
 * @note this runs in debug build only
 */
bool dlSegCheckConsistency(const SymHeap &sh);

#endif /* H_GUARD_SYMSEG_H */
