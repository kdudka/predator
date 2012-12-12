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
bool haveSeg(
        const SymHeap               &sh,
        const TValId                 atAddr,
        const TValId                 pointingTo,
        const EObjKind               kind);

/**
 * return true if there is a DLS (Doubly-linked List Segment) among the given
 * pair of values
 * @param sh an instance of symbolic heap
 * @param atAddr address of the heap object for consideration
 * @param peerAddr potential address of DLS peer object
 */
bool haveDlSegAt(const SymHeap &sh, TValId atAddr, TValId peerAddr);

/// return 'next' pointer in the given segment
inline PtrHandle nextPtrFromSeg(const SymHeap &sh, TObjId seg)
{
    CL_BREAK_IF(OK_REGION == sh.objKind(seg));

    const BindingOff &off = sh.segBinding(seg);
    return PtrHandle(const_cast<SymHeap &>(sh), seg, off.next);
}

/// return 'prev' pointer in the given segment
inline PtrHandle prevPtrFromSeg(const SymHeap &sh, TObjId seg)
{
    CL_BREAK_IF(OK_REGION == sh.objKind(seg));

    const BindingOff &off = sh.segBinding(seg);
    return PtrHandle(const_cast<SymHeap &>(sh), seg, off.prev);
}

/// return the value of 'next' in the given segment
inline TValId nextValFromSeg(const SymHeap &sh, TObjId seg)
{
    if (OK_OBJ_OR_NULL == sh.objKind(seg))
        return VAL_NULL;

    const FldHandle ptrNext = nextPtrFromSeg(sh, seg);
    return ptrNext.value();
}

TValId nextValFromSegAddr(const SymHeap &sh, const TValId addr);

TValId prevValFromSegAddr(const SymHeap &sh, const TValId addr);

inline TOffset headOffset(const SymHeap &sh, const TObjId seg)
{
    const EObjKind kind = sh.objKind(seg);
    CL_BREAK_IF(OK_REGION == kind);

    return (OK_OBJ_OR_NULL == kind)
        ? 0
        : sh.segBinding(seg).head;
}

/// return address of segment's head (useful mainly for Linux lists)
inline TValId segHeadAt(SymHeap &sh, TObjId seg, ETargetSpecifier ts)
{
    CL_BREAK_IF(OK_REGION == sh.objKind(seg));

    const BindingOff &off = sh.segBinding(seg);

    SymHeap &shWritable = const_cast<SymHeap &>(sh);
    return shWritable.addrOfTarget(seg, ts, off.head);
}

/// we require obj to be an abstract object
inline TObjId segNextObj(SymHeap &sh, TObjId obj)
{
    const EObjKind kind = sh.objKind(obj);
    if (OK_OBJ_OR_NULL == kind)
        return OBJ_NULL;

    const BindingOff off = sh.segBinding(obj);
    const TOffset offNext = (OK_DLS == kind)
        ? off.prev
        : off.next;

    return nextObj(sh, obj, offNext);
}

inline TMinLen objMinLength(const SymHeap &sh, TObjId obj)
{
    if (!sh.isValid(obj))
        return 0;

    const EObjKind kind = sh.objKind(obj);
    if (OK_REGION == kind)
        return 1;

    return sh.segMinLength(obj);
}

/// return true if the given pair of values is proven to be non-equal
bool segProveNeq(const SymHeap &sh, TValId v1, TValId v2);

/// if the current segment min length is lower than the given one, update it!
inline void segIncreaseMinLength(SymHeap &sh, const TObjId seg, TMinLen len)
{
    CL_BREAK_IF(!len);

    if (sh.segMinLength(seg) < len)
        sh.segSetMinLength(seg, len);
}

/// we know (v1 != v2), update related segments in the given heap accordingly!
bool segApplyNeq(SymHeap &sh, TValId v1, TValId v2);

inline bool isObjWithBinding(const EObjKind kind)
{
    switch (kind) {
        case OK_REGION:
        case OK_OBJ_OR_NULL:
            return false;

        case OK_SLS:
        case OK_DLS:
        case OK_SEE_THROUGH:
        case OK_SEE_THROUGH_2N:
            break;
    }

    return true;
}

inline void buildIgnoreList(
        TFldSet                 &ignoreList,
        const SymHeap           &sh,
        const TObjId            obj)
{
    SymHeap &writable = const_cast<SymHeap &>(sh);
    TOffset off;
    FldHandle tmp;

    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_REGION:
        case OK_OBJ_OR_NULL:
            return;

        case OK_DLS:
        case OK_SEE_THROUGH_2N:
            // preserve 'peer' field
            off = sh.segBinding(obj).prev;
            tmp = PtrHandle(writable, obj, off);
            ignoreList.insert(tmp);
            // fall through!

        case OK_SLS:
        case OK_SEE_THROUGH:
            // preserve 'next' field
            off = sh.segBinding(obj).next;
            tmp = PtrHandle(writable, obj, off);
            ignoreList.insert(tmp);
    }
}

inline void buildIgnoreList(
        TFldSet                 &ignoreList,
        SymHeap                 &sh,
        const TObjId            obj,
        const BindingOff        &bf)
{
    const PtrHandle next(sh, obj, bf.next);
    if (next.isValidHandle())
        ignoreList.insert(next);

    const PtrHandle prev(sh, obj, bf.prev);
    if (prev.isValidHandle())
        ignoreList.insert(prev);
}

/// look through possibly empty objects and return the value seen
TValId lookThrough(const SymHeap &sh, TValId val, TValSet *pSeen = 0);

/// true if all segments have populated next/prev pointers
bool segCheckConsistency(const SymHeap &sh);

#endif /* H_GUARD_SYMSEG_H */
