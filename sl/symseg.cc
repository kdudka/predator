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
            sh.valDestroyTarget(dlSegPeer(sh, segAt));
            // fall through!

        case OK_MAY_EXIST:
        case OK_SLS:
            sh.valDestroyTarget(segAt);
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
    SymHeap &writable = const_cast<SymHeap &>(sh);
    const TValId valNext = nextValFromSeg(writable, seg);
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

        // release object IDs
        sh.objReleaseId(ppSeg);
        sh.objReleaseId(ppPeer);
    }

    return dup;
}

bool dlSegCheckConsistency(const SymHeap &sh) {
    TValList addrs;
    sh.gatherRootObjects(addrs, isAbstract);
    BOOST_FOREACH(const TValId at, addrs) {
        if (OK_DLS != sh.valTargetKind(at))
            // we are interested in OK_DLS here
            continue;

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

        // check the consistency of Neq predicates
        const unsigned len1 = sh.segMinLength(at);
        const unsigned len2 = sh.segMinLength(peer);
        if (len1 != len2) {
            CL_ERROR("peer of a DLS " << len1 << "+ is a DLS" << len2 << "+");
            return false;
        }
    }

    // all OK
    return true;
}

