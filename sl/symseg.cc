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

TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg) {
    if (OK_HEAD == sh.objKind(seg))
        // jump to root
        seg = objRoot(sh, seg);

    // validate call of nextPtrFromSeg()
    CL_BREAK_IF(OK_CONCRETE == sh.objKind(seg));

    const TOffset offNext = sh.objBinding(seg).next;
    return ptrObjByOffset(sh, seg, offNext);
}

TObjId peerPtrFromSeg(const SymHeap &sh, TObjId seg) {
    CL_BREAK_IF(OK_DLS != sh.objKind(seg));

    const TOffset offPeer = sh.objBinding(seg).prev;
    return ptrObjByOffset(sh, seg, offPeer);
}

TObjId dlSegPeer(const SymHeap &sh, TObjId dls) {
    // validate call of dlSegPeer()
    const TObjId root = objRoot(sh, dls);
    CL_BREAK_IF(OK_DLS != sh.objKind(root));

    TObjId peer = root;
    const BindingOff &off = sh.objBinding(dls);
    skipObj(sh, &peer, off.prev);
    return peer;
}

unsigned dlSegMinLength(const SymHeap &sh, TObjId dls) {
    // validate call of dlSegNotEmpty()
    CL_BREAK_IF(OK_DLS != sh.objKind(dls));

    const TObjId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // red the values (addresses of the surround)
    const TValId val1 = sh.valueOf(next1);
    const TValId val2 = sh.valueOf(next2);

    // attempt to prove both
    const bool ne1 = sh.SymHeapCore::proveNeq(val1, segHeadAddr(sh, peer));
    const bool ne2 = sh.SymHeapCore::proveNeq(val2, segHeadAddr(sh, dls));

    // DLS cross Neq predicates have to be fully symmetric
    CL_BREAK_IF(ne1 != ne2);
    const bool ne = (ne1 && ne2);

    // if DLS heads are two distinct objects, we have at least two objects
    const TValId head1 = segHeadAddr(sh, dls);
    const TValId head2 = segHeadAddr(sh, peer);
    if (sh.SymHeapCore::proveNeq(head1, head2)) {
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
    const TValId nextVal = sh.valueOf(next);
    const TValId headAddr = segHeadAddr(sh, seg);
    return static_cast<unsigned>(sh.SymHeapCore::proveNeq(headAddr, nextVal));
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
            if (!sh.valDestroyTarget(sh.placedAt(dlSegPeer(sh, seg))))
                CL_BREAK_IF("failed to destroy DLS peer");
            // fall through!

        case OK_MAY_EXIST:
        case OK_SLS:
            if (!sh.valDestroyTarget(sh.placedAt(seg)))
                CL_BREAK_IF("failed to destroy segment");
    }
}

bool haveSeg(const SymHeap &sh, TValId atAddr, TValId pointingTo,
             const EObjKind kind)
{
    if ((atAddr <= 0) || UV_ABSTRACT != sh.valGetUnknown(atAddr))
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
    const TValId valNext = sh.valueOf(nextPtr);
    return (valNext == pointingTo);
}

bool haveDlSegAt(const SymHeap &sh, TValId atAddr, TValId peerAddr) {
    if (atAddr <= 0 || peerAddr <= 0)
        // no valid targets
        return false;

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
    const TValId valNext = sh.valueOf(next);

    const TValId headAddr = segHeadAddr(sh, seg);
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
    const TValId a1 = segHeadAddr(sh, dls);
    const TValId a2 = segHeadAddr(sh, peer);
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
        const TOffset offpSeg  = sh.objBinding(dupSeg).prev;
        const TOffset offpPeer = sh.objBinding(dupPeer).prev;

        // resolve selectors -> sub-objects
        const TObjId ppSeg  = ptrObjByOffset(sh, dupSeg , offpSeg);
        const TObjId ppPeer = ptrObjByOffset(sh, dupPeer, offpPeer);

        // now cross the 'peer' pointers
        sh.objSetValue(ppSeg, segHeadAddr(sh, dupPeer));
        sh.objSetValue(ppPeer, segHeadAddr(sh, dupSeg));
    }

    return dupSeg;
}

bool dlSegCheckConsistency(const SymHeap &sh) {
    TObjList roots;
    sh.gatherRootObjs(roots);
    BOOST_FOREACH(const TObjId obj, roots) {
        const EObjKind kind = sh.objKind(obj);
        switch (kind) {
            case OK_HEAD:
                CL_ERROR("OK_HEAD appears among root objects");
                return false;

            case OK_PART:
                CL_ERROR("OK_PART appears among root objects");
                return false;

            case OK_SLS:
            case OK_CONCRETE:
            case OK_MAY_EXIST:
                // we are interested in OK_DLS here
                continue;

            case OK_DLS:
                break;
        }

        if (sh.cVar(0, obj)) {
            CL_ERROR("OK_DLS on stack detected");
            return false;
        }

        if (sh.placedAt(obj) <= 0) {
            CL_ERROR("OK_DLS with invalid address detected");
            return false;
        }

        const TObjId peer = dlSegPeer(sh, obj);
        if (peer <= 0) {
            CL_ERROR("OK_DLS with invalid peer detected");
            return false;
        }

        if (OK_DLS != sh.objKind(peer)) {
            CL_ERROR("DLS peer not a DLS");
            return false;
        }
    }

    // all OK
    return true;
}

