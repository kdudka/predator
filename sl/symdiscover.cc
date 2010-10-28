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

#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <algorithm>                // for std::copy()
#include <iomanip>
#include <set>

#include <boost/foreach.hpp>

namespace {

typedef SymHeap::TContValue TProtoAddrs[2];

struct DataMatchVisitor {
    std::set<TObjId>    ignoreList;
    TProtoAddrs         *protoAddrs;
    TObjPair            roots_;

    DataMatchVisitor(TObjId o1, TObjId o2):
        protoAddrs(0),
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

        // compare _unknown_ value codes
        const EUnknownValue code = sh.valGetUnknown(v1);
        if (code != sh.valGetUnknown(v2))
            return /* mismatch */ false;

        switch (code) {
            case UV_KNOWN:
                // known values have to match
                return false;

            case UV_UNINITIALIZED:
            case UV_UNKNOWN:
                // safe to keep UV_UNKNOWN values as they are
                return true;

            case UV_ABSTRACT:
                // FIXME: unguarded recursion!
                if (!segConsiderPrototype(sh, roots_, v1, v2))
                    break;

                if (protoAddrs) {
                    // FIXME: what about shared prototypes at this point?
                    (*protoAddrs)[0].push_back(v1);
                    (*protoAddrs)[1].push_back(v2);
                }

                return true;
        }
        return /* mismatch */ false;
    }
};

bool validateUpLink(const SymHeap       &sh,
                    const TObjPair      &roots,
                    const TValueId      valNext1,
                    const TValueId      valNext2)
{
    const TObjId root1 = roots.first;
    const TObjId root2 = roots.second;
    SE_BREAK_IF(root1 <= 0 || root2 <= 0);

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

        if (root1 == o1 && root2 == o2)
            // uplink validated!
            break;
    }

    CL_WARN("validateUpLink() matched an up-link, "
            "but concretization of up-links is not yet implemented!");
    return true;
}

bool segMatchNextValues(const SymHeap     &sh,
                        const TObjPair    &roots,
                        const TObjId      seg1,
                        const TObjId      seg2)
{
    const TValueId v1 = sh.valueOf(nextPtrFromSeg(sh, seg1));
    const TValueId v2 = sh.valueOf(nextPtrFromSeg(sh, seg2));
    if (VAL_NULL == v1 && VAL_NULL == v2)
        // both values are NULL
        return true;

    if (validateUpLink(sh, roots, v1, v2))
        // valid uplink found
        return true;

    // TODO
    return false;
}

bool segEqual(const SymHeap     &sh,
              const TObjPair    &roots,
              const EObjKind    kind,
              const TObjId      o1,
              const TObjId      o2)
{
    const bool isDls = (OK_DLS == kind);
    SE_BREAK_IF(!isDls && OK_SLS != kind);

    if (sh.objBinding(o1).next != sh.objBinding(o2).next)
        // 'next' selector mismatch
        return false;

    if (sh.objBinding(o1).peer != sh.objBinding(o2).peer)
        // 'peer' selector mismatch
        return false;

    if (!segMatchNextValues(sh, roots, o1, o2))
        // end-point value mismatch
        return false;

    if (isDls) {
        const TObjId peer1 = dlSegPeer(sh, o1);
        const TObjId peer2 = dlSegPeer(sh, o2);
        if (!segMatchNextValues(sh, roots, peer1, peer2))
            // end-point value mismatch
            return false;
    }

    // compare the data
    DataMatchVisitor visitor(o1, o2);
    buildIgnoreList(sh, o1, visitor.ignoreList);
    const TObjPair item(o1, o2);
    return traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
}

class PointingObjectsFinder {
    SymHeap::TContObj &dst_;

    public:
        PointingObjectsFinder(SymHeap::TContObj &dst): dst_(dst) { }

        bool operator()(const SymHeap &sh, TObjId obj) const {
            const TValueId addr = sh.placedAt(obj);
            SE_BREAK_IF(addr <= 0);

            sh.usedBy(dst_, addr);
            return /* continue */ true;
        }
};

void gatherPointingObjects(const SymHeap            &sh,
                           SymHeap::TContObj        &dst,
                           const TObjId             root,
                           bool                     toInsideOnly)
{
    const PointingObjectsFinder visitor(dst);

    if (!toInsideOnly)
        visitor(sh, root);

    traverseSubObjs(sh, root, visitor, /* leavesOnly */ false);
}

bool segMayBePrototype(const SymHeap        &sh,
                       const EObjKind       kind,
                       const TObjId         seg,
                       const TObjId         upSeg)
{
    SE_BREAK_IF(OK_DLS != kind && OK_SLS != kind);
    const TObjId peer = (OK_DLS == kind)
        ? dlSegPeer(sh, seg)
        : static_cast<TObjId>(OBJ_INVALID);

    // gather pointing objects
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, seg, /* toInsideOnly */ false);
    if (OBJ_INVALID != peer)
        gatherPointingObjects(sh, refs, peer, /* toInsideOnly */ false);

    // declare set of allowed pointing objects
    std::set<TObjId> allowedRoots;
    allowedRoots.insert(seg);
    allowedRoots.insert(upSeg);
    if (OBJ_INVALID != peer)
        allowedRoots.insert(peer);

    // wait, upSeg can already be a segment at this point!
    if (OK_DLS == sh.objKind(upSeg))
        allowedRoots.insert(dlSegPeer(sh, upSeg));

    // go through all pointing objects and validate them
    BOOST_FOREACH(const TObjId obj, refs) {
        const TObjId root = objRoot(sh, obj);
        if (!hasKey(allowedRoots, root))
            return false;
    }

    // no intruder found!
    return true;
}

} // namespace

bool segConsiderPrototype(const SymHeap     &sh,
                          const TObjPair    &roots,
                          const TValueId    v1,
                          const TValueId    v2)
{
    TObjId o1 = sh.pointsTo(v1);
    TObjId o2 = sh.pointsTo(v2);
    SE_BREAK_IF(o1 <= 0 || o2 <= 0);

    EObjKind kind = sh.objKind(o1);
    if (sh.objKind(o2) != kind)
        // object kind mismatch
        return false;

    if (OK_HEAD == kind) {
        // jump to root, which should be a segment
        o1 = objRoot(sh, o1);
        o2 = objRoot(sh, o2);
        SE_BREAK_IF(o1 <= 0 || o2 <= 0);
        SE_BREAK_IF(OK_SLS != sh.objKind(o1) && OK_DLS != sh.objKind(o1));
        SE_BREAK_IF(OK_SLS != sh.objKind(o2) && OK_DLS != sh.objKind(o2));

        kind = sh.objKind(o1);
        if (sh.objKind(o2) != kind)
            // object kind mismatch
            return false;
    }

    const struct cl_type *clt1 = sh.objType(o1);
    const struct cl_type *clt2 = sh.objType(o2);
    SE_BREAK_IF(!clt1 || !clt2);
    if (*clt1 != *clt2)
        // type mismatch
        return false;

    return segEqual(sh, roots, kind, o1, o2)
        && segMayBePrototype(sh, kind, o1, roots.first)
        && segMayBePrototype(sh, kind, o2, roots.second);
}

namespace {

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

bool validateSinglePointingObject(const SymHeap             &sh,
                                  const SegBindingFields    &bf,
                                  const TObjId              obj,
                                  const TObjId              prev,
                                  const TObjId              next)
{
    if (obj == subObjByChain(sh, prev, bf.next))
        return true;

    const bool isDls = !bf.peer.empty();
    if (isDls && obj == subObjByChain(sh, next, bf.peer))
        return true;

    // TODO
    return false;
}

bool validatePointingObjects(const SymHeap              &sh,
                             const SegBindingFields     &bf,
                             const TObjId               root,
                             TObjId                     prev,
                             const TObjId               next,
                             const SymHeap::TContValue  &protoAddrs,
                             const bool                 toInsideOnly = false)
{
    std::set<TObjId> allowedReferers;
    if (OK_DLS == sh.objKind(root))
        // retrieve peer's pointer to this object (if any)
        allowedReferers.insert(peerPtrFromSeg(sh, dlSegPeer(sh, root)));

    if (OK_DLS == sh.objKind(prev))
        // jump to peer in case of DLS
        prev = dlSegPeer(sh, prev);

    // collect all object pointing at/inside the object
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, root, toInsideOnly);

    // consider also up-links from nested prototypes
    BOOST_FOREACH(const TValueId protoAt, protoAddrs) {
        const TObjId seg = sh.pointsTo(protoAt);
        const TObjId nextPtr = nextPtrFromSeg(sh, seg);
        allowedReferers.insert(nextPtr);
    }

    BOOST_FOREACH(const TObjId obj, refs) {
        if (validateSinglePointingObject(sh, bf, obj, prev, next))
            continue;

        if (hasKey(allowedReferers, obj))
            continue;

        // someone points at/inside who should not
        return false;
    }

    // no problems encountered
    return true;
}

bool validateSegEntry(const SymHeap              &sh,
                      const SegBindingFields     &bf,
                      const TObjId               entry,
                      const TObjId               next,
                      const SymHeap::TContValue  &protoAddrs)
{
    const TFieldIdxChain &icHead = bf.head;
    if (icHead.empty()) {
        // no Linux lists involved
        return validatePointingObjects(sh, bf, entry, OBJ_INVALID, next,
                                       protoAddrs, /* toInsideOnly */ true);
    }

    // jump to the head sub-object
    const TObjId head = subObjByChain(sh, entry, icHead);
    SE_BREAK_IF(entry == head || head <= 0);

    // FIXME: this is too strict for the _hlist_ variant of Linux lists
    if (!validatePointingObjects(sh, bf, head, OBJ_INVALID, next,
                                 /* TODO */ SymHeap::TContValue(),
                                 /* toInsideOnly */ true))
        return false;

    // now check that nothing but head is pointed from outside
    // FIXME: suboptimal due to performance
    SymHeap::TContObj refsAll, refsHead;
    const PointingObjectsFinder visAll(refsAll), visHead(refsHead);

    // gather all pointers at entry/head
    visAll(sh, entry);
    visHead(sh, head);

    // gather all pointers inside entry/head
    traverseSubObjs(sh, head, visHead, /* leavesOnly */ false);
    traverseSubObjs(sh, entry, visAll, /* leavesOnly */ false);

    // finally compare the sets
    return (refsAll.size() == refsHead.size());
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
                                        /* TODO */ SymHeap::TContValue()))
        // never step over a peer object that is pointed from outside!
        return OBJ_INVALID;

    return next;
}

bool matchData(const SymHeap                &sh,
               const SegBindingFields       &bf,
               const TObjId                 o1,
               const TObjId                 o2,
               TProtoAddrs                  *protoAddrs)
{
    const TObjId nextPtr = subObjByChain(sh, o1, bf.next);

    DataMatchVisitor visitor(o1, o2);
    visitor.ignoreList.insert(nextPtr);
    visitor.protoAddrs = protoAddrs;

    if (!bf.peer.empty()) {
        const TObjId prevPtr = subObjByChain(sh, o1, bf.peer);
        visitor.ignoreList.insert(prevPtr);
    }

    const TObjPair item(o1, o2);
    return traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
}

bool slSegAvoidSelfCycle(const SymHeap &sh, const TObjId o1, const TObjId o2) {
    const TValueId v1 = sh.placedAt(o1);
    const TValueId v2 = sh.placedAt(o2);

    return haveSeg(sh, v1, v2, OK_SLS)
        || haveSeg(sh, v2, v1, OK_SLS);
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

    const bool isDls = !bf.peer.empty();
    if (isDls) {
        // avoid DLS self-loop
        const TObjId prevPtr = subObjByChain(sh, entry, bf.peer);
        TObjId prev = sh.pointsTo(sh.valueOf(prevPtr));
        prev = subObjByInvChain(sh, prev, bf.head);
        if (0 < prev) {
            haveSeen.insert(prev);
            if (OK_DLS == sh.objKind(prev))
                haveSeen.insert(dlSegPeer(sh, prev));
        }
    }

    TObjId obj = jumpToNextObj(sh, bf, haveSeen, entry);
    if (!insertOnce(haveSeen, obj))
        // loop detected
        return 0;

    // main loop of segDiscover()
    std::vector<TObjId> path;
    while (OBJ_INVALID != obj) {
        // compare the data
        TProtoAddrs protoAddrs;
        if (!matchData(sh, bf, prev, obj, &protoAddrs)) {
            CL_DEBUG("    DataMatchVisitor refuses to create a segment!");
            break;
        }

        if (prev == entry
                && !validateSegEntry(sh, bf, entry, obj, protoAddrs[0]))
            // invalid entry
            break;

        // look ahead
        TObjId next = jumpToNextObj(sh, bf, haveSeen, obj);
        if (!insertOnce(haveSeen, next))
            // loop detected
            break;

        if (!validatePointingObjects(sh, bf, obj, prev, next, protoAddrs[1]))
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

    if (slSegAvoidSelfCycle(sh, entry, path.back()))
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

} // namespace

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
