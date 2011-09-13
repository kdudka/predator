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
        const TValId                seg,
        const BindingOff            &offDiscover)
{
    const EObjKind kind = sh.valTargetKind(seg);
    if (OK_CONCRETE == kind)
        // nothing to match actually
        return true;

    const BindingOff off = sh.segBinding(seg);
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
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                root,
        TValId                      prev,
        const TValId                next,
        const TValList              &protoRoots = TValList(),
        const bool                  toInsideOnly = false)
{
    const bool isDls = isDlsBinding(off);
    std::set<TValId> allowedReferers;
    if (OK_DLS == sh.valTargetKind(root))
        // retrieve peer's pointer to this object (if any)
        allowedReferers.insert(dlSegPeer(sh, root));

    if (OK_DLS == sh.valTargetKind(prev))
        // jump to peer in case of DLS
        prev = dlSegPeer(sh, prev);

    // we allow pointers to self at this point, but we require them to be
    // absolutely uniform along the abstraction path -- matchData() should
    // later take care of that
    allowedReferers.insert(root);

    // collect all objects pointing at/inside the object
    // NOTE: we really intend to pass toInsideOnly == false at this point!
    TObjList refs;
    sh.pointedBy(refs, root);

    // consider also up-links from nested prototypes
    std::copy(protoRoots.begin(), protoRoots.end(),
              std::inserter(allowedReferers, allowedReferers.begin()));

    // please do not validate the binding pointers as data pointers;  otherwise
    // we might mistakenly abstract SLL with head-pointers of length 2 as DLS!!
    std::set<TValId> blackList;
    if (VAL_INVALID != prev || VAL_INVALID != next) {
        blackList.insert(sh.valByOffset(root, off.next));
        if (isDls)
            blackList.insert(sh.valByOffset(root, off.prev));
    }

    const TValId headAddr = sh.valByOffset(root, off.head);
    const bool rootIsProto = sh.valTargetIsProto(root);

    std::set<TValId> whiteList;
    whiteList.insert(sh.valByOffset(prev, off.next));
    if (isDls)
        whiteList.insert(sh.valByOffset(next, off.prev));

    BOOST_FOREACH(const TObjId obj, refs) {
        const TValId at = sh.placedAt(obj);
        if (hasKey(blackList, at))
            return false;

        if (hasKey(whiteList, at))
            continue;

        if (toInsideOnly && (sh.valueOf(obj) == headAddr))
            continue;

        if (hasKey(allowedReferers, sh.valRoot(at)))
            continue;

        if (!rootIsProto && sh.valTargetIsProto(at))
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
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                rootAt,
        TValList                    protoRoots)
{
    TValId peerAt = VAL_INVALID;
    protoRoots.push_back(rootAt);
    if (OK_DLS == sh.valTargetKind(rootAt)) {
        peerAt = dlSegPeer(sh, rootAt);
        protoRoots.push_back(peerAt);
    }

    BOOST_FOREACH(const TValId protoAt, protoRoots) {
        if (protoAt == rootAt || protoAt == peerAt)
            // we have inserted root/peer into protoRoots, in order to get them
            // on the list of allowed referrers, but it does not mean that they
            // are prototypes
            continue;

        if (!validatePointingObjects(sh, off, protoAt, VAL_INVALID, VAL_INVALID,
                                     protoRoots))
            return false;
    }

    // all OK!
    return true;
}

bool validateSegEntry(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry,
        const TValId                prev,
        const TValId                next,
        const TValList              &protoRoots)
{
    // first validate 'root' itself
    if (!validatePointingObjects(sh, off, entry, prev, next, protoRoots,
                                 /* toInsideOnly */ true))
        return false;

    return validatePrototypes(sh, off, entry, protoRoots);
}

TValId jumpToNextObj(
        SymHeap                     &sh,
        const BindingOff            &off,
        std::set<TValId>            &haveSeen,
        TValId                      at)
{
    if (!matchSegBinding(sh, at, off))
        // binding mismatch
        return VAL_INVALID;

    const bool dlSegOnPath = (OK_DLS == sh.valTargetKind(at));
    if (dlSegOnPath) {
        // jump to peer in case of DLS
        at = dlSegPeer(sh, at);
        haveSeen.insert(at);
    }

    const TValId nextHead = valOfPtrAt(sh, at, off.next);
    if (nextHead <= 0 || off.head != sh.valOffset(nextHead))
        // no valid head pointed by nextPtr
        return VAL_INVALID;

    const TValId next = sh.valRoot(nextHead);
    if (!isOnHeap(sh.valTarget(next)))
        // only objects on heap can be abstracted out
        return VAL_INVALID;

    if (!matchSegBinding(sh, next, off))
        // binding mismatch
        return VAL_INVALID;

    if (sh.valSizeOfTarget(at) != sh.valSizeOfTarget(next))
        // mismatch in size of targets
        return VAL_INVALID;

    const TObjType clt = sh.valLastKnownTypeOfTarget(at);
    if (clt) {
        const TObjType cltNext = sh.valLastKnownTypeOfTarget(next);
        if (cltNext && *cltNext != *clt)
            // both roots are (kind of) type-aware, but the types differ
            return VAL_INVALID;
    }

    const bool isDls = isDlsBinding(off);
    if (isDls) {
        // check DLS back-link
        const TValId headAt = sh.valByOffset(at, off.head);
        const TValId valPrev = valOfPtrAt(sh, next, off.prev);
        if (headAt != valPrev)
            // DLS back-link mismatch
            return VAL_INVALID;
    }

    if (dlSegOnPath
            && !validatePointingObjects(sh, off, at, /* prev */ at, next))
        // never step over a peer object that is pointed from outside!
        return VAL_INVALID;

    // check if valNext inside the 'next' object is at least VAL_NULL
    // (otherwise we are not able to construct Neq edges to express its length)
    if (valOfPtrAt(sh, next, off.next) < 0)
        return VAL_INVALID;

    return next;
}

typedef TValList TProtoRoots[2];

bool matchData(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                at1,
        const TValId                at2,
        TProtoRoots                 *protoRoots,
        int                         *pCost)
{
    EJoinStatus status;
    if (!joinDataReadOnly(&status, sh, off, at1, at2, protoRoots))
        return false;

    int cost = 0;
    switch (status) {
        case JS_USE_ANY:
            break;

        case JS_USE_SH1:
        case JS_USE_SH2:
            cost = (SE_PROTO_COST_ASYM);
            break;

        case JS_THREE_WAY:
            cost = (SE_PROTO_COST_THREEWAY);
            break;
    }

    *pCost = cost;
    return true;
}

bool dlSegHaveChain(
        SymHeap                     &sh,
        const TValId                beg,
        const TValId                end,
        const TOffset               offNext)
{
    TValSet seen;

    for (TValId cursor = beg;
            0 < cursor && insertOnce(seen, cursor);
            cursor = nextRootObj(sh, cursor, offNext))
    {
        const TValId next = nextRootObj(sh, cursor, offNext);
        if (next == end)
            return true;

        if (OK_DLS != sh.valTargetKind(next))
            return false;
    }

    return false;
}

bool segAvoidSelfCycle(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                beg,
        const TValId                end)
{
    if (!isDlsBinding(off)) {
        const TValId next = nextRootObj(sh, end, off.next);
        return haveSeg(sh, next, beg, OK_SLS);
    }

    TValId seg = end;
    if (OK_DLS == sh.valTargetKind(end))
        seg = dlSegPeer(sh, end);

    return dlSegHaveChain(sh, beg, seg, off.prev)
        || dlSegHaveChain(sh, seg, beg, off.next);
}

bool segDiscover(
        int                         *pLen,
        int                         *pCost,
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry)
{
    // we use std::set to detect loops
    std::set<TValId> haveSeen;
    haveSeen.insert(entry);
    TValId prev = entry;

    TValId at = jumpToNextObj(sh, off, haveSeen, entry);
    if (!insertOnce(haveSeen, at))
        // loop detected
        return false;

    // we need a way to prefer lossless prototypes
    *pCost = 0;

    // main loop of segDiscover()
    std::vector<TValId> path;
    while (VAL_INVALID != at) {
        // compare the data
        TProtoRoots protoRoots;
        int cost = 0;

        // TODO: optimize such that matchData() is not called at all when any
        // _program_ variable points at/inside;  call of matchData() in such
        // cases is significant waste for us!
        if (!matchData(sh, off, prev, at, &protoRoots, &cost)) {
            CL_DEBUG("    DataMatchVisitor refuses to create a segment!");
            break;
        }

        if (prev == entry && !validateSegEntry(sh, off, entry, VAL_INVALID, at,
                                               protoRoots[0]))
            // invalid entry
            break;

        if (!insertOnce(haveSeen, segNextRootObj(sh, at, off.next)))
            // loop detected
            break;

        if (!validatePrototypes(sh, off, at, protoRoots[1]))
            // someone points to a prototype
            break;

        // look ahead
        TValId next = jumpToNextObj(sh, off, haveSeen, at);
        if (!validatePointingObjects(sh, off, at, prev, next, protoRoots[1])) {
            // someone points at/inside who should not

            const bool allowReferredEnd =
                /* looking of a DLS */ isDlsBinding(off)
                && OK_DLS != sh.valTargetKind(at)
                && validateSegEntry(sh, off, at, prev, VAL_INVALID,
                                    protoRoots[1]);

            if (allowReferredEnd) {
                // we allow others to point at DLS end-point's _head_
                path.push_back(at);
                if (*pCost < cost)
                    *pCost = cost;
            }

            break;
        }

        // enlarge the path by one
        path.push_back(at);
        if (*pCost < cost)
            *pCost = cost;

        // jump to the next object on the path
        prev = at;
        at = next;
    }

    if (path.empty())
        // found nothing
        return false;

    int len = path.size();
    if (segAvoidSelfCycle(sh, off, entry, path.back()))
        // avoid creating self-cycle of two segments
        --len;

    *pLen = len;
    return (0 < len);
}

class PtrFinder {
    private:
        const TValId                lookFor_;
        TOffset                     offFound_;

    public:
        PtrFinder(TValId lookFor):
            lookFor_(lookFor)
        {
        }

        TOffset offFound() const {
            return offFound_;
        }

    bool operator()(SymHeap &sh, TObjId sub) {
        const TValId val = sh.valueOf(sub);
        if (val <= 0)
            return /* continue */ true;

        if (val != lookFor_)
            return /* continue */ true;

        // target found!
        offFound_ = sh.valOffset(sh.placedAt(sub));
        return /* break */ false;
    }
};

bool digBackLink(
        BindingOff                 *pOff,
        SymHeap                     &sh,
        const TValId                root,
        const TValId                next)
{
    // set up a visitor
    const TValId lookFor = sh.valByOffset(root, pOff->head);
    PtrFinder visitor(lookFor);

    // guide it through the next root entity
    const TValId lookAt = sh.valRoot(next);
    if (/* found nothing */ traverseLivePtrs(sh, lookAt, visitor))
        return false;

    // got a back-link!
    pOff->prev = visitor.offFound();
    return true;
}

typedef std::vector<BindingOff> TBindingCandidateList;

class ProbeEntryVisitor {
    private:
        TBindingCandidateList   &dst_;
        const TValId            root_;

    public:
        ProbeEntryVisitor(
                TBindingCandidateList         &dst,
                const TValId                  root):
            dst_(dst),
            root_(root)
        {
        }

        bool operator()(SymHeap &sh, TObjId sub) const
        {
            const TValId next = sh.valueOf(sub);
            if (!canWriteDataPtrAt(sh, next))
                return /* continue */ true;

            // read head offset
            BindingOff off;
            off.head = sh.valOffset(next);

            // entry candidate found, check the back-link in case of DLL
            off.next = sh.valOffset(sh.placedAt(sub));
            off.prev = off.next;
#if !SE_DISABLE_DLS
            digBackLink(&off, sh, root_, next);
#endif

#if SE_DISABLE_SLS
            // allow only DLS abstraction
            if (!isDlsBinding(off))
                return /* continue */ true;
#endif
            // append a candidate
            dst_.push_back(off);
            return /* continue */ true;
        }
};

bool slSegOnPath(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry,
        const unsigned              len)
{
    TValId cursor = entry;

    for (unsigned pos = 0; pos <= len; ++pos) {
        if (VT_ABSTRACT == sh.valTarget(cursor))
            return true;

        cursor = nextRootObj(sh, cursor, off.next);
    }

    return false;
}

struct SegCandidate {
    TValId                      entry;
    TBindingCandidateList       offList;
};

typedef std::vector<SegCandidate> TSegCandidateList;

unsigned /* len */ selectBestAbstraction(
        SymHeap                     &sh,
        const TSegCandidateList     &candidates,
        BindingOff                  *pOff,
        TValId                      *entry)
{
    const unsigned cnt = candidates.size();
    if (!cnt)
        // no candidates given
        return 0;

    CL_DEBUG("--> initiating segment discovery, "
            << cnt << " entry candidate(s) given");

    // go through entry candidates
    int                 bestLen     = 0;
    int                 bestCost    = INT_MAX;
    unsigned            bestIdx     = 0;
    BindingOff          bestBinding;

    for (unsigned idx = 0; idx < cnt; ++idx) {

        // go through binding candidates
        const SegCandidate &segc = candidates[idx];
        BOOST_FOREACH(const BindingOff &off, segc.offList) {
            int len, cost;
            if (!segDiscover(&len, &cost, sh, off, segc.entry))
                continue;

#if SE_DEFER_SLS_INTRO
            if (!cost 
                    && !isDlsBinding(off)
                    && !slSegOnPath(sh, off, segc.entry, len))
            {
                cost = (SE_DEFER_SLS_INTRO);
            }
#endif

            if (bestCost < cost)
                // we already got something cheaper
                continue;

            bestCost = cost;
            if (len <= bestLen)
                // we already got something longer
                continue;

            if (len <= (cost >> 2))
                // too expensive
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
        TValId              *entry)
{
    TSegCandidateList candidates;

    // go through all potential segment entries
    TValList addrs;
    sh.gatherRootObjects(addrs, isOnHeap);
    BOOST_FOREACH(const TValId at, addrs) {
        // use ProbeEntryVisitor visitor to validate the potential segment entry
        SegCandidate segc;
        const ProbeEntryVisitor visitor(segc.offList, at);
        traverseLivePtrs(sh, at, visitor);
        if (segc.offList.empty())
            // found nothing
            continue;

        // append a segment candidate
        segc.entry = at;
        candidates.push_back(segc);
    }

    return selectBestAbstraction(sh, candidates, off, entry);
}
