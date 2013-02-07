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

TValId valFromSegAddr(
        const SymHeap              &sh,
        const TValId                addr,
        const bool                  backward)
{
    const TObjId seg = sh.objByAddr(addr);
    if (OK_OBJ_OR_NULL == sh.objKind(seg)) {
        CL_BREAK_IF(backward);
        return VAL_NULL;
    }

    const ETargetSpecifier ts = sh.targetSpec(addr);
    const bool isLast = (TS_LAST == ts);

    const PtrHandle ptr = (backward == isLast)
        ? nextPtrFromSeg(sh, seg)
        : prevPtrFromSeg(sh, seg);

    return ptr.value();
}

TValId nextValFromSegAddr(const SymHeap &sh, const TValId addr)
{
    return valFromSegAddr(sh, addr, /* backward */ false);
}

TValId prevValFromSegAddr(const SymHeap &sh, const TValId addr)
{
    return valFromSegAddr(sh, addr, /* backward */ true);
}

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

    if (obj2 != obj1)
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

    if (peer != seg)
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

    if (!isAbstractObject(sh, obj1) && !isAbstractObject(sh, obj2))
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

        const TObjId seg = sh.objByAddr(val);
        if (!isAbstractObject(sh, seg))
            // a non-abstract object reached
            break;

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
