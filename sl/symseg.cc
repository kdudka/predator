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

#include "symheap.hh"
#include "symutil.hh"

TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg) {
    if (OK_CONCRETE == sh.objKind(seg))
        // invalid call of nextPtrFromSeg()
        TRAP;

    const TFieldIdxChain icNext = sh.objBinding(seg).next;
    return subObjByChain(sh, seg, icNext);
}

TObjId dlSegPeer(const SymHeap &sh, TObjId dls) {
    const TObjId root = objRoot(sh, dls);
    if (OK_DLS != sh.objKind(root))
        // invalid call of dlSegPeer()
        TRAP;

    TObjId peer = root;
    const SegBindingFields &bf = sh.objBinding(dls);
    skipObj(sh, &peer, bf.head, bf.peer);
    return peer;
}

namespace {
    bool segProveNeq(const SymHeap &sh, TValueId v1, TValueId v2) {
        bool eq;
        if (!sh.proveEq(&eq, v1, v2))
            return /* no idea */ false;

        if (eq)
            // equal ... basically means 'invalid segment'
            TRAP;

        return /* not equal */ true;
    }
}

bool dlSegNotEmpty(const SymHeap &sh, TObjId dls) {
    if (OK_DLS != sh.objKind(dls))
        // invalid call of dlSegNotEmpty()
        TRAP;

    const TObjId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // red the values (addresses of the surround)
    const TValueId val1 = sh.valueOf(next1);
    const TValueId val2 = sh.valueOf(next2);

    // attempt to prove both
    const bool ne1 = segProveNeq(sh, val1, segHeadAddr(sh, peer));
    const bool ne2 = segProveNeq(sh, val2, segHeadAddr(sh, dls));
    if (ne1 && ne2)
        return /* not empty */ true;

    if (!ne1 && !ne2)
        return /* possibly empty */ false;

    // the given DLS is guaranteed to be non empty in one direction, but not
    // vice versa --> such a DLS is considered as mutant and should not be
    // passed through
    TRAP;
    return false;
}

bool segNotEmpty(const SymHeap &sh, TObjId seg) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_CONCRETE:
        case OK_HEAD:
            // invalid call of segNotEmpty()
            TRAP;

        case OK_SLS:
            break;

        case OK_DLS:
            return dlSegNotEmpty(sh, seg);
    }

    const TObjId next = nextPtrFromSeg(sh, seg);
    const TValueId nextVal = sh.valueOf(next);
    const TValueId headAddr = segHeadAddr(sh, seg);
    return /* not empty */ segProveNeq(sh, headAddr, nextVal);
}

void segDestroy(SymHeap &sh, TObjId seg) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_CONCRETE:
        case OK_HEAD:
            // invalid call of segDestroy()
            TRAP;

        case OK_DLS:
            sh.objDestroy(dlSegPeer(sh, seg));
            // fall through!

        case OK_SLS:
            sh.objDestroy(seg);
    }
}


bool haveDlSeg(SymHeap &sh, TValueId atAddr, TValueId pointingTo) {
    if (UV_ABSTRACT != sh.valGetUnknown(atAddr))
        // not an abstract object
        return false;

    const TObjId seg = objRoot(sh, sh.pointsTo(atAddr));
    if (OK_DLS != sh.objKind(seg))
        // not a DLS
        return false;

    const TObjId peer = dlSegPeer(sh, seg);
    if (OK_DLS != sh.objKind(peer))
        // invalid peer
        return false;

    // compare the end-points
    const TObjId nextPtr = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(nextPtr);
    return (valNext == pointingTo);
}

bool haveDlSegAt(SymHeap &sh, TValueId atAddr, TValueId peerAddr) {
    if (UV_ABSTRACT != sh.valGetUnknown(atAddr)
            || UV_ABSTRACT != sh.valGetUnknown(peerAddr))
        // not abstract objects
        return false;

    const TObjId seg = objRoot(sh, sh.pointsTo(atAddr));
    if (OK_DLS != sh.objKind(seg))
        // not a DLS
        return false;

    const TObjId peer = dlSegPeer(sh, seg);
    if (OK_DLS != sh.objKind(peer))
        // invalid peer
        return false;

    // compare the end-points
    return (segHeadAddr(sh, peer) == peerAddr);
}
