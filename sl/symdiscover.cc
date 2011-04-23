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
#include "symdiscover.hh"

#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "symcmp.hh"
#include "symjoin.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <algorithm>                // for std::copy()
#include <set>

#include <boost/foreach.hpp>

bool matchSegBinding(
        const SymHeap               &sh,
        const TObjId                obj,
        const BindingOff            &offDiscover)
{
    const EObjKind kind = objKind(sh, obj);
    if (OK_CONCRETE == kind)
        // nothing to match actually
        return true;

    const BindingOff off = segBinding(sh, obj);
    if (off.head != offDiscover.head)
        // head mismatch
        return false;

    if (!isDlsBinding(offDiscover)) {
        // OK_SLS
        switch (kind) {
            case OK_MAY_EXIST:
            case OK_SLS:
                return (off.next == offDiscover.next);

            default:
                return false;
        }
    }

    // OK_DLS
    switch (kind) {
        case OK_MAY_EXIST:
            // TODO: check polarity
            return (off.next == offDiscover.next);

        case OK_DLS:
            return (off.next == offDiscover.prev)
                && (off.prev == offDiscover.next);

        default:
            return false;
    }
}

// TODO: rewrite, simplify, and make it easier to follow
bool validatePointingObjects(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                root,
        TObjId                      prev,
        const TObjId                next,
        const TObjList              &protoRoots = TObjList(),
        const bool                  toInsideOnly = false)
{
    const bool isDls = isDlsBinding(off);
    std::set<TObjId> allowedReferers;
    if (OK_DLS == objKind(sh, root))
        // retrieve peer's pointer to this object (if any)
        allowedReferers.insert(dlSegPeer(sh, root));

    if (OK_DLS == objKind(sh, prev))
        // jump to peer in case of DLS
        prev = dlSegPeer(sh, prev);

    // we allow pointers to self at this point, but we require them to be
    // absolutely uniform along the abstraction path -- matchData() should
    // later take care of that
    allowedReferers.insert(root);

    // collect all objects pointing at/inside the object
    // NOTE: we really intend to pass toInsideOnly == false at this point!
    TObjList refs;
    gatherPointingObjects(sh, refs, root, /* toInsideOnly */ false);

    // consider also up-links from nested prototypes
    std::copy(protoRoots.begin(), protoRoots.end(),
              std::inserter(allowedReferers, allowedReferers.begin()));

    // please do not validate the binding pointers as data pointers;  otherwise
    // we might mistakenly abstract SLL with head-pointers of length 2 as DLS!!
    std::set<TObjId> blackList;
    blackList.insert(ptrObjByOffset(sh, root, off.next));
    if (isDls)
        blackList.insert(ptrObjByOffset(sh, root, off.prev));

    const TValId headAddr = sh.placedAt(compObjByOffset(sh, root, off.head));
    const bool rootIsProto = sh.valTargetIsProto(sh.placedAt(root));

    // TODO: move subObjByChain() calls out of the loop
    BOOST_FOREACH(const TObjId obj, refs) {
        if (hasKey(blackList, obj))
            return false;

        if (obj == ptrObjByOffset(sh, prev, off.next))
            continue;

        if (isDls && obj == ptrObjByOffset(sh, next, off.prev))
            continue;

        if (toInsideOnly && (sh.valueOf(obj) == headAddr))
            continue;

        if (hasKey(allowedReferers, objRoot(sh, obj)))
            continue;

        if (!rootIsProto && sh.valTargetIsProto(sh.placedAt(obj)))
            // FIXME: subtle
            continue;

        // someone points at/inside who should not
        return false;
    }

    // no problems encountered
    return true;
}

// FIXME: suboptimal implementation
bool validatePrototypes(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                root,
        TObjList                    protoRoots)
{
    TObjId peer = OBJ_INVALID;
    protoRoots.push_back(root);
    if (OK_DLS == objKind(sh, root))
        protoRoots.push_back((peer = dlSegPeer(sh, root)));

    BOOST_FOREACH(const TObjId proto, protoRoots) {
        if (proto == root || proto == peer)
            // we have inserted root/peer into protoRoots, in order to get them
            // on the list of allowed referrers, but it does not mean that they
            // are prototypes
            continue;

        if (!validatePointingObjects(sh, off, proto, OBJ_INVALID, OBJ_INVALID,
                                     protoRoots))
            return false;
    }

    // all OK!
    return true;
}

bool validateSegEntry(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                entry,
        const TObjId                prev,
        const TObjId                next,
        const TObjList              &protoRoots)
{
    if (isDlsBinding(off)) {
        // valPrev has to be at least VAL_NULL, withdraw it otherwise
        const TObjId prev = ptrObjByOffset(sh, entry, off.prev);
        CL_BREAK_IF(prev <= 0);
        if (sh.valueOf(prev) < 0)
            return false;
    }

    // first validate 'root' itself
    if (!validatePointingObjects(sh, off, entry, prev, next, protoRoots,
                                 /* toInsideOnly */ true))
        return false;

    return validatePrototypes(sh, off, entry, protoRoots);
}

TObjId nextObj(
        const SymHeap               &sh,
        const BindingOff            &off,
        TObjId                      obj)
{
    if (OK_DLS == objKind(sh, obj))
        // jump to peer in case of DLS
        obj = dlSegPeer(sh, obj);

    const TObjId nextPtr = ptrObjByOffset(sh, obj, off.next);
    const TObjId nextHead = sh.pointsTo(sh.valueOf(nextPtr));
    return objRoot(sh, nextHead);
}

TObjId jumpToNextObj(
        const SymHeap               &sh,
        const BindingOff            &off,
        std::set<TObjId>            &haveSeen,
        TObjId                      obj)
{
    if (!matchSegBinding(sh, obj, off))
        // binding mismatch
        return OBJ_INVALID;

    const bool dlSegOnPath = (OK_DLS == objKind(sh, obj));
    if (dlSegOnPath) {
        // jump to peer in case of DLS
        obj = dlSegPeer(sh, obj);
        haveSeen.insert(obj);
    }

    const struct cl_type *clt = sh.objType(obj);
    const TObjId nextPtr = ptrObjByOffset(sh, obj, off.next);
    CL_BREAK_IF(nextPtr <= 0);

    const TObjId nextHead = sh.pointsTo(sh.valueOf(nextPtr));
    if (nextHead <= 0)
        // no head pointed by nextPtr
        return OBJ_INVALID;

    // FIXME: should we use objRootByPtr() here?
    const TObjId next = compObjByOffset(sh, nextHead, (-off.head));
    if (next <= 0)
        // no suitable next object
        return OBJ_INVALID;

    if (OBJ_INVALID != sh.objParent(next))
        // next object is embedded into another object
        return OBJ_INVALID;

    const struct cl_type *cltNext = sh.objType(next);
    if (!cltNext || *cltNext != *clt)
        // type mismatch
        return OBJ_INVALID;

    if (sh.cVar(0, objRoot(sh, next)))
        // objects on stack should NOT be abstracted out
        return OBJ_INVALID;

    if (!matchSegBinding(sh, next, off))
        // binding mismatch
        return OBJ_INVALID;

    const bool isDls = isDlsBinding(off);
    if (isDls) {
        // check DLS back-link
        const TValId valPrev = sh.valueOf(ptrObjByOffset(sh, next, off.prev));
        const TValId headAt = sh.placedAt(compObjByOffset(sh, obj, off.head));
        if (valPrev != headAt)
            // DLS back-link mismatch
            return OBJ_INVALID;
    }

    if (dlSegOnPath
            && !validatePointingObjects(sh, off, obj, /* prev */ obj, next))
        // never step over a peer object that is pointed from outside!
        return OBJ_INVALID;

    // check if valNext inside the 'next' object is at least VAL_NULL
    // (otherwise we are not able to construct Neq edges to express its length)
    if (sh.valueOf(ptrObjByOffset(sh, next, off.next)) < 0)
        return OBJ_INVALID;

    return next;
}

typedef TObjList TProtoRoots[2];

bool matchData(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                o1,
        const TObjId                o2,
        TProtoRoots                 *protoRoots,
        int                         *pThreshold)
{
    EJoinStatus status;
    if (!joinDataReadOnly(&status, sh, off, o1, o2, protoRoots))
        return false;

    int thr = 0;
#if SE_PREFER_LOSSLESS_PROTOTYPES
    switch (status) {
        case JS_USE_ANY:
            break;

        case JS_USE_SH1:
        case JS_USE_SH2:
            thr = 2;
            break;

        case JS_THREE_WAY:
            thr = 3;
            break;
    }
#endif

    if (pThreshold)
        *pThreshold = thr;

    return true;
}

bool slSegAvoidSelfCycle(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                objFrom,
        const TObjId                objTo)
{
    if (isDlsBinding(off))
        // not a SLS
        return false;

    const TValId v1 = sh.placedAt(objFrom);
    const TValId v2 = sh.valueOf(ptrObjByOffset(sh, objTo, off.next));

    return haveSeg(sh, v1, v2, OK_SLS)
        || haveSeg(sh, v2, v1, OK_SLS);
}

void dlSegAvoidSelfCycle(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                entry,
        std::set<TObjId>            &haveSeen)
{
    if (!isDlsBinding(off))
        // not a DLS
        return;

    const TObjId prevPtr = ptrObjByOffset(sh, entry, off.prev);
    const TObjId prev = objRootByPtr(sh, prevPtr);
    if (prev <= 0)
        // no valid previous object
        return;

    const struct cl_type *const cltEntry = sh.objType(entry);
    const struct cl_type *const cltPrev = sh.objType(prev);
    CL_BREAK_IF(!cltEntry);
    if (!cltPrev || *cltPrev != *cltEntry)
        // type mismatch
        return;

    // note we have seen the previous object (and its peer in case of DLS)
    haveSeen.insert(prev);
    if (OK_DLS == objKind(sh, prev))
        haveSeen.insert(dlSegPeer(sh, prev));
}

unsigned /* len */ segDiscover(
        const SymHeap               &sh,
        const BindingOff            &off,
        const TObjId                entry)
{
    // we use std::set to detect loops
    std::set<TObjId> haveSeen;
    haveSeen.insert(entry);
    TObjId prev = entry;

    dlSegAvoidSelfCycle(sh, off, entry, haveSeen);
    TObjId obj = jumpToNextObj(sh, off, haveSeen, entry);
    if (!insertOnce(haveSeen, obj))
        // loop detected
        return 0;

    // [experimental] we need a way to prefer lossless prototypes
    int maxThreshold = 0;

    // main loop of segDiscover()
    std::vector<TObjId> path;
    while (OBJ_INVALID != obj) {
        // compare the data
        TProtoRoots protoRoots;
        int threshold = 0;

        // TODO: optimize such that matchData() is not called at all when any
        // _program_ variable points at/inside;  call of matchData() in such
        // cases is significant waste for us!
        if (!matchData(sh, off, prev, obj, &protoRoots, &threshold)) {
            CL_DEBUG("    DataMatchVisitor refuses to create a segment!");
            break;
        }

        if (prev == entry && !validateSegEntry(sh, off, entry, OBJ_INVALID, obj,
                                               protoRoots[0]))
            // invalid entry
            break;

        if (!insertOnce(haveSeen, nextObj(sh, off, obj)))
            // loop detected
            break;

        if (!validatePrototypes(sh, off, obj, protoRoots[1]))
            // someone points to a prototype
            break;

        // look ahead
        TObjId next = jumpToNextObj(sh, off, haveSeen, obj);
        if (!validatePointingObjects(sh, off, obj, prev, next, protoRoots[1])) {
            // someone points at/inside who should not

            const bool allowReferredEnd =
                /* looking of a DLS */ isDlsBinding(off)
                && OK_DLS != objKind(sh, obj)
                && validateSegEntry(sh, off, obj, prev, OBJ_INVALID,
                                    protoRoots[1]);

            if (allowReferredEnd) {
                // we allow others to point at DLS end-point's _head_
                path.push_back(obj);
                if (maxThreshold < threshold)
                    maxThreshold = threshold;
            }

            break;
        }

        // enlarge the path by one
        path.push_back(obj);
        if (maxThreshold < threshold)
            maxThreshold = threshold;

        // jump to the next object on the path
        prev = obj;
        obj = next;
    }

    if (path.empty())
        // found nothing
        return 0;

    int len = path.size();
    if (slSegAvoidSelfCycle(sh, off, entry, path.back()))
        // avoid creating self-cycle of two SLS segments
        --len;

    return std::max(0, len - maxThreshold);
}

bool digSegmentHead(
        TOffset                     *pOff,
        const SymHeap               &sh,
        const struct cl_type        *cltRoot,
        TObjId                      obj)
{
    const TObjId head = obj;

    while (*cltRoot != *sh.objType(obj)) {
        int nth;
        obj = sh.objParent(obj, &nth);
        if (OBJ_INVALID == obj)
            // head not found
            return false;
    }

    // FIXME: not tested
    *pOff = subOffsetIn(sh, obj, head);
    return true;
}

struct PtrFinder {
    TObjId              root;
    TValId              targetAddr;
    TOffset             offFound;

    bool operator()(const SymHeap &sh, TObjId sub) {
        const TValId val = sh.valueOf(sub);
        if (val <= 0)
            return /* continue */ true;

        if (val != targetAddr)
            return /* continue */ true;

        // target found!
        offFound = subOffsetIn(sh, root, sub);
        return /* break */ false;
    }
};

void digBackLink(
        BindingOff                  &off,
        const SymHeap               &sh,
        const TObjId                next,
        TObjId                      root)
{
    PtrFinder visitor;
    visitor.root = next;
    visitor.offFound = 0;
    visitor.targetAddr = sh.placedAt(compObjByOffset(sh, root, off.head));
    if (traverseSubObjs(sh, next, visitor, /* leavesOnly */ true)) {
        // not found
        off.prev = off.next;
        return;
    }

    // found
    off.prev = off.head + visitor.offFound;
}

typedef std::vector<BindingOff> TBindingCandidateList;

class ProbeEntryVisitor {
    private:
        TBindingCandidateList   &dst_;
        const TObjId            root_;
        const struct cl_type    *clt_;

    public:
        ProbeEntryVisitor(TBindingCandidateList         &dst,
                          const SymHeap                 &sh,
                          const TObjId                  root):
            dst_(dst),
            root_(root),
            clt_(sh.objType(root))
        {
            CL_BREAK_IF(!clt_);
        }

        bool operator()(const SymHeap &sh, TObjId sub) const
        {
            const TValId val = sh.valueOf(sub);
            if (val <= 0)
                return /* continue */ true;

            const TObjId next = sh.pointsTo(val);
            if (next <= 0 || root_ == objRoot(sh, next))
                return /* continue */ true;

            if (!isComposite(sh.objType(next)))
                // we take only composite types in case of segment head for now
                return /* continue */ true;

            BindingOff off;
            if (!digSegmentHead(&off.head, sh, clt_, next))
                return /* continue */ true;

            // entry candidate found, check the back-link in case of DLL
            off.next = subOffsetIn(sh, root_, sub);
#if SE_DISABLE_DLS
            off.prev = off.next;
#else
            digBackLink(off, sh, next, root_);
#endif

#if SE_DISABLE_SLS
            // allow only DLS abstraction
            if (!isDlsBinding(off));
                return /* continue */ true;
#endif
            // append a candidate
            dst_.push_back(off);
            return /* continue */ true;
        }
};

struct SegCandidate {
    TObjId                      entry;
    TBindingCandidateList       offList;
};

typedef std::vector<SegCandidate> TSegCandidateList;

unsigned /* len */ selectBestAbstraction(
        const SymHeap               &sh,
        const TSegCandidateList     &candidates,
        BindingOff                  *pOff,
        TObjId                      *entry)
{
    const unsigned cnt = candidates.size();
    if (!cnt)
        // no candidates given
        return 0;

    CL_DEBUG("--> initiating segment discovery, "
            << cnt << " entry candidate(s) given");

    // go through entry candidates
    unsigned            bestLen = 0;
    unsigned            bestIdx = 0;
    BindingOff          bestBinding;

    for (unsigned idx = 0; idx < cnt; ++idx) {

        // go through binding candidates
        const SegCandidate &segc = candidates[idx];
        BOOST_FOREACH(const BindingOff &off, segc.offList) {
            const unsigned len = segDiscover(sh, off, segc.entry);
            if (len <= bestLen)
                continue;

            // update best candidate
            bestIdx = idx;
            bestLen = len;
            bestBinding = off;
        }
    }

    if (!bestLen) {
        CL_DEBUG("<-- no new segment found");
        return 0;
    }

    // pick up the best candidate
    *pOff = bestBinding;
    *entry = candidates[bestIdx].entry;
    return bestLen;
}

unsigned /* len */ discoverBestAbstraction(
        SymHeap             &sh,
        BindingOff          *off,
        TObjId              *entry)
{
    TSegCandidateList candidates;

    // go through all potential segment entries
    TValList addrs;
    sh.gatherRootObjects(addrs, SymHeap::isOnHeap);
    BOOST_FOREACH(const TValId at, addrs) {
        const TObjId obj = sh.objAt(at);
        if (!isComposite(sh.objType(obj)))
            // we do not support generic objects atm, this will change soonish!
            continue;

        // use ProbeEntryVisitor visitor to validate the potential segment entry
        SegCandidate segc;
        const ProbeEntryVisitor visitor(segc.offList, sh, obj);
        traverseLivePtrs(sh, sh.placedAt(obj), visitor);
        if (segc.offList.empty())
            // found nothing
            continue;

        // append a segment candidate
        segc.entry = obj;
        candidates.push_back(segc);
    }

    return selectBestAbstraction(sh, candidates, off, entry);
}
