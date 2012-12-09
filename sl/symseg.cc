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

#include "config.h"
#include "symseg.hh"

#include <cl/cl_msg.hh>

#include "prototype.hh"
#include "symheap.hh"
#include "symutil.hh"

#include <boost/foreach.hpp>

bool segProveNeq(const SymHeap &sh, TValId ref, TValId val)
{
    if (proveNeq(sh, ref, val))
        // values are non-equal in non-abstract world
        return true;

    // collect the sets of values we get by jumping over 0+ abstract objects
    TValSet seen1, seen2;
    if (VAL_INVALID == lookThrough(sh, ref, &seen1))
        return false;
    if (VAL_INVALID == lookThrough(sh, val, &seen2))
        return false;

    // try to look through possibly empty abstract objects
    ref = lookThrough(sh, ref, &seen2);
    val = lookThrough(sh, val, &seen1);
    if (ref == val)
        return false;

    if (proveNeq(sh, ref, val))
        // values are non-equal in non-abstract world
        return true;

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(sh, ref, val);

    const TSizeRange size2 = valSizeOfTarget(sh, val);
    if (size2.lo <= IR::Int0)
        // oops, we cannot prove the address is safely allocated, giving up
        return false;

    const TObjId obj2 = sh.objByAddr(val);
    const TMinLen len2 = objMinLength(sh, obj2);
    if (!len2)
        // one of the targets is possibly empty, giving up
        return false;

    if (VAL_NULL == ref)
        // one of them is VAL_NULL the other one is address of non-empty object
        return true;

    const TSizeRange size1 = valSizeOfTarget(sh, ref);
    if (size1.lo <= IR::Int0)
        // oops, we cannot prove the address is safely allocated, giving up
        return false;

    const TObjId obj1 = sh.objByAddr(ref);
    const TMinLen len1 = objMinLength(sh, obj1);
    if (!len1)
        // both targets are possibly empty, giving up
        return false;

    const EObjKind kind1 = sh.objKind(obj1);
    if (OK_REGION == kind1)
        // non-empty abstract object vs. concrete object
        return true;

    // FIXME: this needs to be rewritten once we unimplement DLS peers
    if (obj2 != segPeer(sh, obj1))
        // a pair of non-empty abstract objects
        return true;

    // one value points at segment and the other points at its peer
    CL_BREAK_IF(len1 != len2);
    return (1 < len1);
}

bool haveSeg(
        const SymHeap               &sh,
        const TValId                 atAddr,
        const TValId                 pointingTo,
        const EObjKind               kind)
{
    const TObjId seg = sh.objByAddr(atAddr);
    if (kind != sh.objKind(seg))
        // kind mismatch
        return false;

    // compare the end-points
    const TValId valNext = nextValFromSegAddr(sh, atAddr);
    return (valNext == pointingTo);
}

bool haveDlSegAt(const SymHeap &sh, TValId atAddr, TValId peerAddr)
{
    if (VT_RANGE == sh.valTarget(atAddr) || VT_RANGE == sh.valTarget(peerAddr))
        // VT_RANGE not supported for now
        return false;

    const TObjId seg = sh.objByAddr(atAddr);
    if (OK_DLS != sh.objKind(seg))
        // not a DLS
        return false;

    const TObjId peer = sh.objByAddr(peerAddr);
    if (OK_DLS != sh.objKind(peer))
        // invalid peer
        return false;

    const TOffset off1 = sh.valOffset(atAddr);
    const TOffset off2 = sh.valOffset(peerAddr);
    if (off1 != off2)
        // offset mismatch
        return false;

    if (peer != dlSegPeer(sh, seg))
        // not in a relation
        return false;

    const ETargetSpecifier ts1 = sh.targetSpec(atAddr);
    const ETargetSpecifier ts2 = sh.targetSpec(peerAddr);
    CL_BREAK_IF(TS_ALL == ts1 || TS_ALL == ts2);
    CL_BREAK_IF(ts1 == ts2);
    return (TS_FIRST == ts1 && TS_LAST == ts2)
        || (TS_LAST == ts1 && TS_FIRST == ts2);
}

bool haveSegBidir(
        TObjId                     *pDst,
        const SymHeap              &sh,
        const EObjKind              kind,
        const TValId                v1,
        const TValId                v2)
{
    if (haveSeg(sh, v1, v2, kind)) {
        *pDst = sh.objByAddr(v1);
        return true;
    }

    if (haveSeg(sh, v2, v1, kind)) {
        *pDst = sh.objByAddr(v2);
        return true;
    }

    // found nothing
    return false;
}

bool segApplyNeq(SymHeap &sh, TValId v1, TValId v2)
{
    const TObjId obj1 = sh.objByAddr(v1);
    const TObjId obj2 = sh.objByAddr(v2);

    if (!isAbstractValue(sh, v1) && !isAbstractValue(sh, v2))
        // no abstract objects involved
        return false;

    if (VAL_NULL == v1 && sh.valOffset(v2) == headOffset(sh, obj2))
        v1 = nextValFromSegAddr(sh, v2);

    if (VAL_NULL == v2 && sh.valOffset(v1) == headOffset(sh, obj1))
        v2 = nextValFromSegAddr(sh, v1);

    TObjId seg;
    if (haveSegBidir(&seg, sh, OK_OBJ_OR_NULL, v1, v2)
            || haveSegBidir(&seg, sh, OK_SEE_THROUGH, v1, v2)
            || haveSegBidir(&seg, sh, OK_SEE_THROUGH_2N, v1, v2))
    {
        // replace OK_SEE_THROUGH/OK_OBJ_OR_NULL by OK_CONCRETE
        decrementProtoLevel(sh, seg);
        sh.objSetConcrete(seg);
        return true;
    }

    if (haveSegBidir(&seg, sh, OK_SLS, v1, v2)) {
        segIncreaseMinLength(sh, seg, /* SLS 1+ */ 1);
        return true;
    }

    if (haveSegBidir(&seg, sh, OK_DLS, v1, v2)) {
        segIncreaseMinLength(sh, seg, /* DLS 1+ */ 1);
        return true;
    }

    if (haveDlSegAt(sh, v1, v2)) {
        seg = sh.objByAddr(v1);
        segIncreaseMinLength(sh, seg, /* DLS 2+ */ 2);
        return true;
    }

    // fallback to explicit Neq predicate
    return false;
}

void dlSegRecover(SymHeap &sh, const TObjId obj1, const TObjId obj2)
{
    if (obj1 == obj2)
        return;

    CL_BREAK_IF(isDlSegPeer(sh, obj1) == isDlSegPeer(sh, obj2));

    const FldHandle ptr1 = prevPtrFromSeg(sh, obj1);
    const FldHandle ptr2 = prevPtrFromSeg(sh, obj2);

    ETargetSpecifier ts1 = TS_LAST;
    ETargetSpecifier ts2 = TS_FIRST;
    if (isDlSegPeer(sh, obj1))
        swapValues(ts1, ts2);

    const TValId addr1 = segHeadAt(sh, obj1, ts1);
    const TValId addr2 = segHeadAt(sh, obj2, ts2);

    ptr1.setValue(addr2);
    ptr2.setValue(addr1);
}

TObjId segClone(SymHeap &sh, const TObjId obj)
{
    const TObjId dup = objClone(sh, obj);

    if (OK_DLS == sh.objKind(obj)) {
        // we need to clone the peer as well
        const TObjId peer = dlSegPeer(sh, obj);
        if (peer == obj)
            return dup;

        const TObjId dupPeer = sh.objClone(peer);
        dlSegRecover(sh, dup, dupPeer);
    }

    return dup;
}

TValId lookThrough(const SymHeap &sh, TValId val, TValSet *pSeen)
{
    if (VT_RANGE == sh.valTarget(val))
        // not supported yet
        return VAL_INVALID;

    const TOffset off = sh.valOffset(val);

    while (0 < val) {
        if (pSeen && !insertOnce(*pSeen, val))
            // an already seen value
            break;

        if (!isAbstractValue(sh, val))
            // a non-abstract object reached
            break;

        const TObjId seg = sh.objByAddr(val);

        if (sh.segMinLength(seg))
            // non-empty abstract object reached
            break;

        const EObjKind kind = sh.objKind(seg);
        if (OK_OBJ_OR_NULL == kind) {
            // we always end up with VAL_NULL if OK_OBJ_OR_NULL is removed
            val = VAL_NULL;
            continue;
        }

        // when computing the actual shift, take the head offset into account
        const BindingOff &bOff = sh.segBinding(seg);
        const TOffset shiftBy = off - bOff.head;

        if (OK_DLS == kind) {
            // put the shifted address of DLS peer to the list of seen values
            TValId valPrev = prevValFromSegAddr(sh, val);
            valPrev = const_cast<SymHeap &>(sh).valByOffset(valPrev, shiftBy);
            if (!insertOnce(*pSeen, valPrev))
                return val;
        }

        // jump to next value
        const TValId valNext = nextValFromSegAddr(sh, val);
        val = const_cast<SymHeap &>(sh).valByOffset(valNext, shiftBy);
    }

    return val;
}

bool segCheckConsistency(const SymHeap &sh)
{
    TObjList objs;
    sh.gatherObjects(objs, isOnHeap);
    BOOST_FOREACH(const TObjId seg, objs) {
        bool hasNext = false;
        bool hasPrev = false;

        const EObjKind kind = sh.objKind(seg);
        switch (kind) {
            case OK_REGION:
            case OK_OBJ_OR_NULL:
                break;

            case OK_DLS:
            case OK_SEE_THROUGH_2N:
                hasPrev = true;
                // fall through!

            case OK_SLS:
            case OK_SEE_THROUGH:
                hasNext = true;
        }

        if (hasNext) {
            const TValId valNext = nextValFromSeg(sh, seg);
            const EValueTarget code = sh.valTarget(valNext);
            if (VT_OBJECT != code) {
                CL_ERROR("valNext of #" << seg << " has no target object");
                return false;
            }
        }

        if (hasPrev) {
            const PtrHandle prevPtr = prevPtrFromSeg(sh, seg);
            const TValId valPrev = prevPtr.value();
            const EValueTarget code = sh.valTarget(valPrev);
            if (VT_OBJECT != code) {
                CL_ERROR("valPrev of #" << seg << " has no target object");
                return false;
            }
        }
    }

    // all OK
    return true;
}

bool dlSegCheckConsistency(const SymHeap &sh)
{
    if (!segCheckConsistency(sh))
        return false;

    TObjList objs;
    sh.gatherObjects(objs, isOnHeap);
    BOOST_FOREACH(const TObjId seg, objs) {
        if (OK_DLS != sh.objKind(seg))
            // we are interested in OK_DLS here
            continue;

        const TObjId peer = dlSegPeer(sh, seg);
        if (OBJ_INVALID == peer) {
            CL_ERROR("OK_DLS with invalid peer detected");
            return false;
        }

        if (OK_DLS != sh.objKind(peer)) {
            CL_ERROR("DLS peer not a DLS");
            return false;
        }

        // check the consistency of Neq predicates
        const TMinLen len1 = sh.segMinLength(seg);
        const TMinLen len2 = sh.segMinLength(peer);
        if (len1 != len2) {
            CL_ERROR("peer of a DLS " << len1 << "+ is a DLS" << len2 << "+");
            return false;
        }
    }

    // all OK
    return true;
}
