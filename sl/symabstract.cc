/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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
#include "symabstract.hh"

#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "symcmp.hh"
#include "symdebug.hh"
#include "symjoin.hh"
#include "symdiscover.hh"
#include "symgc.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <iomanip>
#include <set>
#include <sstream>

#include <boost/foreach.hpp>

LOCAL_DEBUG_PLOTTER(symabstract, DEBUG_SYMABSTRACT)

void debugSymAbstract(const bool enable) {
    if (enable == __ldp_enabled_symabstract)
        return;

    CL_DEBUG("symabstract: debugSymAbstract(" << enable << ") takes effect");
    __ldp_enabled_symabstract = enable;
}

#define REQUIRE_GC_ACTIVITY(sh, val, fnc) do {                                 \
    if (collectJunk(sh, val))                                                  \
        break;                                                                 \
    CL_ERROR(#fnc "() failed to collect garbage, " #val " still referenced");  \
    CL_BREAK_IF("REQUIRE_GC_ACTIVITY has not been successful");                \
} while (0)

/// abstraction trigger threshold for SLS
static const unsigned slsThreshold = 1;

/// abstraction trigger threshold for DLS
static const unsigned dlsThreshold = 1;

// visitor
struct UnknownValuesDuplicator {
    std::set<TObjId> ignoreList;

    bool operator()(SymHeap &sh, TObjId obj) const {
        if (hasKey(ignoreList, obj))
            return /* continue */ true;

        const TValId valOld = sh.valueOf(obj);
        if (valOld <= 0)
            return /* continue */ true;

        const EValueTarget code = sh.valTarget(valOld);
        if (isPossibleToDeref(code) || (VT_CUSTOM == code))
            return /* continue */ true;

        // duplicate unknown value
        const TValId valNew = sh.valClone(valOld);
        sh.objSetValue(obj, valNew);

        return /* continue */ true;
    }
};

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TValId at) {
    UnknownValuesDuplicator visitor;
    buildIgnoreList(visitor.ignoreList, sh, at);

    // traverse all sub-objects
    traverseLiveObjs(sh, at, visitor);
}

void detachClonedPrototype(
        SymHeap                 &sh,
        const TValId            proto,
        const TValId            clone,
        const TValId            rootDst,
        const TValId            rootSrc)
{
    const bool isRootDls = (OK_DLS == sh.valTargetKind(rootDst));
    CL_BREAK_IF(isRootDls && (OK_DLS != sh.valTargetKind(rootSrc)));

    TValId rootSrcPeer = VAL_INVALID;
    if (isRootDls) {
        rootSrcPeer = dlSegPeer(sh, rootSrc);
        CL_BREAK_IF(dlSegPeer(sh, rootDst) != rootSrcPeer);
    }

    redirectRefs(sh, rootDst, proto, clone);
    redirectRefs(sh, proto, rootDst, rootSrc);
    if (isRootDls)
        redirectRefs(sh, clone, rootSrcPeer, rootDst);

    if (OK_DLS == sh.valTargetKind(proto)) {
        const TValId protoPeer = dlSegPeer(sh, proto);
        const TValId clonePeer = dlSegPeer(sh, clone);
        redirectRefs(sh, rootDst, protoPeer, clonePeer);
        redirectRefs(sh, protoPeer, rootDst, rootSrc);
        if (isRootDls)
            redirectRefs(sh, clonePeer, rootSrcPeer, rootDst);
    }
}

TValId protoClone(SymHeap &sh, const TValId proto) {
    TValId clone;

    if (isAbstract(sh.valTarget(proto))) {
        // clone segment prototype
        clone = segClone(sh, proto);
        segSetProto(sh, clone, false);
    }
    else {
        // clone bare prototype
        clone = sh.valClone(proto);
        sh.valTargetSetProto(clone, false);
        duplicateUnknownValues(sh, clone);
    }

    // if there was "a pointer to self", it should remain "a pointer to self";
    // however "self" has been changed, so that a redirection is necessary
    redirectRefs(sh, clone, proto, clone);

    return clone;
}

struct ProtoFinder {
    std::set<TValId> protos;

    bool operator()(SymHeap &sh, TObjId sub) {
        const TValId val = sh.valueOf(sub);
        if (val <= 0)
            return /* continue */ true;

        if (sh.valOffset(val))
            // TODO: support for prototypes not pointed by roots?
            return /* continue */ true;

        if (sh.valTargetIsProto(val))
            protos.insert(val);

        return /* continue */ true;
    }
};

// FIXME: this completely ignores Neq predicates for instance...
void cloneGenericPrototype(
        SymHeap                 &sh,
        const TValId            proto,
        const TValId            rootDst,
        const TValId            rootSrc)
{
    std::vector<TValId>         protoList;
    std::vector<TValId>         cloneList;
    std::vector<int>            lengthList;
    std::set<TValId>            haveSeen;
    std::stack<TValId>          todo;
    todo.push(proto);
    haveSeen.insert(proto);

    CL_DEBUG("WARNING: cloneGenericPrototype() is just a hack for now!");

    while (!todo.empty()) {
        const TValId proto = todo.top();
        todo.pop();
        protoList.push_back(proto);

        ProtoFinder visitor;
        traverseLivePtrs(sh, proto, visitor);
        BOOST_FOREACH(const TValId protoAt, visitor.protos) {
            if (!insertOnce(haveSeen, protoAt))
                continue;

            if (OK_DLS == sh.valTargetKind(protoAt) &&
                    !insertOnce(haveSeen, dlSegPeer(sh, protoAt)))
                continue;

            todo.push(protoAt);
        }
    }

    // allocate some space for clone IDs and minimal lengths
    const unsigned cnt = protoList.size();
    CL_BREAK_IF(!cnt);
    cloneList.resize(cnt);
    lengthList.resize(cnt);

    // clone the prototypes while reseting the minimal size to zero
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = protoList[i];
        if (isAbstract(sh.valTarget(proto))) {
            lengthList[i] = sh.segMinLength(proto);
            sh.segSetMinLength(proto, /* LS 0+ */ 0);
        }
        else
            lengthList[i] = -1;

        cloneList[i] = protoClone(sh, proto);
    }

    // FIXME: works, but likely to kill the CPU
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = protoList[i];
        const TValId clone = cloneList[i];
        detachClonedPrototype(sh, proto, clone, rootDst, rootSrc);

        for (unsigned j = 0; j < cnt; ++j) {
            if (i == j)
                continue;

            const TValId otherProto = protoList[j];
            const TValId otherClone = cloneList[j];
            detachClonedPrototype(sh, proto, clone, otherClone, otherProto);
        }
    }

    // finally restore the minimal size of all segments
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = protoList[i];
        const TValId clone = cloneList[i];
        const int len = lengthList[i];
        if (len <= 0)
            // -1 means "not a segment"
            continue;

        sh.segSetMinLength(proto, len);
        sh.segSetMinLength(clone, len);
    }
}

// visitor
struct ProtoCloner {
    std::set<TObjId> ignoreList;
    TValId rootDst;
    TValId rootSrc;

    bool operator()(SymHeap &sh, TObjId obj) const {
        if (hasKey(ignoreList, obj))
            return /* continue */ true;

        const TValId val = sh.valueOf(obj);
        if (val <= 0)
            return /* continue */ true;

        if (!isPossibleToDeref(sh.valTarget(val)))
            return /* continue */ true;

        // check if we point to prototype, or shared data
        if (sh.valTargetIsProto(val))
            cloneGenericPrototype(sh, sh.valRoot(val), rootDst, rootSrc);

        return /* continue */ true;
    }
};

struct ValueSynchronizer {
    SymHeap            &sh;
    std::set<TObjId>    ignoreList;

    ValueSynchronizer(SymHeap &sh_): sh(sh_) { }

    bool operator()(TObjId item[2]) const {
        const TObjId src = item[0];
        const TObjId dst = item[1];
        if (hasKey(ignoreList, src))
            return /* continue */ true;

        // store value of 'src' into 'dst'
        TValId valSrc = sh.valueOf(src);
        TValId valDst = sh.valueOf(dst);
        sh.objSetValue(dst, valSrc);

        // if the last reference is gone, we have a problem
        if (collectJunk(sh, valDst))
            CL_DEBUG("    ValueSynchronizer drops a sub-heap (dlSegPeer)");

        return /* continue */ true;
    }
};

void dlSegSyncPeerData(SymHeap &sh, const TValId dls) {
    const TValId peer = dlSegPeer(sh, dls);
    ValueSynchronizer visitor(sh);
    buildIgnoreList(visitor.ignoreList, sh, dls);

    // if there was "a pointer to self", it should remain "a pointer to self";
    TObjList refs;
    sh.pointedBy(refs, dls);
    std::copy(refs.begin(), refs.end(),
              std::inserter(visitor.ignoreList, visitor.ignoreList.begin()));

    const TValId roots[] = { dls, peer };
    traverseLiveObjs<2>(sh, roots, visitor);
}

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(
        SymHeap                     &sh,
        const TValId                srcAt,
        const TValId                dstAt,
        const bool                  bidir = false)
{
    if (!joinData(sh, dstAt, srcAt, bidir))
        CL_BREAK_IF("joinData() failed, failure of segDiscover()?");

    if (bidir)
        // already done as side-effect of joinData()
        return;

    if (OK_DLS == sh.valTargetKind(dstAt))
        dlSegSyncPeerData(sh, dstAt);

    if (bidir && OK_DLS == sh.valTargetKind(srcAt))
        dlSegSyncPeerData(sh, srcAt);
}

void clonePrototypes(SymHeap &sh, TValId addr, TValId dup) {
    CL_BREAK_IF(sh.valOffset(addr) || sh.valOffset(dup));
    duplicateUnknownValues(sh, addr);

    ProtoCloner visitor;
    visitor.rootDst = addr;
    visitor.rootSrc = dup;
    buildIgnoreList(visitor.ignoreList, sh, addr);

    // traverse all live sub-objects
    traverseLivePtrs(sh, addr, visitor);

    // if there was "a pointer to self", it should remain "a pointer to self";
    // however "self" has been changed, so that a redirection is necessary
    redirectRefs(sh, dup, addr, dup);
}

void slSegAbstractionStep(
        SymHeap                     &sh,
        const TValId                at,
        const TValId                nextAt,
        const BindingOff            &off)
{
    // read minimal length of 'obj' and set it temporarily to zero
    unsigned len = objMinLength(sh, at);
    if (isAbstract(sh.valTarget(at)))
        sh.segSetMinLength(at, /* SLS 0+ */ 0);

    len += objMinLength(sh, nextAt);
    if (OK_SLS == sh.valTargetKind(nextAt))
        sh.segSetMinLength(nextAt, /* SLS 0+ */ 0);
    else
        // abstract the _next_ object
        sh.valTargetSetAbstract(nextAt, OK_SLS, off);

    // merge data
    CL_BREAK_IF(OK_SLS != sh.valTargetKind(nextAt));
    abstractNonMatchingValues(sh, at, nextAt);

    // replace all references to 'head'
    const TOffset offHead = sh.segBinding(nextAt).head;
    const TValId headAt = sh.valByOffset(at, offHead);
    sh.valReplace(headAt, segHeadAt(sh, nextAt));

    // destroy self, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, headAt, slSegAbstractionStep);

    if (len)
        // declare resulting segment's minimal length
        sh.segSetMinLength(nextAt, len);
}

void enlargeMayExist(SymHeap &sh, const TValId at) {
    const EObjKind kind = sh.valTargetKind(at);
    switch (kind) {
        case OK_MAY_EXIST:
            sh.valTargetSetConcrete(at);
            // fall through!

        case OK_CONCRETE:
            return;

        default:
            CL_BREAK_IF("invalid call of enlargeMayExist()");
    }
}

void dlSegCreate(SymHeap &sh, TValId a1, TValId a2, BindingOff off) {
    // compute resulting segment's length
    const unsigned len = objMinLength(sh, a1) + objMinLength(sh, a2);

    // OK_MAY_EXIST -> OK_CONCRETE if necessary
    enlargeMayExist(sh, a1);
    enlargeMayExist(sh, a2);

    swapValues(off.next, off.prev);
    sh.valTargetSetAbstract(a1, OK_DLS, off);

    swapValues(off.next, off.prev);
    sh.valTargetSetAbstract(a2, OK_DLS, off);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, a1, a2, /* bidir */ true);

    // just created DLS is said to be 2+ as long as no OK_MAY_EXIST are involved
    sh.segSetMinLength(a1, len);
}

void dlSegGobble(SymHeap &sh, TValId dls, TValId var, bool backward) {
    CL_BREAK_IF(OK_DLS != sh.valTargetKind(dls));

    // handle DLS Neq predicates and OK_MAY_EXIST
    const unsigned len = sh.segMinLength(dls) + objMinLength(sh, var);
    sh.segSetMinLength(dls, /* DLS 0+ */ 0);
    enlargeMayExist(sh, var);

    if (!backward)
        // jump to peer
        dls = dlSegPeer(sh, dls);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, var, dls);

    // store the pointer DLS -> VAR
    const BindingOff &off = sh.segBinding(dls);
    const TObjId nextPtr = sh.ptrAt(sh.valByOffset(dls, off.next));
    const TValId valNext = valOfPtrAt(sh, var, off.next);
    sh.objSetValue(nextPtr, valNext);
    sh.objReleaseId(nextPtr);

    // replace VAR by DLS
    const TValId headAt = sh.valByOffset(var, off.head);
    sh.valReplace(headAt, segHeadAt(sh, dls));
    REQUIRE_GC_ACTIVITY(sh, headAt, dlSegGobble);

    // handle DLS Neq predicates
    sh.segSetMinLength(dls, len);

    dlSegSyncPeerData(sh, dls);
}

void dlSegMerge(SymHeap &sh, TValId seg1, TValId seg2) {
    // handle DLS Neq predicates
    const unsigned len = sh.segMinLength(seg1) + sh.segMinLength(seg2);
    sh.segSetMinLength(seg1, /* DLS 0+ */ 0);
    sh.segSetMinLength(seg2, /* DLS 0+ */ 0);

    // dig peers
    const TValId peer1 = dlSegPeer(sh, seg1);
    const TValId peer2 = dlSegPeer(sh, seg2);

    // check for a failure of segDiscover()
    CL_BREAK_IF(sh.segBinding(seg1) != sh.segBinding(seg2));
    CL_BREAK_IF(nextValFromSeg(sh, peer1) != segHeadAt(sh, seg2));

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, seg1, seg2, /* bidir */ true);

    // preserve backLink
    const TValId valNext1 = nextValFromSeg(sh, seg1);
    const TObjId ptrNext2 = nextPtrFromSeg(sh, seg2);
    sh.objSetValue(ptrNext2, valNext1);
    sh.objReleaseId(ptrNext2);

    // replace both parts point-wise
    const TValId headAt = segHeadAt(sh,  seg1);
    const TValId peerAt = segHeadAt(sh, peer1);

    sh.valReplace(headAt, segHeadAt(sh,  seg2));
    sh.valReplace(peerAt, segHeadAt(sh, peer2));

    // destroy headAt and peerAt, including all prototypes -- either at once, or
    // one by one (depending on the shape of subgraph)
    REQUIRE_GC_ACTIVITY(sh, headAt, dlSegMerge);
    if (!collectJunk(sh, peerAt) && isPossibleToDeref(sh.valTarget(peerAt))) {
        CL_ERROR("dlSegMerge() failed to collect garbage"
                 ", peerAt still referenced");
        CL_BREAK_IF("collectJunk() has not been successful");
    }

    if (len)
        // handle DLS Neq predicates
        sh.segSetMinLength(seg2, len);

    dlSegSyncPeerData(sh, seg2);
}

bool /* jump next */ dlSegAbstractionStep(
        SymHeap                     &sh,
        const TValId                at,
        const TValId                next,
        const BindingOff            &off)
{
    const EObjKind kind = sh.valTargetKind(at);
    const EObjKind kindNext = sh.valTargetKind(next);
    CL_BREAK_IF(OK_SLS == kind || OK_SLS == kindNext);

    if (OK_DLS == kindNext) {
        if (OK_DLS == kind)
            // DLS + DLS
            dlSegMerge(sh, at, next);
        else
            // CONCRETE + DLS
            dlSegGobble(sh, next, at, /* backward */ true);

        return /* jump next */ true;
    }
    else {
        if (OK_DLS == kind)
            // DLS + CONCRETE
            dlSegGobble(sh, at, next, /* backward */ false);
        else
            // CONCRETE + CONCRETE
            dlSegCreate(sh, at, next, off);

        return /* nobody moves */ false;
    }
}

bool segAbstractionStep(
        SymHeap                     &sh,
        const BindingOff            &off,
        TValId                      *pCursor)
{
    const TValId at = *pCursor;

    // jump to peer in case of DLS
    TValId peer = at;
    if (OK_DLS == sh.valTargetKind(at))
        peer = dlSegPeer(sh, at);

    // jump to the next object (as we know such an object exists)
    const TValId next = nextRootObj(sh, peer, off.next);

    // check wheter he upcoming abstraction step is still doable
    EJoinStatus status;
    if (!joinDataReadOnly(&status, sh, off, at, next, 0))
        return false;

    if (isDlsBinding(off)) {
        // DLS
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
        const bool jumpNext = dlSegAbstractionStep(sh, at, next, off);
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
        if (!jumpNext)
            // stay in place
            return true;
    }
    else {
        // SLS
        slSegAbstractionStep(sh, at, next, off);
    }

    // move the cursor one step forward
    *pCursor = next;
    return true;
}

void adjustAbstractionThreshold(
        unsigned                    *pThr,
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry,
        const unsigned              lenTotal)
{
    const unsigned thrOrig = *pThr;
    CL_BREAK_IF(!thrOrig);
    if (1 == thrOrig)
        // we are already at the lowest possible threshold
        return;

    TValId cursor = entry;

    for (unsigned pos = 0; pos <= lenTotal; ++pos) {
        if (VT_ABSTRACT == sh.valTarget(cursor)) {
            CL_DEBUG("    adjustAbstractionThreshold() changes innerSegLen: "
                    << thrOrig << " -> 1");

            *pThr = 1;
            return;
        }

        if (OK_DLS == sh.valTargetKind(cursor))
            cursor = dlSegPeer(sh, cursor);

        cursor = nextRootObj(sh, cursor, off.next);
    }
}

bool considerAbstraction(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry,
        const unsigned              lenTotal)
{
    unsigned thr;
    const char *name;

    if (isDlsBinding(off)) {
        thr = dlsThreshold;
        name = "DLS";
    }
    else {
        thr = slsThreshold;
        name = "SLS";
    }

    adjustAbstractionThreshold(&thr, sh, off, entry, lenTotal);

    // check whether the threshold is satisfied or not
    if (lenTotal < thr) {
        CL_DEBUG("<-- length (" << lenTotal
                << ") of the longest segment is under the threshold ("
                << thr << ")");
        return false;
    }

    CL_DEBUG("    --- length of the longest segment is " << lenTotal);

    // cursor
    TValId cursor = entry;

    // handle sparePrefix/spareSuffix
    CL_DEBUG("    AAA initiating " << name
             << " abstraction of length " << lenTotal);

    LDP_INIT(symabstract, name);
    LDP_PLOT(symabstract, sh);

    for (unsigned i = 0; i < lenTotal; ++i) {
        if (!segAbstractionStep(sh, off, &cursor)) {
            CL_DEBUG("<-- validity of next " << (lenTotal - i - 1)
                    << " abstraction step(s) broken, forcing re-discovery...");

            if (i)
                return true;

            CL_BREAK_IF("segAbstractionStep() failed, nothing has been done");
            return false;
        }

        LDP_PLOT(symabstract, sh);
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void dlSegReplaceByConcrete(SymHeap &sh, TValId seg, TValId peer) {
    LDP_INIT(symabstract, "dlSegReplaceByConcrete");
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));

    // first kill any related Neq predicates, we're going to concretize anyway
    sh.segSetMinLength(seg, /* DLS 0+ */ 0);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));

    // take the value of 'next' pointer from peer
    const TObjId peerPtr = prevPtrFromSeg(sh, seg);
    const TValId valNext = nextValFromSeg(sh, peer);
    sh.objSetValue(peerPtr, valNext);
    sh.objReleaseId(peerPtr);

    // redirect all references originally pointing to peer to the current object
    redirectRefs(sh,
            /* pointingFrom */ VAL_INVALID,
            /* pointingTo   */ peer,
            /* redirectTo   */ seg);

    // destroy peer, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, peer, dlSegReplaceByConcrete);

    // concretize self
    sh.valTargetSetConcrete(seg);
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
}

void spliceOutListSegment(
        SymHeap                 &sh,
        const TValId            seg,
        const TValId            peer,
        const TValId            valNext)
{
    LDP_INIT(symabstract, "spliceOutListSegment");
    LDP_PLOT(symabstract, sh);

    const TOffset offHead = sh.segBinding(seg).head;

    if (OK_DLS == sh.valTargetKind(seg)) {
        // OK_DLS --> unlink peer
        CL_BREAK_IF(seg == peer);
        CL_BREAK_IF(offHead != sh.segBinding(peer).head);
        const TValId valPrev = nextValFromSeg(sh, seg);
        redirectRefs(sh,
                /* pointingFrom */ VAL_INVALID,
                /* pointingTo   */ peer,
                /* redirectTo   */ valPrev,
                /* offHead      */ offHead);
    }

    // unlink self
    redirectRefs(sh,
            /* pointingFrom */ VAL_INVALID,
            /* pointingTo   */ seg,
            /* redirectTo   */ valNext,
            /* offHead      */ offHead);

    // destroy peer in case of DLS
    if (peer != seg && collectJunk(sh, peer))
        CL_DEBUG("spliceOutListSegment() drops a sub-heap (peer)");

    // destroy self, including all nested prototypes
    if (collectJunk(sh, seg))
        CL_DEBUG("spliceOutListSegment() drops a sub-heap (seg)");

    LDP_PLOT(symabstract, sh);
}

unsigned /* len */ spliceOutSegmentIfNeeded(
        SymHeap                 &sh,
        const TValId            seg,
        const TValId            peer,
        TSymHeapList            &todo)
{
    const unsigned len = sh.segMinLength(seg);
    if (!len) {
        // possibly empty LS
        SymHeap sh0(sh);
        const TValId valNext = nextValFromSeg(sh0, peer);
        spliceOutListSegment(sh0, seg, peer, valNext);
        todo.push_back(sh0);
    }

    LDP_INIT(symabstract, "concretizeObj");
    LDP_PLOT(symabstract, sh);

    if (!len)
        return /* LS 0+ */ 0;

    // forget the current minimal length for a while
    sh.segSetMinLength(seg, 0);

    // we are going to detach one node
    return len - 1;
}

void abstractIfNeeded(SymHeap &sh) {
#if SE_DISABLE_SLS && SE_DISABLE_DLS
    return;
#endif
    BindingOff          off;
    TValId              entry;
    unsigned            len;

    while ((len = discoverBestAbstraction(sh, &off, &entry))) {
        if (!considerAbstraction(sh, off, entry, len))
            // the best abstraction given is unfortunately not good enough
            break;

        // some part of the symbolic heap has just been successfully abstracted,
        // let's look if there remains anything else suitable for abstraction
    }
}

void concretizeObj(SymHeap &sh, TValId addr, TSymHeapList &todo) {
    const TValId seg = sh.valRoot(addr);
    const TValId peer = segPeer(sh, seg);

    // handle the possibly empty variant (if exists)
    const unsigned lenRemains = spliceOutSegmentIfNeeded(sh, seg, peer, todo);

    const EObjKind kind = sh.valTargetKind(seg);
    if (OK_MAY_EXIST == kind) {
        // this kind is much easier than regular list segments
        sh.valTargetSetConcrete(seg);
        LDP_PLOT(symabstract, sh);
        return;
    }

    // duplicate self as abstract object
    const TValId dup = sh.valClone(seg);
    const TValId dupHead = segHeadAt(sh, dup);
    if (OK_DLS == kind) {
        // DLS relink
        const TObjId ptr = prevPtrFromSeg(sh, peer);
        sh.objSetValue(ptr, dupHead);
        sh.objReleaseId(ptr);
    }

    // duplicate all unknown values, to keep the prover working
    clonePrototypes(sh, seg, dup);

    // resolve the relative placement of the 'next' pointer
    const BindingOff off = sh.segBinding(seg);
    const TOffset offNext = (OK_SLS == kind)
        ? off.next
        : off.prev;

    // concretize self and recover the list
    sh.valTargetSetConcrete(seg);
    const TObjId nextPtr = sh.ptrAt(sh.valByOffset(seg, offNext));
    sh.objSetValue(nextPtr, dupHead);
    sh.objReleaseId(nextPtr);

    if (OK_DLS == kind) {
        // update DLS back-link
        const TObjId backLink = sh.ptrAt(sh.valByOffset(dup, off.next));
        const TValId headAddr = sh.valByOffset(seg, off.head);
        sh.objSetValue(backLink, headAddr);
        sh.objReleaseId(backLink);
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
    }

    sh.segSetMinLength(dup, lenRemains);

    LDP_PLOT(symabstract, sh);
}

bool spliceOutAbstractPathCore(
        SymHeap                 &sh,
        const TValId            beg,
        const TValId            endPoint,
        const bool              readOnlyMode)
{
    // NOTE: If there is a cycle consisting of empty list segments only, we will
    // loop indefinitely.  However, the basic list segment axiom guarantees that
    // there is no such cycle.

    TValId seg = beg;
    TValId peer, valNext;

    for (;;) {
        const EValueTarget code = sh.valTarget(seg);
        if (VT_ABSTRACT != code || objMinLength(sh, seg)) {
            // we are on a wrong way already...
            CL_BREAK_IF(!readOnlyMode);
            return false;
        }

        peer = segPeer(sh, seg);
        valNext = nextValFromSeg(sh, peer);

        if (!readOnlyMode && beg != seg)
            destroyRootAndCollectJunk(sh, seg);

        if (valNext == endPoint)
            // we have the chain we are looking for
            break;

        if (!readOnlyMode && seg != peer)
            destroyRootAndCollectJunk(sh, peer);

        seg = sh.valRoot(valNext);
    }

    if (!readOnlyMode)
        spliceOutListSegment(sh, beg, peer, valNext);

    return true;
}

bool spliceOutAbstractPath(SymHeap &sh, TValId atAddr, TValId pointingTo) {
    const TValId seg = sh.valRoot(atAddr);
    const TValId peer = segPeer(sh, seg);

    if (pointingTo == peer && peer != seg) {
        // assume identity over the two parts of a DLS
        dlSegReplaceByConcrete(sh, seg, peer);
        return true;
    }

    // if atAddr is above/bellow head, we need to shift pointingTo accordingly
    const TOffset off = sh.valOffset(atAddr) - sh.segBinding(seg).head;
    const TValId endPoint = sh.valByOffset(pointingTo, off);

    if (!spliceOutAbstractPathCore(sh, seg, endPoint, /* readOnlyMode */ true))
        // giving up
        return false;

    if (!spliceOutAbstractPathCore(sh, seg, endPoint, /* readOnlyMode */ false))
        CL_BREAK_IF("spliceOutAbstractPathCore() completely broken");

    return true;
}
