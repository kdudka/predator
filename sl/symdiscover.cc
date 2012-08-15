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

#include "prototype.hh"
#include "symcmp.hh"
#include "symjoin.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <algorithm>                // for std::copy()
#include <set>

#include <boost/foreach.hpp>

// costs are now hard-wired in the paper, so they were removed from config.h
#define SE_PROTO_COST_SYM           0
#define SE_PROTO_COST_ASYM          1
#define SE_PROTO_COST_THREEWAY      2

int minLengthByCost(int cost) {
    // abstraction length thresholds are now configurable in config.h
    static const int thrTable[] = {
        (SE_COST0_LEN_THR),
        (SE_COST1_LEN_THR),
        (SE_COST2_LEN_THR)
    };

    static const int maxCost = sizeof(thrTable)/sizeof(thrTable[0]) - 1;
    if (maxCost < cost)
        cost = maxCost;

    // Predator counts elementar merges whereas the paper counts objects on path
    const int minLength = thrTable[cost] - 1;
    CL_BREAK_IF(minLength < 1);
    return minLength;
}

bool matchSegBinding(
        const SymHeap               &sh,
        const TValId                seg,
        const BindingOff            &offPath)
{
    const EObjKind kind = sh.valTargetKind(seg);
    switch (kind) {
        case OK_CONCRETE:
            // nothing to match actually
            return true;

        case OK_OBJ_OR_NULL:
            // OK_OBJ_OR_NULL can be the last node of a NULL-terminated list
            return true;

        default:
            break;
    }

    const BindingOff offObj = sh.segBinding(seg);
    if (offObj.head != offPath.head)
        // head mismatch
        return false;

    if (!isDlsBinding(offPath)) {
        // OK_SLS
        switch (kind) {
            case OK_SEE_THROUGH:
            case OK_SLS:
                return (offObj.next == offPath.next);

            default:
                return false;
        }
    }

    // OK_DLS
    switch (kind) {
        case OK_SEE_THROUGH_2N:
            if ((offObj.next == offPath.next) && (offObj.prev == offPath.prev))
                // both fields are equal
                return true;

            // fall through!

        case OK_DLS:
            return (offObj.next == offPath.prev)
                && (offObj.prev == offPath.next);

        default:
            return false;
    }
}

/// (VAL_INVALID == prev && VAL_INVALID == next) denotes prototype validation
bool validatePointingObjects(
        SymHeap                    &sh,
        const BindingOff           &off,
        const TValId                root,
        TValId                      prev,
        const TValId                next,
        TValSet                     allowedReferers,
        const bool                  allowHeadPtr = false)
{
    // we allow pointers to self at this point, but we require them to be
    // absolutely uniform along the abstraction path -- joinDataReadOnly()
    // is responsible for that
    allowedReferers.insert(root);
    if (OK_DLS == sh.valTargetKind(root))
        allowedReferers.insert(dlSegPeer(sh, root));

    if (OK_DLS == sh.valTargetKind(prev))
        // jump to peer in case of DLS
        prev = dlSegPeer(sh, prev);

    // we allow pointers to self at this point, but we require them to be
    // absolutely uniform along the abstraction path -- matchData() should
    // later take care of that
    allowedReferers.insert(root);

    // collect all objects pointing at/inside the object
    ObjList refs;
    sh.pointedBy(refs, root);

    // unless this is a prototype, disallow self loops from _binding_ pointers
    TObjSet blackList;
    if (VAL_INVALID != prev || VAL_INVALID != next)
        buildIgnoreList(blackList, sh, root, off);

    const TValId headAddr = sh.valByOffset(root, off.head);

    TValSet whiteList;
    whiteList.insert(sh.valByOffset(prev, off.next));
    if (isDlsBinding(off))
        whiteList.insert(sh.valByOffset(next, off.prev));

    BOOST_FOREACH(const ObjHandle &obj, refs) {
        if (hasKey(blackList, obj))
            return false;

        const TValId at = obj.placedAt();
        if (hasKey(whiteList, at))
            continue;

        if (allowHeadPtr && (obj.value() == headAddr))
            continue;

        if (hasKey(allowedReferers, sh.valRoot(at)))
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
        TValSet                     protoRoots)
{
    TValId peerAt = VAL_INVALID;
    protoRoots.insert(rootAt);
    if (OK_DLS == sh.valTargetKind(rootAt)) {
        peerAt = dlSegPeer(sh, rootAt);
        protoRoots.insert(peerAt);
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
        const TValSet               &protoRoots)
{
    // first validate 'root' itself
    if (!validatePointingObjects(sh, off, entry, prev, next, protoRoots,
                                 /* allowHeadPtr */ true))
        return false;

    return validatePrototypes(sh, off, entry, protoRoots);
}

TValId jumpToNextObj(
        SymHeap                     &sh,
        const BindingOff            &off,
        std::set<TValId>            &haveSeen,
        const TValSet               &protoRoots,
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

    if (dlSegOnPath && !validatePointingObjects(sh, off,
                /* root */ at,
                /* prev */ at,
                /* next */ next,
                protoRoots))
        // never step over a peer object that is pointed from outside!
        return VAL_INVALID;

    // check if valNext inside the 'next' object is at least VAL_NULL
    // (otherwise we are not able to construct Neq edges to express its length)
    if (valOfPtrAt(sh, next, off.next) < 0)
        return VAL_INVALID;

    return next;
}

bool isPointedByVar(SymHeap &sh, const TValId root) {
    ObjList refs;
    sh.pointedBy(refs, root);
    BOOST_FOREACH(const ObjHandle obj, refs) {
        const TValId at = obj.placedAt();
        const EValueTarget code = sh.valTarget(at);
        if (isProgramVar(code))
            return true;
    }

    // no reference by a program variable
    return false;
}

typedef TValSet TProtoRoots[2];

bool matchData(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                at1,
        const TValId                at2,
        TProtoRoots                 *protoRoots,
        int                         *pCost)
{
    if (!isDlsBinding(off) && isPointedByVar(sh, at2))
        // only first node of an SLS can be pointed by a program var, giving up
        return false;

    EJoinStatus status;
    if (!joinDataReadOnly(&status, sh, off, at1, at2, protoRoots)) {
        CL_DEBUG("    joinDataReadOnly() refuses to create a segment!");
        return false;
    }

    int cost = 0;
    switch (status) {
        case JS_USE_ANY:
            cost = (SE_PROTO_COST_SYM);
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

typedef std::map<int /* cost */, int /* length */> TRankMap;

void segDiscover(
        TRankMap                    &dst,
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry)
{
    CL_BREAK_IF(!dst.empty());

    // we use std::set to detect loops
    std::set<TValId> haveSeen;
    haveSeen.insert(entry);
    TValId prev = entry;

    // the entry can already have some prototypes we should take into account
    TValSet initialProtos;
    if (OK_DLS == sh.valTargetKind(entry)) {
        TValList protoList;
        collectPrototypesOf(protoList, sh, entry, /* skipDlsPeers */ false);
        BOOST_FOREACH(const TValId proto, protoList)
            initialProtos.insert(proto);
    }

    // jump to the immediate successor
    TValId at = jumpToNextObj(sh, off, haveSeen, initialProtos, entry);
    if (!insertOnce(haveSeen, at))
        // loop detected
        return;

    // we need a way to prefer lossless prototypes
    int maxCostOnPath = 0;

    // main loop of segDiscover()
    std::vector<TValId> path;
    while (VAL_INVALID != at) {
        // compare the data
        TProtoRoots protoRoots;
        int cost = 0;

        // join data of the current pair of objects
        if (!matchData(sh, off, prev, at, &protoRoots, &cost))
            break;

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

        bool leaving = false;

        // look ahead
        TValId next = jumpToNextObj(sh, off, haveSeen, protoRoots[1], at);
        if (!validatePointingObjects(sh, off, at, prev, next, protoRoots[1])) {
            // someone points at/inside who should not

            leaving = /* looking for a DLS */ isDlsBinding(off)
                && /* got a DLS */ OK_DLS != sh.valTargetKind(at)
                && validateSegEntry(sh, off, at, prev, VAL_INVALID,
                                    protoRoots[1]);

            if (!leaving)
                break;
        }

        // enlarge the path by one
        path.push_back(at);
        if (maxCostOnPath < cost)
            maxCostOnPath = cost;

        // remember the longest path at this cost level
#if !SE_ALLOW_SUBPATH_RANKING
        dst.clear();
#endif
        dst[maxCostOnPath] = path.size();

        if (leaving)
            // we allow others to point at DLS end-point's _head_
            break;

        // jump to the next object on the path
        prev = at;
        at = next;
    }
}

class PtrFinder {
    private:
        const TValId                lookFor_;
        TOffset                     offFound_;

    public:
        // cppcheck-suppress uninitMemberVar
        PtrFinder(TValId lookFor):
            lookFor_(lookFor)
        {
        }

        TOffset offFound() const {
            return offFound_;
        }

    bool operator()(const ObjHandle &sub) {
        const TValId val = sub.value();
        if (val <= 0)
            return /* continue */ true;

        if (val != lookFor_)
            return /* continue */ true;

        // target found!
        SymHeapCore *sh = sub.sh();
        offFound_ = sh->valOffset(sub.placedAt());
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

        bool operator()(const ObjHandle &sub) const
        {
            SymHeap &sh = *static_cast<SymHeap *>(sub.sh());
            const TValId next = sub.value();
            if (!canWriteDataPtrAt(sh, next))
                return /* continue */ true;

            // read head offset
            BindingOff off;
            off.head = sh.valOffset(next);

            // entry candidate found, check the back-link in case of DLL
            off.next = sh.valOffset(sub.placedAt());
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

bool segOnPath(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry,
        const unsigned              len)
{
    TValId cursor = entry;

    for (unsigned pos = 0; pos <= len; ++pos) {
        if (VT_ABSTRACT == sh.valTarget(cursor))
            return true;

        cursor = segNextRootObj(sh, cursor, off.next);
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
            TRankMap rMap;
            segDiscover(rMap, sh, off, segc.entry);

            // go through all cost/length pairs
            BOOST_FOREACH(TRankMap::const_reference rank, rMap) {
                const int len = rank.second;
                if (len <= 0)
                    continue;

                int cost = rank.first;
#if SE_COST_OF_SEG_INTRODUCTION
                if (!segOnPath(sh, off, segc.entry, len))
                    cost += (SE_COST_OF_SEG_INTRODUCTION);
#endif

                if (len < minLengthByCost(cost))
                    // too short path at this cost level
                    continue;

                if (bestCost < cost)
                    // we already got something cheaper
                    continue;

                if (len <= bestLen)
                    // we already got something longer
                    continue;

                // update best candidate
                bestIdx = idx;
                bestLen = len;
                bestCost = cost;
                bestBinding = off;
            }
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
