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
    // validate call of nextPtrFromSeg()
    SE_BREAK_IF(OK_CONCRETE == sh.objKind(seg));

    const TFieldIdxChain icNext = sh.objBinding(seg).next;
    return subObjByChain(sh, seg, icNext);
}

TObjId peerPtrFromSeg(const SymHeap &sh, TObjId seg) {
    SE_BREAK_IF(OK_DLS != sh.objKind(seg));

    const TFieldIdxChain icPeer = sh.objBinding(seg).peer;
    return subObjByChain(sh, seg, icPeer);
}

TObjId dlSegPeer(const SymHeap &sh, TObjId dls) {
    // validate call of dlSegPeer()
    const TObjId root = objRoot(sh, dls);
    SE_BREAK_IF(OK_DLS != sh.objKind(root));

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

        // equal basically means 'invalid segment'
        SE_BREAK_IF(eq);

        return /* not equal */ true;
    }
}

bool dlSegNotEmpty(const SymHeap &sh, TObjId dls) {
    // validate call of dlSegNotEmpty()
    SE_BREAK_IF(OK_DLS != sh.objKind(dls));

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

#if SE_SELF_TEST
    if (!ne1 && !ne2)
        return /* possibly empty */ false;

    // the given DLS is guaranteed to be non empty in one direction, but not
    // vice versa --> such a DLS is considered as mutant and should not be
    // passed through
    SE_TRAP;
#endif
    return false;
}

bool segNotEmpty(const SymHeap &sh, TObjId seg) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_CONCRETE:
        case OK_HEAD:
        case OK_PART:
            // invalid call of segNotEmpty()
            SE_TRAP;

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
        case OK_PART:
            // invalid call of segDestroy()
            SE_TRAP;

        case OK_DLS:
            sh.objDestroy(dlSegPeer(sh, seg));
            // fall through!

        case OK_SLS:
            sh.objDestroy(seg);
    }
}

bool haveSeg(const SymHeap &sh, TValueId atAddr, TValueId pointingTo,
             const EObjKind kind)
{
    if (UV_ABSTRACT != sh.valGetUnknown(atAddr))
        // not an abstract object
        return false;

    TObjId seg = objRoot(sh, sh.pointsTo(atAddr));
    if (kind != sh.objKind(seg))
        // kind mismatch
        return false;

    if (OK_DLS == kind) {
        seg = dlSegPeer(sh, seg);
        if (OK_DLS != sh.objKind(seg))
            // invalid peer
            return false;
    }

    // compare the end-points
    const TObjId nextPtr = nextPtrFromSeg(sh, seg);
    const TValueId valNext = sh.valueOf(nextPtr);
    return (valNext == pointingTo);
}

bool haveDlSegAt(const SymHeap &sh, TValueId atAddr, TValueId peerAddr) {
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
