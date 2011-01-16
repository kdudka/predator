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

bool matchSegBinding(const SymHeap              &sh,
                     const TObjId               obj,
                     const SegBindingFields     &bfDiscover)
{
    const EObjKind kind = sh.objKind(obj);
    if (OK_CONCRETE == kind)
        // nothing to match actually
        return true;

    const SegBindingFields bf = sh.objBinding(obj);
    if (bf.head != bfDiscover.head)
        // head mismatch
        return false;

    if (bfDiscover.peer.empty()) {
        // OK_SLS
        switch (kind) {
            case OK_MAY_EXIST:
            case OK_SLS:
                return (bf.next == bfDiscover.next);

            default:
                return false;
        }
    }

    // OK_DLS
    switch (kind) {
        case OK_MAY_EXIST:
            // TODO: check polarity
            return (bf.next == bfDiscover.next);

        case OK_DLS:
            return (bf.next == bfDiscover.peer)
                && (bf.peer == bfDiscover.next);

        default:
            return false;
    }
}

// TODO: rewrite, simplify, and make it easier to follow
bool validatePointingObjects(
        const SymHeap               &sh,
        const SegBindingFields      &bf,
        const TObjId                root,
        TObjId                      prev,
        const TObjId                next,
        const SymHeap::TContObj     &protoRoots = SymHeap::TContObj(),
        const bool                  toInsideOnly = false)
{
    const bool isDls = !bf.peer.empty();
    std::set<TObjId> allowedReferers;
    if (OK_DLS == sh.objKind(root))
        // retrieve peer's pointer to this object (if any)
        allowedReferers.insert(dlSegPeer(sh, root));

    if (OK_DLS == sh.objKind(prev))
        // jump to peer in case of DLS
        prev = dlSegPeer(sh, prev);

    // we allow pointers to self at this point, but we require them to be
    // absolutely uniform along the abstraction path -- matchData() should
    // later take care of that
    allowedReferers.insert(root);

    // collect all objects pointing at/inside the object
    // NOTE: we really intend to pass toInsideOnly == false at this point!
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, root, /* toInsideOnly */ false);

    // consider also up-links from nested prototypes
    std::copy(protoRoots.begin(), protoRoots.end(),
              std::inserter(allowedReferers, allowedReferers.begin()));

    // please do not validate the binding pointers as data pointers;  otherwise
    // we might mistakenly abstract SLL with head-pointers of length 2 as DLS!!
    std::set<TObjId> blackList;
    blackList.insert(subObjByChain(sh, root, bf.next));
    if (isDls)
        blackList.insert(subObjByChain(sh, root, bf.peer));

    const TValueId headAddr = sh.placedAt(subObjByChain(sh, root, bf.head));
    const bool rootIsProto = sh.objIsProto(root);

    // TODO: move subObjByChain() calls out of the loop
    BOOST_FOREACH(const TObjId obj, refs) {
        if (hasKey(blackList, obj))
            return false;

        if (obj == subObjByChain(sh, prev, bf.next))
            continue;

        if (isDls && obj == subObjByChain(sh, next, bf.peer))
            continue;

        if (toInsideOnly && sh.valueOf(obj) == headAddr)
            continue;

        if (hasKey(allowedReferers, objRoot(sh, obj)))
            continue;

        if (!rootIsProto && sh.objIsProto(obj))
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
        const SegBindingFields      &bf,
        const TObjId                root,
        SymHeap::TContObj           protoRoots)
{
    TObjId peer = OBJ_INVALID;
    protoRoots.push_back(root);
    if (OK_DLS == sh.objKind(root))
        protoRoots.push_back((peer = dlSegPeer(sh, root)));

    BOOST_FOREACH(const TObjId proto, protoRoots) {
        if (proto == root || proto == peer)
            // we have inserted root/peer into protoRoots, in order to get them
            // on the list of allowed referrers, but it does not mean that they
            // are prototypes
            continue;

        if (!validatePointingObjects(sh, bf, proto, OBJ_INVALID, OBJ_INVALID,
                                     protoRoots))
            return false;
    }

    // all OK!
    return true;
}

bool validateSegEntry(const SymHeap              &sh,
                      const SegBindingFields     &bf,
                      const TObjId               entry,
                      const TObjId               prev,
                      const TObjId               next,
                      const SymHeap::TContObj    &protoRoots)
{
    // first validate 'root' itself
    if (!validatePointingObjects(sh, bf, entry, prev, next, protoRoots,
                                 /* toInsideOnly */ true))
        return false;

    return validatePrototypes(sh, bf, entry, protoRoots);
}

TObjId nextObj(
        const SymHeap               &sh,
        const SegBindingFields      &bf,
        TObjId                      obj)
{
    if (OK_DLS == sh.objKind(obj))
        // jump to peer in case of DLS
        obj = dlSegPeer(sh, obj);

    const TObjId nextPtr = subObjByChain(sh, obj, bf.next);
    const TObjId nextHead = sh.pointsTo(sh.valueOf(nextPtr));
    return objRoot(sh, nextHead);
}

TObjId jumpToNextObj(const SymHeap              &sh,
                     const SegBindingFields     &bf,
                     std::set<TObjId>           &haveSeen,
                     TObjId                     obj)
{
    if (!matchSegBinding(sh, obj, bf))
        // binding mismatch
        return OBJ_INVALID;

    const bool dlSegOnPath = (OK_DLS == sh.objKind(obj));
    if (dlSegOnPath) {
        // jump to peer in case of DLS
        obj = dlSegPeer(sh, obj);
        haveSeen.insert(obj);
    }

    const struct cl_type *clt = sh.objType(obj);
    const TObjId nextPtr = subObjByChain(sh, obj, bf.next);
    CL_BREAK_IF(nextPtr <= 0);

    const TObjId nextHead = sh.pointsTo(sh.valueOf(nextPtr));
    if (nextHead <= 0)
        // no head pointed by nextPtr
        return OBJ_INVALID;

    const TObjId next = subObjByInvChain(sh, nextHead, bf.head);
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

    if (!matchSegBinding(sh, next, bf))
        // binding mismatch
        return OBJ_INVALID;

    const bool isDls = !bf.peer.empty();
    if (isDls) {
        // check DLS back-link
        const TObjId prevPtr = subObjByChain(sh, next, bf.peer);
        const TObjId head = subObjByChain(sh, obj, bf.head);
        if (sh.valueOf(prevPtr) != sh.placedAt(head))
            // DLS back-link mismatch
            return OBJ_INVALID;
    }

    if (dlSegOnPath
            && !validatePointingObjects(sh, bf, obj, /* prev */ obj, next))
        // never step over a peer object that is pointed from outside!
        return OBJ_INVALID;

    return next;
}

typedef SymHeap::TContObj TProtoRoots[2];

bool matchData(const SymHeap                &sh,
               const SegBindingFields       &bf,
               const TObjId                 o1,
               const TObjId                 o2,
               TProtoRoots                  *protoRoots,
               int                          *pThreshold)
{
    EJoinStatus status;
    if (!joinDataReadOnly(&status, sh, bf, o1, o2, protoRoots))
        return false;

    // FIXME: highly experimental
    if (objIsSeg(sh, o2) && objIsSeg(sh, nextObj(sh, bf, o2)))
        return true;

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
        const SegBindingFields      &bf,
        const TObjId                objFrom,
        const TObjId                objTo)
{
    if (!bf.peer.empty())
        // not a SLS
        return false;

    const TValueId v1 = sh.placedAt(objFrom);
    const TValueId v2 = sh.valueOf(subObjByChain(sh, objTo, bf.next));

    return haveSeg(sh, v1, v2, OK_SLS)
        || haveSeg(sh, v2, v1, OK_SLS);
}

void dlSegAvoidSelfCycle(
        const SymHeap               &sh,
        const SegBindingFields      &bf,
        const TObjId                entry,
        std::set<TObjId>            &haveSeen)
{
    if (bf.peer.empty())
        // not a DLS
        return;

    const TObjId prevPtr = subObjByChain(sh, entry, bf.peer);
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
    if (OK_DLS == sh.objKind(prev))
        haveSeen.insert(dlSegPeer(sh, prev));
}

unsigned /* len */ segDiscover(const SymHeap            &sh,
                               const SegBindingFields   &bf,
                               const TObjId             entry)
{
    // we use std::set to detect loops
    std::set<TObjId> haveSeen;
    haveSeen.insert(entry);
    TObjId prev = entry;

    dlSegAvoidSelfCycle(sh, bf, entry, haveSeen);
    TObjId obj = jumpToNextObj(sh, bf, haveSeen, entry);
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
        if (!matchData(sh, bf, prev, obj, &protoRoots, &threshold)) {
            CL_DEBUG("    DataMatchVisitor refuses to create a segment!");
            break;
        }

        if (prev == entry && !validateSegEntry(sh, bf, entry, OBJ_INVALID, obj,
                                               protoRoots[0]))
            // invalid entry
            break;

        if (!insertOnce(haveSeen, nextObj(sh, bf, obj)))
            // loop detected
            break;

        if (!validatePrototypes(sh, bf, obj, protoRoots[1]))
            // someone points to a prototype
            break;

        // look ahead
        TObjId next = jumpToNextObj(sh, bf, haveSeen, obj);
        if (!validatePointingObjects(sh, bf, obj, prev, next, protoRoots[1])) {
            // someone points at/inside who should not

            const bool allowReferredEnd =
                /* looking of a DLS */ !bf.peer.empty()
                && OK_DLS != sh.objKind(obj)
                && validateSegEntry(sh, bf, obj, prev, OBJ_INVALID,
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
    if (slSegAvoidSelfCycle(sh, bf, entry, path.back()))
        // avoid creating self-cycle of two SLS segments
        --len;

    return std::max(0, len - maxThreshold);
}

bool digSegmentHead(TFieldIdxChain          &dst,
                    const SymHeap           &sh,
                    const struct cl_type    *cltRoot,
                    TObjId                  obj)
{
    TFieldIdxChain invIc;
    while (*cltRoot != *sh.objType(obj)) {
        int nth;
        obj = sh.objParent(obj, &nth);
        if (OBJ_INVALID == obj)
            // head not found
            return false;

        invIc.push_back(nth);
    }

    // head found, now reverse the index chain (if any)
    dst.clear();
    std::copy(invIc.rbegin(), invIc.rend(), std::back_inserter(dst));
    return true;
}

struct PtrFinder {
    TObjId              target;
    TFieldIdxChain      icFound;

    bool operator()(const SymHeap &sh, TObjId sub, TFieldIdxChain ic) {
        const TValueId val = sh.valueOf(sub);
        if (val <= 0)
            return /* continue */ true;

        const TObjId obj = sh.pointsTo(val);
        if (obj != target)
            return /* continue */ true;

        // target found!
        icFound = ic;
        return /* break */ false;
    }
};

void digBackLink(SegBindingFields           &bf,
                 const SymHeap              &sh,
                 const TObjId               next,
                 TObjId                     root)
{
    PtrFinder visitor;
    visitor.target = /* head */ subObjByChain(sh, root, bf.head);
    if (traverseSubObjsIc(sh, next, visitor))
        // not found
        return;

    // join the idx chain with head
    const TFieldIdxChain &fromHeadToBack = visitor.icFound;
    bf.peer = bf.head;
    std::copy(fromHeadToBack.begin(),
              fromHeadToBack.end(),
              std::back_inserter(bf.peer));

    if (bf.peer == bf.next)
        // next and prev pointers have to be two distinct pointers, withdraw it
        bf.peer.clear();
}

typedef std::vector<SegBindingFields> TBindingCandidateList;

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

        bool operator()(const SymHeap &sh, TObjId sub, TFieldIdxChain ic) const
        {
            const TValueId val = sh.valueOf(sub);
            if (val <= 0)
                return /* continue */ true;

            const TObjId next = sh.pointsTo(val);
            if (next <= 0)
                return /* continue */ true;

            if (!isComposite(sh.objType(next)))
                // we take only composite types in case of segment head for now
                return /* continue */ true;

            SegBindingFields bf;
            if (!digSegmentHead(bf.head, sh, clt_, next))
                return /* continue */ true;

            // entry candidate found, check the back-link in case of DLL
            bf.next = ic;
#if !SE_DISABLE_DLS
            digBackLink(bf, sh, next, root_);
#endif

#if SE_DISABLE_SLS
            // allow only DLS abstraction
            if (bf.peer.empty())
                return /* continue */ true;
#endif

            // append a candidate
            dst_.push_back(bf);
            return /* continue */ true;
        }
};

struct SegCandidate {
    TObjId                      entry;
    TBindingCandidateList       bfs;
};

typedef std::vector<SegCandidate> TSegCandidateList;

unsigned /* len */ selectBestAbstraction(
        const SymHeap               &sh,
        const TSegCandidateList     &candidates,
        SegBindingFields            *bf,
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
    SegBindingFields    bestBinding;

    for (unsigned idx = 0; idx < cnt; ++idx) {

        // go through binding candidates
        const SegCandidate &segc = candidates[idx];
        BOOST_FOREACH(const SegBindingFields &bf, segc.bfs) {
            const unsigned len = segDiscover(sh, bf, segc.entry);
            if (len <= bestLen)
                continue;

            // update best candidate
            bestIdx = idx;
            bestLen = len;
            bestBinding = bf;
        }
    }

    if (!bestLen) {
        CL_DEBUG("<-- no new segment found");
        return 0;
    }

    // pick up the best candidate
    *bf = bestBinding;
    *entry = candidates[bestIdx].entry;
    return bestLen;
}

unsigned /* len */ discoverBestAbstraction(
        const SymHeap       &sh,
        SegBindingFields    *bf,
        TObjId              *entry)
{
    TSegCandidateList candidates;

    // go through all potential segment entries
    SymHeapCore::TContObj roots;
    sh.gatherRootObjs(roots);
    BOOST_FOREACH(const TObjId obj, roots) {
        if (sh.cVar(0, obj))
            // skip static/automatic objects
            continue;

        const TValueId addr = sh.placedAt(obj);
        if (VAL_INVALID == addr)
            // no valid object anyway
            continue;

        // use ProbeEntryVisitor visitor to validate the potential segment entry
        SegCandidate segc;
        const ProbeEntryVisitor visitor(segc.bfs, sh, obj);
        traverseSubObjsIc(sh, obj, visitor);
        if (segc.bfs.empty())
            // found nothing
            continue;

        // append a segment candidate
        segc.entry = obj;
        candidates.push_back(segc);
    }

    return selectBestAbstraction(sh, candidates, bf, entry);
}
