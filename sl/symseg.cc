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

#include "symheap.hh"
#include "symutil.hh"

#include <boost/foreach.hpp>

unsigned dlSegMinLength(const SymHeap &sh, TValId dls) {
    // validate call of dlSegNotEmpty()
    CL_BREAK_IF(OK_DLS != sh.valTargetKind(dls));

    const TValId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // red the values (addresses of the surround)
    const TValId val1 = sh.valueOf(next1);
    const TValId val2 = sh.valueOf(next2);

    // attempt to prove both
    const bool ne1 = sh.SymHeapCore::proveNeq(val1, segHeadAt(sh, peer));
    const bool ne2 = sh.SymHeapCore::proveNeq(val2, segHeadAt(sh, dls));

    // DLS cross Neq predicates have to be fully symmetric
    CL_BREAK_IF(ne1 != ne2);
    const bool ne = (ne1 && ne2);

    // if DLS heads are two distinct objects, we have at least two objects
    const TValId head1 = segHeadAt(sh, dls);
    const TValId head2 = segHeadAt(sh, peer);
    if (sh.SymHeapCore::proveNeq(head1, head2)) {
        CL_BREAK_IF(!ne);
        return /* DLS 2+ */ 2;
    }

    return static_cast<unsigned>(ne);
}

unsigned segMinLength(const SymHeap &sh, TValId seg) {
    CL_BREAK_IF(sh.valOffset(seg));

    const EObjKind kind = sh.valTargetKind(seg);
    switch (kind) {
        case OK_CONCRETE:
            CL_TRAP;

        case OK_MAY_EXIST:
            return 0;

        case OK_SLS:
            break;

        case OK_DLS:
            return dlSegMinLength(sh, seg);
    }

    const TObjId next = nextPtrFromSeg(/* XXX */const_cast<SymHeap &>(sh), seg);
    const TValId nextVal = sh.valueOf(next);
    const TValId headAddr = segHeadAt(/* XXX */const_cast<SymHeap &>(sh), seg);
    return static_cast<unsigned>(sh.SymHeapCore::proveNeq(headAddr, nextVal));
}

void segSetProto(SymHeap &sh, TValId seg, bool isProto) {
    CL_BREAK_IF(sh.valOffset(seg));

    const EObjKind kind = sh.valTargetKind(seg);
    switch (kind) {
        case OK_DLS:
            sh.valTargetSetProto(dlSegPeer(sh, seg), isProto);
            // fall through

        case OK_SLS:
        case OK_MAY_EXIST:
            sh.valTargetSetProto(seg, isProto);
            break;

        default:
            CL_BREAK_IF("ivalid call of segSetProto()");
    }
}

void segDestroy(SymHeap &sh, TObjId seg) {
    const TValId segAt = sh.valRoot(sh.placedAt(seg));

    const EObjKind kind = sh.valTargetKind(segAt);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of segDestroy()
            CL_TRAP;

        case OK_DLS:
            if (!sh.valDestroyTarget(dlSegPeer(sh, segAt)))
                CL_BREAK_IF("failed to destroy DLS peer");
            // fall through!

        case OK_MAY_EXIST:
        case OK_SLS:
            if (!sh.valDestroyTarget(segAt))
                CL_BREAK_IF("failed to destroy segment");
    }
}

bool haveSeg(const SymHeap &sh, TValId atAddr, TValId pointingTo,
             const EObjKind kind)
{
    if (VT_ABSTRACT != sh.valTarget(atAddr))
        // not an abstract object
        return false;

    TValId seg = sh.valRoot(atAddr);
    if (kind != sh.valTargetKind(seg))
        // kind mismatch
        return false;

    if (OK_DLS == kind) {
        seg = dlSegPeer(sh, seg);
        if (OK_DLS != sh.valTargetKind(seg))
            // invalid peer
            return false;
    }

    // compare the end-points
    const TObjId nextPtr = nextPtrFromSeg(sh, seg);
    const TValId valNext = sh.valueOf(nextPtr);
    return (valNext == pointingTo);
}

bool haveDlSegAt(const SymHeap &sh, TValId atAddr, TValId peerAddr) {
    if (atAddr <= 0 || peerAddr <= 0)
        // no valid targets
        return false;

    if (VT_ABSTRACT != sh.valTarget(atAddr)
            || VT_ABSTRACT != sh.valTarget(peerAddr))
        // not abstract objects
        return false;

    const TValId seg = sh.valRoot(atAddr);
    if (OK_DLS != sh.valTargetKind(seg))
        // not a DLS
        return false;

    const TValId peer = dlSegPeer(sh, seg);
    if (OK_DLS != sh.valTargetKind(peer))
        // invalid peer
        return false;

    // compare the end-points
    return (segHeadAt(sh, peer) == peerAddr);
}

bool haveMayExistAt(SymHeap &sh, TValId at, TValId pointingTo) {
    const EObjKind kind = sh.valTargetKind(at);
    if (OK_MAY_EXIST != kind)
        return false;

    // jump to the root of OK_MAY_EXIST and read binding offsets
    const TValId seg = sh.valRoot(at);
    const BindingOff bf = sh.segBinding(seg);

    // compute the resulting offset and shift 'pointingTo' accordingly
    const TOffset off = sh.valOffset(at) - bf.head;
    pointingTo = sh.valByOffset(pointingTo, off);

    // finally compare the 'next' value
    const TValId valNext = valOfPtrAt(sh, seg, bf.next);
    return (valNext == pointingTo);
}

void segHandleNeq(SymHeap &sh, TValId seg, TValId peer, SymHeap::ENeqOp op) {
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValId valNext = sh.valueOf(next);

    const TValId headAddr = segHeadAt(sh, seg);
    sh.neqOp(op, headAddr, valNext);
}

void dlSegSetMinLength(SymHeap &sh, TValId dls, unsigned len) {
    const TValId peer = dlSegPeer(sh, dls);
    switch (len) {
        case 0:
            segHandleNeq(sh, dls, peer, SymHeap::NEQ_DEL);
            return;

        case 1:
            segHandleNeq(sh, dls, peer, SymHeap::NEQ_ADD);
            return;

        case 2:
        default:
            break;
    }

    // let it be DLS 2+
    const TValId a1 = segHeadAt(sh, dls);
    const TValId a2 = segHeadAt(sh, peer);
    sh.neqOp(SymHeap::NEQ_ADD, a1, a2);
}

void segSetMinLength(SymHeap &sh, TValId seg, unsigned len) {
    const EObjKind kind = sh.valTargetKind(seg);
    switch (kind) {
        case OK_SLS:
            segHandleNeq(sh, seg, seg, (len)
                    ? SymHeap::NEQ_ADD
                    : SymHeap::NEQ_DEL);
            break;

        case OK_DLS:
            dlSegSetMinLength(sh, seg, len);
            break;

        case OK_MAY_EXIST:
            if (!len)
                break;
            // fall through!

        default:
            CL_BREAK_IF("ivalid call of segSetMinLength()");
            break;
    }
}

TValId segClone(SymHeap &sh, const TValId seg) {
    const TValId dup = sh.valClone(seg);

    if (OK_DLS == sh.valTargetKind(seg)) {
        // we need to clone the peer as well
        const TValId peer = dlSegPeer(sh, seg);
        const TValId dupPeer = sh.valClone(peer);

        // dig the 'peer' selectors of the cloned objects
        const TOffset offpSeg  = sh.segBinding(dup).prev;
        const TOffset offpPeer = sh.segBinding(dupPeer).prev;

        // resolve selectors -> sub-objects
        const TObjId ppSeg  = sh.ptrAt(sh.valByOffset(dup, offpSeg));
        const TObjId ppPeer = sh.ptrAt(sh.valByOffset(dupPeer, offpPeer));

        // now cross the 'peer' pointers
        sh.objSetValue(ppSeg, segHeadAt(sh, dupPeer));
        sh.objSetValue(ppPeer, segHeadAt(sh, dup));
    }

    return dup;
}

bool dlSegCheckConsistency(const SymHeap &sh) {
    TValList addrs;
    sh.gatherRootObjects(addrs, isAbstract);
    BOOST_FOREACH(const TValId at, addrs) {
        const EObjKind kind = sh.valTargetKind(at);
        switch (kind) {
            case OK_SLS:
            case OK_CONCRETE:
            case OK_MAY_EXIST:
                // we are interested in OK_DLS here
                continue;

            case OK_DLS:
                break;
        }

        if (at <= 0) {
            CL_ERROR("OK_DLS with invalid address detected");
            return false;
        }

        const TValId peer = dlSegPeer(sh, at);
        if (peer <= 0) {
            CL_ERROR("OK_DLS with invalid peer detected");
            return false;
        }

        if (OK_DLS != sh.valTargetKind(peer)) {
            CL_ERROR("DLS peer not a DLS");
            return false;
        }
    }

    // all OK
    return true;
}

