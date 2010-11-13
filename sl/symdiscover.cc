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
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <algorithm>                // for std::copy()
#include <set>

#include <boost/foreach.hpp>

bool validateUpLink(const SymHeap       &sh,
                    const TObjPair      &roots,
                    const TValueId      valNext1,
                    const TValueId      valNext2)
{
    const TObjId root1 = roots.first;
    const TObjId root2 = roots.second;
    SE_BREAK_IF(root1 <= 0 || root2 <= 0);

    TObjId peer1 = OBJ_INVALID;
    if (OK_DLS == sh.objKind(root1))
        peer1 = dlSegPeer(sh, root1);

    TObjId peer2 = OBJ_INVALID;
    if (OK_DLS == sh.objKind(root2))
        peer2 = dlSegPeer(sh, root2);

    TObjId o1 = sh.pointsTo(valNext1);
    TObjId o2 = sh.pointsTo(valNext2);
    if (o1 <= 0 || o2 <= 0)
        // non starter
        return false;

    for (;;) {
        int nth1, nth2;
        o1 = sh.objParent(o1, &nth1);
        o2 = sh.objParent(o2, &nth2);

        if (OBJ_INVALID == o1 || OBJ_INVALID == o2)
            // root mismatch
            return false;

        if (nth1 != nth2)
            // selector mismatch
            return false;

        if ((root1 == o1 || peer1 == o1)
                && (root2 == o2 || peer2 == o2))
            // uplink validated!
            break;
    }

    CL_DEBUG("validateUpLink() has successfully validated an up-link!");
    return true;
}

class GenericPrototypeFinder: public ISubMatchVisitor {
    private:
        const SymHeap           &sh_;
        const TObjPair          &roots_;
        bool                    ok_;
        std::set<TObjPair>      protoRoots_;

    public:
        GenericPrototypeFinder(const SymHeap &sh, const TObjPair &roots):
            sh_(sh),
            roots_(roots),
            ok_(true)
        {
        }

        bool result(void) const { return ok_; }

        const std::set<TObjPair>& protoRoots(void) const { return protoRoots_; }

        virtual bool considerVisiting(TValPair vp) {
            const TValueId v1 = vp.first;
            const TValueId v2 = vp.second;

            const TObjId o1 = sh_.pointsTo(v1);
            const TObjId o2 = sh_.pointsTo(v2);
            if (o1 <= 0 || o2 <= 0)
                // no valid objects anyway, keep going...
                return true;

            if (!ok_)
                // FIXME: suboptimal interface of ISubMatchVisitor
                return false;

            const TObjId root1 = objRoot(sh_, o1);
            const TObjId root2 = objRoot(sh_, o2);
            const TObjId up1 = roots_.first;
            const TObjId up2 = roots_.second;

            TObjId peerUp1 = OBJ_INVALID;
            if (OK_DLS == sh_.objKind(up1))
                peerUp1 = dlSegPeer(sh_, up1);

            TObjId peerUp2 = OBJ_INVALID;
            if (OK_DLS == sh_.objKind(up2))
                peerUp2 = dlSegPeer(sh_, up2);

            const bool rootOk1 = (root1 == up1 || root1 == peerUp1);
            const bool rootOk2 = (root2 == up2 || root2 == peerUp2);
            if (rootOk1 != rootOk2) {
                // up-link candidate mismatch
                ok_ = false;
                return false;
            }

            if (rootOk1) {
                ok_ = validateUpLink(sh_, roots_, v1, v2);

                // never step over roots_
                return false;
            }

            bool eq;
            if (sh_.proveEq(&eq, v1, v2) && eq)
                // do not traverse over shared data
                return false;

            // FIXME: At this point, we _have_ to check if we are able to
            //        establish a prototype object for (v1, v2).  If we
            //        later realize that we don't know how to create the
            //        prototype, it will be simply too late to do anything!
            SE_BREAK_IF(root1 <= 0 || root2 <= 0);
            const TObjPair proto(root1, root2);
            protoRoots_.insert(proto);

            // keep searching
            return true;
        }
};

bool considerGenericPrototype(
        const SymHeap           &sh,
        const TObjPair          &roots,
        const TValueId          v1,
        const TValueId          v2,
        TProtoRoots             *protoRoots)
{
    CL_DEBUG("considerGenericPrototype() called...");
    SE_BREAK_IF(v1 <= 0 || v2 <= 0);

    // declare starting points
    TValPairList startingPoints(1, TValPair(v1, v2));

    // initialize visitor
    GenericPrototypeFinder visitor(sh, roots);

    // traverse pointed sub-heaps
    if (!matchSubHeaps(sh, startingPoints, &visitor) || !visitor.result())
        return false;

    CL_DEBUG("considerGenericPrototype() has succeeded!");
    if (protoRoots) {
        // dump prototype adresses
        TProtoRoots &dst = *protoRoots;
        BOOST_FOREACH(const TObjPair &proto, visitor.protoRoots()) {
            dst[0].push_back(proto.first);
            dst[1].push_back(proto.second);
        }
    }

    return true;
}

// TODO: extend this for lists of length 1 and 2, now we support only empty ones
bool segMatchSmallList(
        const SymHeap           &sh,
        const TObjId            segUp,
        const TObjId            conUp,
        const TValueId          segVal,
        const TValueId          conVal)
{
    TObjId seg = objRoot(sh, sh.pointsTo(segVal));
    if (OK_DLS == sh.objKind(seg))
        seg = dlSegPeer(sh, seg);

    const TObjPair roots(segUp, conUp);
    const TValueId valNext = sh.valueOf(nextPtrFromSeg(sh, seg));
    return validateUpLink(sh, roots, valNext, conVal);
}

// TODO: I don't think we need separate handling for segment and non-segment
//       prototypes, we should merge those routines together at some point!
struct DataMatchVisitor {
    std::set<TObjId>    ignoreList;
    TProtoRoots         *protoRoots;
    TObjPair            roots_;

    DataMatchVisitor(TObjId o1, TObjId o2):
        protoRoots(0),
        roots_(o1, o2)
    {
    }

    bool operator()(const SymHeap &sh, TObjPair item) const {
        const TObjId o1 = item.first;
        if (hasKey(ignoreList, o1))
            return /* continue */ true;

        // first compare value IDs
        const TValueId v1 = sh.valueOf(o1);
        const TValueId v2 = sh.valueOf(item.second);

        bool eq;
        if (sh.proveEq(&eq, v1, v2) && eq)
            return /* continue */ true;

        // special values have to match
        if (v1 <= 0 || v2 <= 0)
            return /* mismatch */ false;

        if (validateUpLink(sh, roots_, v1, v2))
            return /* continue */ true;

        // compare _unknown_ value codes
        const EUnknownValue code1 = sh.valGetUnknown(v1);
        const EUnknownValue code2 = sh.valGetUnknown(v2);

        const bool isAbstract1 = (UV_ABSTRACT == code1);
        const bool isAbstract2 = (UV_ABSTRACT == code2);
        if (isAbstract1 != isAbstract2 && segMatchSmallList(sh,
                    (isAbstract1) ? roots_.first : roots_.second,
                    (isAbstract2) ? roots_.first : roots_.second,
                    (isAbstract1) ? v1 : v2,
                    (isAbstract2) ? v1 : v2))
        {
            // in case we have a list segment vs. a small compatible list, we
            // can cover both by a list segment with the appropriate length
            if (protoRoots) {
                (*protoRoots)[0].push_back(objRoot(sh, sh.pointsTo(v1)));
                (*protoRoots)[1].push_back(objRoot(sh, sh.pointsTo(v2)));
            }

            return /* continue */ true;
        }

        if (code1 != code2)
            return /* mismatch */ false;

        switch (code1) {
            case UV_UNINITIALIZED:
            case UV_UNKNOWN:
                // we don't require unkown values to match
                // FIXME: what about Neq predicates with foreign values???
                return /* continue */ true;

            case UV_KNOWN:
            case UV_ABSTRACT:
                break;
        }

        return considerGenericPrototype(sh, roots_, v1, v2, protoRoots);
    }
};

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

    switch (kind) {
        case OK_SLS:
            return (bf.next == bfDiscover.next);

        case OK_DLS:
            return (bf.next == bfDiscover.peer)
                && (bf.peer == bfDiscover.next);

        default:
            SE_TRAP;
            return false;
    }
}

bool preserveHeadPtr(const SymHeap                &sh,
                     const SegBindingFields       &bf,
                     const TObjId                 obj)
{
    const TValueId valPrev = sh.valueOf(subObjByChain(sh, obj, bf.peer));
    const TValueId valNext = sh.valueOf(subObjByChain(sh, obj, bf.next));
    if (valPrev <= 0 && valNext <= 0)
        // no valid address anyway
        return false;

    const TValueId addrHead = sh.placedAt(subObjByChain(sh, obj, bf.head));
    if (valPrev == addrHead || valNext == addrHead)
        // head pointer detected
        return true;

    const TValueId addrRoot = sh.placedAt(obj);
    if (valPrev == addrRoot || valNext == addrHead)
        // root pointer detected
        return true;

    // found nothing harmful
    return false;
}

// TODO: rewrite, simplify, and make it easier to follow
bool validatePointingObjectsCore(
        const SymHeap               &sh,
        const SegBindingFields      &bf,
        const TObjId                root,
        TObjId                      prev,
        const TObjId                next,
        const SymHeap::TContObj     &protoRoots,
        const bool                  toInsideOnly)
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
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, root, toInsideOnly);

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
bool validatePointingObjects(
        const SymHeap               &sh,
        const SegBindingFields      &bf,
        const TObjId                root,
        TObjId                      prev,
        const TObjId                next,
        SymHeap::TContObj           protoRoots,
        const bool                  toInsideOnly = false)
{
    // first validate 'root' itself
    if (!validatePointingObjectsCore(sh, bf, root, prev, next,
                                     protoRoots, toInsideOnly))
        return false;

    // then validate all prototypes
    TObjId peer = OBJ_INVALID;
    protoRoots.push_back(root);
    if (OK_DLS == sh.objKind(root))
        protoRoots.push_back((peer = dlSegPeer(sh, root)));

    BOOST_FOREACH(const TObjId proto, protoRoots) {
        if (proto == root || proto == peer)
            continue;

        if (!validatePointingObjectsCore(sh, bf, proto, OBJ_INVALID,
                                         OBJ_INVALID, protoRoots,
                                         /* toInsideOnly */ false))
            return false;
    }

    // all OK!
    return true;
}

bool validateSegEntry(const SymHeap              &sh,
                      const SegBindingFields     &bf,
                      const TObjId               entry,
                      const TObjId               next,
                      const SymHeap::TContObj    &protoRoots)
{
    return validatePointingObjects(sh, bf, entry, OBJ_INVALID, next,
                                   protoRoots, /* toInsideOnly */ true);
}

TObjId jumpToNextObj(const SymHeap              &sh,
                     const SegBindingFields     &bf,
                     std::set<TObjId>           &haveSeen,
                     TObjId                     obj)
{
    const bool dlSegOnPath = (OK_DLS == sh.objKind(obj));
    if (dlSegOnPath) {
        // jump to peer in case of DLS
        obj = dlSegPeer(sh, obj);
        haveSeen.insert(obj);
    }

    const struct cl_type *clt = sh.objType(obj);
    const TObjId nextPtr = subObjByChain(sh, obj, bf.next);
    SE_BREAK_IF(nextPtr <= 0);

    const TObjId nextHead = sh.pointsTo(sh.valueOf(nextPtr));
    if (nextHead <= 0)
        // no head pointed by nextPtr
        return OBJ_INVALID;

    const TObjId next = subObjByInvChain(sh, nextHead, bf.head);
    if (next <= 0)
        // no suitable next object
        return OBJ_INVALID;

    const struct cl_type *cltNext = sh.objType(next);
    if (!cltNext || *cltNext != *clt)
        // type mismatch
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
            && !validatePointingObjects(sh, bf, obj, /* prev */ obj, next,
                                        /* TODO */ SymHeap::TContObj()))
        // never step over a peer object that is pointed from outside!
        return OBJ_INVALID;

    return next;
}

bool matchData(const SymHeap                &sh,
               const SegBindingFields       &bf,
               const TObjId                 o1,
               const TObjId                 o2,
               TProtoRoots                  *protoRoots)
{
    const TObjId nextPtr = subObjByChain(sh, o1, bf.next);

    DataMatchVisitor visitor(o1, o2);
    visitor.ignoreList.insert(nextPtr);
    visitor.protoRoots = protoRoots;

    if (!bf.peer.empty()) {
        const TObjId prevPtr = subObjByChain(sh, o1, bf.peer);
        visitor.ignoreList.insert(prevPtr);
    }

    const TObjPair item(o1, o2);
    return traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
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
    TObjId prev = sh.pointsTo(sh.valueOf(prevPtr));
    prev = subObjByInvChain(sh, prev, bf.head);
    if (prev <= 0)
        // no valid previous object
        return;

    const struct cl_type *const cltEntry = sh.objType(entry);
    const struct cl_type *const cltPrev = sh.objType(prev);
    SE_BREAK_IF(!cltEntry);
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
    if (preserveHeadPtr(sh, bf, entry))
        // special quirk for head pointers
        return 0;

    // we use std::set to detect loops
    std::set<TObjId> haveSeen;
    haveSeen.insert(entry);
    TObjId prev = entry;

    dlSegAvoidSelfCycle(sh, bf, entry, haveSeen);
    TObjId obj = jumpToNextObj(sh, bf, haveSeen, entry);
    if (!insertOnce(haveSeen, obj))
        // loop detected
        return 0;

    // main loop of segDiscover()
    std::vector<TObjId> path;
    while (OBJ_INVALID != obj) {
        // compare the data
        TProtoRoots protoRoots;
        if (!matchData(sh, bf, prev, obj, &protoRoots)) {
            CL_DEBUG("    DataMatchVisitor refuses to create a segment!");
            break;
        }

        if (prev == entry
                && !validateSegEntry(sh, bf, entry, obj, protoRoots[0]))
            // invalid entry
            break;

        // look ahead
        TObjId next = jumpToNextObj(sh, bf, haveSeen, obj);
        if (OBJ_INVALID != next && !insertOnce(haveSeen, next))
            // loop detected
            break;

        if (!validatePointingObjects(sh, bf, obj, prev, next, protoRoots[1]))
            // someone points to inside who should not
            break;

        // enlarge the path by one
        path.push_back(obj);
        prev = obj;
        obj = next;
    }

    if (path.empty())
        // found nothing
        return 0;

    if (slSegAvoidSelfCycle(sh, bf, entry, path.back()))
        // avoid creating self-cycle of two SLS segments
        return path.size() - 1;

    return path.size();
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
            SE_BREAK_IF(!clt_);
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
