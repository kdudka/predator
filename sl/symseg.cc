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
    if (OK_HEAD == sh.objKind(seg))
        // jump to root
        seg = objRoot(sh, seg);

    // validate call of nextPtrFromSeg()
    CL_BREAK_IF(OK_CONCRETE == sh.objKind(seg));

    const TFieldIdxChain icNext = sh.objBinding(seg).next;
    return subObjByChain(sh, seg, icNext);
}

TObjId peerPtrFromSeg(const SymHeap &sh, TObjId seg) {
    CL_BREAK_IF(OK_DLS != sh.objKind(seg));

    const TFieldIdxChain icPeer = sh.objBinding(seg).peer;
    return subObjByChain(sh, seg, icPeer);
}

TObjId dlSegPeer(const SymHeap &sh, TObjId dls) {
    // validate call of dlSegPeer()
    const TObjId root = objRoot(sh, dls);
    CL_BREAK_IF(OK_DLS != sh.objKind(root));

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
        CL_BREAK_IF(eq);

        return /* not equal */ true;
    }
}

unsigned dlSegMinLength(const SymHeap &sh, TObjId dls) {
    // validate call of dlSegNotEmpty()
    CL_BREAK_IF(OK_DLS != sh.objKind(dls));

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

    // DLS cross Neq predicates have to be fully symmetric
    CL_BREAK_IF(ne1 != ne2);
    const bool ne = (ne1 && ne2);

    // if DLS heads are two distinct objects, we have at least two objects
    const TValueId head1 = segHeadAddr(sh, dls);
    const TValueId head2 = segHeadAddr(sh, peer);
    if (segProveNeq(sh, head1, head2)) {
        CL_BREAK_IF(!ne);
        return /* DLS 2+ */ 2;
    }

    return static_cast<unsigned>(ne);
}

unsigned segMinLength(const SymHeap &sh, TObjId seg) {
    EObjKind kind = sh.objKind(seg);
    if (OK_HEAD == kind) {
        seg = objRoot(sh, seg);
        kind = sh.objKind(seg);
    }

    switch (kind) {
        case OK_CONCRETE:
        case OK_PART:
        case OK_HEAD:
            CL_TRAP;

        case OK_MAY_EXIST:
            return 0;

        case OK_SLS:
            break;

        case OK_DLS:
            return dlSegMinLength(sh, seg);
    }

    const TObjId next = nextPtrFromSeg(sh, seg);
    const TValueId nextVal = sh.valueOf(next);
    const TValueId headAddr = segHeadAddr(sh, seg);
    return static_cast<unsigned>(segProveNeq(sh, headAddr, nextVal));
}

void segSetProto(SymHeap &sh, TObjId seg, bool isProto) {
    EObjKind kind = sh.objKind(seg);
    if (OK_HEAD == kind) {
        seg = objRoot(sh, seg);
        kind = sh.objKind(seg);
    }

    switch (kind) {
        case OK_DLS:
            sh.objSetProto(dlSegPeer(sh, seg), isProto);
            // fall through

        case OK_SLS:
        case OK_MAY_EXIST:
            sh.objSetProto(seg, isProto);
            break;

        default:
            CL_BREAK_IF("ivalid call of segSetProto()");
    }
}

void segDestroy(SymHeap &sh, TObjId seg) {
    EObjKind kind = sh.objKind(seg);
    if (OK_HEAD == kind) {
        seg = objRoot(sh, seg);
        kind = sh.objKind(seg);
    }

    switch (kind) {
        case OK_CONCRETE:
        case OK_PART:
        case OK_HEAD:
            // invalid call of segDestroy()
            CL_TRAP;

        case OK_DLS:
            sh.objDestroy(dlSegPeer(sh, seg));
            // fall through!

        case OK_MAY_EXIST:
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

void segHandleNeq(SymHeap &sh, TObjId seg, TObjId peer, SymHeap::ENeqOp op) {
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);

    const TValueId headAddr = segHeadAddr(sh, seg);
    sh.neqOp(op, headAddr, valNext);
}

void dlSegSetMinLength(SymHeap &sh, TObjId dls, unsigned len) {
    const TObjId peer = dlSegPeer(sh, dls);
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
    const TValueId a1 = segHeadAddr(sh, dls);
    const TValueId a2 = segHeadAddr(sh, peer);
    sh.neqOp(SymHeap::NEQ_ADD, a1, a2);
}

void segSetMinLength(SymHeap &sh, TObjId seg, unsigned len) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_SLS:
            segHandleNeq(sh, seg, seg, (len)
                    ? SymHeap::NEQ_ADD
                    : SymHeap::NEQ_DEL);
            break;

        case OK_HEAD:
            seg = objRoot(sh, seg);
            // fall through!

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

TObjId segClone(SymHeap &sh, const TObjId seg) {
    const TObjId dupSeg = sh.objDup(seg);

    if (OK_DLS == sh.objKind(seg)) {
        // we need to clone the peer as well
        const TObjId peer = dlSegPeer(sh, seg);
        const TObjId dupPeer = sh.objDup(peer);

        // dig the 'peer' selectors of the cloned objects
        const TFieldIdxChain icpSeg  = sh.objBinding(dupSeg).peer;
        const TFieldIdxChain icpPeer = sh.objBinding(dupPeer).peer;

        // resolve selectors -> sub-objects
        const TObjId ppSeg  = subObjByChain(sh, dupSeg , icpSeg);
        const TObjId ppPeer = subObjByChain(sh, dupPeer, icpPeer);

        // now cross the 'peer' pointers
        sh.objSetValue(ppSeg, segHeadAddr(sh, dupPeer));
        sh.objSetValue(ppPeer, segHeadAddr(sh, dupSeg));
    }

    return dupSeg;
}

// works, but not used for now
#if 0
namespace {
void dropHeadIc(
        TFieldIdxChain          &dst,
        const TFieldIdxChain    &icSrc,
        const TFieldIdxChain    &icHead)
{
    const unsigned cnt = icSrc.size();
    const unsigned cntHead = icHead.size();
    for (unsigned i = 0; i < cnt; ++i) {
        if (i < cntHead) {
            CL_BREAK_IF(icHead[i] != icSrc[i]);
            continue;
        }

        dst.push_back(icSrc[i]);
    }
}
} // namespace

TFieldIdxChain nextByHead(const SegBindingFields &bf) {
    TFieldIdxChain dst;
    dropHeadIc(dst, bf.next, bf.head);
    return dst;
}

TFieldIdxChain peerByHead(const SegBindingFields &bf) {
    TFieldIdxChain dst;
    dropHeadIc(dst, bf.peer, bf.head);
    return dst;
}
#endif
