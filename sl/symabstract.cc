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

#define REQUIRE_GC_ACTIVITY(sh, val, fnc) do {                                 \
    if (collectJunk(sh, val))                                                  \
        break;                                                                 \
    CL_ERROR(#fnc "() failed to collect garbage, " #val " still referenced");  \
    CL_BREAK_IF("REQUIRE_GC_ACTIVITY has not been successful");                \
} while (0)

/// common configuration template for abstraction triggering
struct AbstractionThreshold {
    unsigned sparePrefix;
    unsigned innerSegLen;
    unsigned spareSuffix;
};

/// abstraction trigger threshold for SLS
static struct AbstractionThreshold slsThreshold = {
    /* sparePrefix */ 0,
    /* innerSegLen */ 1,
    /* spareSuffix */ 0
};

/// abstraction trigger threshold for DLS
static struct AbstractionThreshold dlsThreshold = {
    /* sparePrefix */ 0,
    /* innerSegLen */ 1,
    /* spareSuffix */ 0
};

// visitor
struct UnknownValuesDuplicator {
    std::set<TObjId> ignoreList;

    bool operator()(SymHeap &sh, TObjId obj) const {
        if (hasKey(ignoreList, obj))
            return /* continue */ true;

        const TValId valOld = sh.valueOf(obj);
        if (valOld <= 0)
            return /* continue */ true;

        const EUnknownValue code = sh.valGetUnknown(valOld);
        switch (code) {
            case UV_ABSTRACT:
            case UV_KNOWN:
                return /* continue */ true;

            case UV_UNKNOWN:
            case UV_DONT_CARE:
            case UV_UNINITIALIZED:
                break;
        }

        // duplicate unknown value
        const TValId valNew = sh.valClone(valOld);
        sh.objSetValue(obj, valNew);

        return /* continue */ true;
    }
};

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TObjId obj) {
    UnknownValuesDuplicator visitor;
    buildIgnoreList(visitor.ignoreList, sh, obj);

    // traverse all sub-objects
    traverseLiveObjs(sh, sh.placedAt(obj), visitor);
}

void detachClonedPrototype(
        SymHeap                 &sh,
        const TObjId            proto,
        const TObjId            clone,
        const TObjId            rootDst,
        const TObjId            rootSrc)
{
    const bool isRootDls = (OK_DLS == objKind(sh, rootDst));
    CL_BREAK_IF(isRootDls && (OK_DLS != objKind(sh, rootSrc)));

    TObjId rootSrcPeer = OBJ_INVALID;
    if (isRootDls) {
        rootSrcPeer = dlSegPeer(sh, rootSrc);
        CL_BREAK_IF(dlSegPeer(sh, rootDst) != rootSrcPeer);
    }

    redirectInboundEdges(sh, rootDst, proto, clone);
    redirectInboundEdges(sh, proto, rootDst, rootSrc);
    if (isRootDls)
        redirectInboundEdges(sh, clone, rootSrcPeer, rootDst);

    if (OK_DLS == objKind(sh, proto)) {
        const TObjId protoPeer = dlSegPeer(sh, proto);
        const TObjId clonePeer = dlSegPeer(sh, clone);
        redirectInboundEdges(sh, rootDst, protoPeer, clonePeer);
        redirectInboundEdges(sh, protoPeer, rootDst, rootSrc);
        if (isRootDls)
            redirectInboundEdges(sh, clonePeer, rootSrcPeer, rootDst);
    }
}

TObjId protoClone(SymHeap &sh, const TObjId proto) {
    TObjId clone = OBJ_INVALID;

    if (objIsSeg(sh, proto)) {
        // clone segment prototype
        clone = segClone(sh, proto);
        segSetProto(sh, clone, false);
        return clone;
    }
    else {
        // clone bare prototype
        clone = objDup(sh, proto);
        sh.valTargetSetProto(sh.placedAt(clone), false);
    }

    duplicateUnknownValues(sh, clone);

    return clone;
}

struct ProtoFinder {
    std::set<TObjId> protos;

    bool operator()(SymHeap &sh, TObjId sub) {
        const TValId val = sh.valueOf(sub);
        if (val <= 0)
            return /* continue */ true;

        if (sh.valOffset(val))
            // TODO: support for prototypes not pointed by roots?
            return /* continue */ true;

        if (sh.valTargetIsProto(val))
            protos.insert(sh.objAt(val, CL_TYPE_STRUCT));

        return /* continue */ true;
    }
};

// FIXME: this completely ignores Neq predicates for instance...
void cloneGenericPrototype(
        SymHeap                 &sh,
        const TObjId            proto,
        const TObjId            rootDst,
        const TObjId            rootSrc)
{
    std::vector<TObjId>         protoList;
    std::vector<TObjId>         cloneList;
    std::vector<int>            lengthList;
    std::set<TObjId>            haveSeen;
    std::stack<TObjId>          todo;
    todo.push(proto);
    haveSeen.insert(proto);

    CL_DEBUG("WARNING: cloneGenericPrototype() is just a hack for now!");

    while (!todo.empty()) {
        const TObjId proto = todo.top();
        todo.pop();
        protoList.push_back(proto);

        ProtoFinder visitor;
        traverseLivePtrs(sh, sh.placedAt(proto), visitor);
        BOOST_FOREACH(const TObjId obj, visitor.protos) {
            if (!insertOnce(haveSeen, obj))
                continue;

            if (OK_DLS == objKind(sh, obj) &&
                    !insertOnce(haveSeen, dlSegPeer(sh, obj)))
                continue;

            todo.push(obj);
        }
    }

    // allocate some space for clone IDs and minimal lengths
    const unsigned cnt = protoList.size();
    CL_BREAK_IF(!cnt);
    cloneList.resize(cnt);
    lengthList.resize(cnt);

    // clone the prototypes while reseting the minimal size to zero
    for (unsigned i = 0; i < cnt; ++i) {
        const TObjId proto = protoList[i];
        if (objIsSeg(sh, proto)) {
            lengthList[i] = segMinLength(sh, sh.placedAt(proto));
            segSetMinLength(sh, sh.placedAt(proto), /* LS 0+ */ 0);
        }
        else
            lengthList[i] = -1;

        cloneList[i] = protoClone(sh, proto);
    }

    // FIXME: works, but likely to kill the CPU
    for (unsigned i = 0; i < cnt; ++i) {
        const TObjId proto = protoList[i];
        const TObjId clone = cloneList[i];
        detachClonedPrototype(sh, proto, clone, rootDst, rootSrc);

        for (unsigned j = 0; j < cnt; ++j) {
            if (i == j)
                continue;

            const TObjId otherProto = protoList[j];
            const TObjId otherClone = cloneList[j];
            detachClonedPrototype(sh, proto, clone, otherClone, otherProto);
        }
    }

    // finally restore the minimal size of all segments
    for (unsigned i = 0; i < cnt; ++i) {
        const TObjId proto = protoList[i];
        const TObjId clone = cloneList[i];
        const int len = lengthList[i];
        if (len <= 0)
            // -1 means "not a segment"
            continue;

        segSetMinLength(sh, sh.placedAt(proto), len);
        segSetMinLength(sh, sh.placedAt(clone), len);
    }
}

// visitor
struct ProtoCloner {
    std::set<TObjId> ignoreList;
    TObjId rootDst;
    TObjId rootSrc;

    bool operator()(SymHeap &sh, TObjId obj) const {
        if (hasKey(ignoreList, obj))
            return /* continue */ true;

        const TValId valOld = sh.valueOf(obj);
        if (valOld <= 0)
            return /* continue */ true;

        const EUnknownValue code = sh.valGetUnknown(valOld);
        switch (code) {
            case UV_UNKNOWN:
            case UV_DONT_CARE:
            case UV_UNINITIALIZED:
                return /* continue */ true;

            case UV_ABSTRACT:
            case UV_KNOWN:
                break;
        }

        // check if we point to prototype, or shared data
        if (sh.valTargetIsProto(valOld)) {
            const TObjId target = objRootByVal(sh, valOld);
            cloneGenericPrototype(sh, target, rootDst, rootSrc);
        }

        return /* continue */ true;
    }
};

struct ValueSynchronizer {
    std::set<TObjId>    ignoreList;

    bool operator()(SymHeap &sh, TObjPair item) const {
        const TObjId src = item.first;
        const TObjId dst = item.second;
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

void dlSegSyncPeerData(SymHeap &sh, const TValId dlsAt) {
    // TODO: remove this
    const TObjId dls = sh.objAt(dlsAt);

    const TObjId peer = dlSegPeer(sh, dls);
    ValueSynchronizer visitor;
    buildIgnoreList(visitor.ignoreList, sh, dls);

    // if there was "a pointer to self", it should remain "a pointer to self";
    TObjList refs;
    sh.pointedBy(refs, sh.placedAt(dls));
    std::copy(refs.begin(), refs.end(),
              std::inserter(visitor.ignoreList, visitor.ignoreList.begin()));

    const TObjPair item(dls, peer);
    traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
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

    if (OK_DLS == sh.valTargetKind(dstAt))
        dlSegSyncPeerData(sh, dstAt);

    if (bidir && OK_DLS == sh.valTargetKind(srcAt))
        dlSegSyncPeerData(sh, srcAt);
}

void clonePrototypes(SymHeap &sh, TValId objAt, TValId dupAt) {
    // remove this
    TObjId obj = sh.objAt(objAt);
    TObjId dup = sh.objAt(dupAt);

    duplicateUnknownValues(sh, obj);

    ProtoCloner visitor;
    visitor.rootDst = obj;
    visitor.rootSrc = dup;
    buildIgnoreList(visitor.ignoreList, sh, obj);

    // traverse all live sub-objects
    traverseLivePtrs(sh, sh.placedAt(obj), visitor);

    // if there was "a pointer to self", it should remain "a pointer to self";
    // however "self" has been changed, so that a redirection is necessary
    redirectInboundEdges(sh, dup, obj, dup);
}

void slSegAbstractionStep(SymHeap &sh, TValId *pCursor, const BindingOff &off)
{
    const TObjId obj = sh.objAt(*pCursor);
    const TObjId objPtrNext = ptrObjByOffset(sh, obj, off.next);
    const TValId valNext = sh.valueOf(objPtrNext);
    CL_BREAK_IF(valNext <= 0);

    // read minimal length of 'obj' and set it temporarily to zero
    unsigned len = objMinLength(sh, sh.placedAt(obj));
    if (objIsSeg(sh, obj))
        segSetMinLength(sh, sh.placedAt(obj), /* SLS 0+ */ 0);

    // jump to the next object
    const TObjId objNext = objRootByVal(sh, valNext);
    len += objMinLength(sh, sh.placedAt(objNext));
    if (OK_SLS == sh.valTargetKind(valNext))
        segSetMinLength(sh, sh.placedAt(objNext), /* SLS 0+ */ 0);
    else
        // abstract the _next_ object
        sh.valTargetSetAbstract(valNext, OK_SLS, off);

    // merge data
    CL_BREAK_IF(OK_SLS != objKind(sh, objNext));
    abstractNonMatchingValues(sh, sh.placedAt(obj), sh.placedAt(objNext));

    // replace all references to 'head'
    const TOffset offHead = sh.segBinding(valNext).head;
    const TValId headAt = sh.placedAt(compObjByOffset(sh, obj, offHead));
    sh.valReplace(headAt, segHeadAddr(sh, objNext));

    // destroy self, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, headAt, slSegAbstractionStep);

    if (len)
        // declare resulting segment's minimal length
        segSetMinLength(sh, sh.placedAt(objNext), len);

    // move to the next object
    *pCursor = sh.placedAt(objNext);
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
    dlSegSetMinLength(sh, a1, len);

    dlSegSyncPeerData(sh, a1);
}

void dlSegGobble(SymHeap &sh, TValId dls, TValId var, bool backward) {
    CL_BREAK_IF(OK_DLS != sh.valTargetKind(dls));

    // handle DLS Neq predicates and OK_MAY_EXIST
    const unsigned len = dlSegMinLength(sh, dls) + objMinLength(sh, var);
    dlSegSetMinLength(sh, dls, /* DLS 0+ */ 0);
    enlargeMayExist(sh, var);

    if (!backward)
        // jump to peer
        dls = dlSegPeer(sh, dls);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, var, dls);

    // store the pointer DLS -> VAR
    const BindingOff &off = sh.segBinding(dls);
    const TObjId dlsNextPtr = sh.ptrAt(sh.valByOffset(dls, off.next));
    const TObjId varNextPtr = sh.ptrAt(sh.valByOffset(var, off.next));
    sh.objSetValue(dlsNextPtr, sh.valueOf(varNextPtr));

    // replace VAR by DLS
    const TValId headAt = sh.valByOffset(var, off.head);
    sh.valReplace(headAt, segHeadAt(sh, dls));
    REQUIRE_GC_ACTIVITY(sh, headAt, dlSegGobble);

    // handle DLS Neq predicates
    dlSegSetMinLength(sh, dls, len);

    dlSegSyncPeerData(sh, dls);
}

void dlSegMerge(SymHeap &sh, TValId seg1, TValId seg2) {
    // handle DLS Neq predicates
    const unsigned len = dlSegMinLength(sh, seg1) + dlSegMinLength(sh, seg2);
    dlSegSetMinLength(sh, seg1, /* DLS 0+ */ 0);
    dlSegSetMinLength(sh, seg2, /* DLS 0+ */ 0);

    // check for a failure of segDiscover()
    CL_BREAK_IF(sh.segBinding(seg1) != sh.segBinding(seg2));

    const TValId peer1 = dlSegPeer(sh, seg1);
#ifndef NDEBUG
    const TObjId nextPtr = nextPtrFromSeg(sh, peer1);
    const TValId valNext = sh.valueOf(nextPtr);
    CL_BREAK_IF(valNext != segHeadAt(sh, seg2));
#endif

    const TValId peer2 = dlSegPeer(sh, seg2);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, seg1, seg2, /* bidir */ true);

    // preserve backLink
    const TValId valNext2 = sh.valueOf(nextPtrFromSeg(sh, seg1));
    sh.objSetValue(nextPtrFromSeg(sh, seg2), valNext2);

    // replace both parts point-wise
    const TValId headAt = segHeadAt(sh,  seg1);
    const TValId peerAt = segHeadAt(sh, peer1);

    sh.valReplace(headAt, segHeadAt(sh,  seg2));
    sh.valReplace(peerAt, segHeadAt(sh, peer2));

    // destroy headAt and peerAt, including all prototypes -- either at once, or
    // one by one (depending on the shape of subgraph)
    REQUIRE_GC_ACTIVITY(sh, headAt, dlSegMerge);
    if (!collectJunk(sh, peerAt) && 0 < sh.pointsTo(peerAt)) {
        CL_ERROR("dlSegMerge() failed to collect garbage"
                 ", peerAt still referenced");
        CL_BREAK_IF("collectJunk() has not been successful");
    }

    if (len)
        // handle DLS Neq predicates
        dlSegSetMinLength(sh, seg2, len);

    dlSegSyncPeerData(sh, seg2);
}

void dlSegAbstractionStep(SymHeap &sh, TValId *pCursor, const BindingOff &off)
{
    // the first object is clear
    const TValId a1 = *pCursor;

    // we'll find the next one later on
    TValId a2 = a1;

    const EObjKind kind = sh.valTargetKind(a1);
    switch (kind) {
        case OK_SLS:
            // *** segDiscover() failure detected ***
            CL_TRAP;

        case OK_DLS:
            // jump to peer
            a2 = dlSegPeer(sh, a2);

            // jump to the next object (as we know such an object exists)
            a2 = nextRootObj(sh, a2, sh.segBinding(a2).next);
            if (OK_DLS != sh.valTargetKind(a2)) {
                // DLS + VAR
                dlSegGobble(sh, a1, a2, /* backward */ false);
                return;
            }

            // DLS + DLS
            dlSegMerge(sh, a1, a2);
            break;

        case OK_MAY_EXIST:
        case OK_CONCRETE:
            // jump to the next object (as we know such an object exists)
            a2 = nextRootObj(sh, a2, off.next);
            if (OK_DLS != sh.valTargetKind(a2)) {
                // VAR + VAR
                dlSegCreate(sh, a1, a2, off);
                return;
            }

            // VAR + DLS
            dlSegGobble(sh, a2, a1, /* backward */ true);
            break;
    }

    // the current object has been just consumed, move to the next one
    *pCursor = a2;

#ifndef NDEBUG
    // just check if the Neq predicates work well so far
    dlSegMinLength(sh, a2);
#endif
}

void segAbstractionStep(
        SymHeap                     &sh,
        const BindingOff            &off,
        TValId                      *pCursor)
{
    if (isDlsBinding(off)) {
        // DLS
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
        dlSegAbstractionStep(sh, pCursor, off);
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
        return;
    }

    // SLS
    slSegAbstractionStep(sh, pCursor, off);
}

bool considerAbstraction(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TValId                entry,
        const unsigned              lenTotal)
{
    const bool isSls = !isDlsBinding(off);
    const AbstractionThreshold &thr = (isSls)
        ? slsThreshold
        : dlsThreshold;

    // check whether the threshold is satisfied or not
    const unsigned threshold = thr.innerSegLen
        + thr.sparePrefix
        + thr.spareSuffix;
    if (lenTotal < threshold) {
        CL_DEBUG("<-- length (" << lenTotal
                << ") of the longest segment is under the threshold ("
                << threshold << ")");
        return false;
    }

    CL_DEBUG("    --- length of the longest segment is " << lenTotal
            << ", prefix=" << thr.sparePrefix
            << ", suffix=" << thr.spareSuffix);

    // cursor
    TValId cursor = entry;

    // handle sparePrefix/spareSuffix
    int len = lenTotal - thr.sparePrefix - thr.spareSuffix;
    for (unsigned i = 0; i < thr.sparePrefix; ++i)
        cursor = nextRootObj(sh, cursor, off.next);

    const char *name = (isSls)
        ? "SLS"
        : "DLS";
    CL_DEBUG("    AAA initiating " << name
             << " abstraction of length " << len);

    LDP_INIT(symabstract, name);
    LDP_PLOT(symabstract, sh);

    for (int i = 0; i < len; ++i) {
        segAbstractionStep(sh, off, &cursor);
        LDP_PLOT(symabstract, sh);
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void segReplaceRefs(SymHeap &sh, TValId seg, TValId valNext) {
    const TValId headAt = segHeadAt(sh, seg);
    const TOffset offHead = sh.valOffset(headAt);
    sh.valReplace(headAt, valNext);

    const TObjId next = sh.objAt(valNext);
    switch (next) {
        case OBJ_DELETED:
        case OBJ_LOST:
            break;

        default:
            CL_BREAK_IF(0 < valNext && next < 0);
    }

    // TODO: check types in debug build
    TObjList refs;
    sh.pointedBy(refs, seg);
    BOOST_FOREACH(const TObjId obj, refs) {
        if (VAL_NULL == valNext) {
            // FIXME: not correct in all cases
            sh.objSetValue(obj, VAL_NULL);
            continue;
        }
            
        const TValId targetAt = sh.valueOf(obj);
        const TObjId target = sh.objAt(targetAt);
        if (next < 0 || target < 0) {
            CL_DEBUG("WARNING: suboptimal implementation of segReplaceRefs()");
            const TValId val = sh.valClone(valNext);
            sh.objSetValue(obj, val);
            continue;
        }

        // redirect!
        const TOffset off = sh.valOffset(targetAt) - offHead;
        const TValId val = sh.valByOffset(valNext, off);
        sh.objSetValue(obj, val);
    }
}

void dlSegReplaceByConcrete(SymHeap &sh, TValId seg, TValId peer) {
    LDP_INIT(symabstract, "dlSegReplaceByConcrete");
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));

    // first kill any related Neq predicates, we're going to concretize anyway
    dlSegSetMinLength(sh, seg, /* DLS 0+ */ 0);

    // take the value of 'next' pointer from peer
    const TObjId peerPtr = prevPtrFromSeg(sh, seg);
    const TValId valNext = sh.valueOf(nextPtrFromSeg(sh, peer));
    sh.objSetValue(peerPtr, valNext);

    // redirect all references originally pointing to peer to the current object
    redirectInboundEdges(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ /* FIXME */ sh.objAt(peer),
            /* redirectTo   */ /* FIXME */ sh.objAt(seg));

    // destroy peer, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, peer, dlSegReplaceByConcrete);

    // concretize self
    sh.valTargetSetConcrete(seg);

    // this can't fail (at least I hope so...)
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
}

void spliceOutListSegmentCore(SymHeap &sh, TValId seg, TValId peer) {
    LDP_INIT(symabstract, "spliceOutListSegmentCore");
    LDP_PLOT(symabstract, sh);

    // read valNext now as we may overwrite it during unlink of peer
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValId valNext = sh.valueOf(next);

    if (seg != peer) {
        // OK_DLS --> unlink peer
        const TObjId prevPtr = nextPtrFromSeg(sh, seg);
        const TValId valPrev = sh.valueOf(prevPtr);
        segReplaceRefs(sh, peer, valPrev);
    }

    // unlink self
    segReplaceRefs(sh, seg, valNext);

    // destroy peer in case of DLS
    if (peer != seg && collectJunk(sh, peer))
        CL_DEBUG("spliceOutSegmentIfNeeded() drops a sub-heap (peer)");

    // destroy self, including all nested prototypes
    if (collectJunk(sh, seg))
        CL_DEBUG("spliceOutSegmentIfNeeded() drops a sub-heap (seg)");

    LDP_PLOT(symabstract, sh);
}

unsigned /* len */ spliceOutSegmentIfNeeded(
        SymHeap                 &sh,
        const TValId            seg,
        const TValId            peer,
        TSymHeapList            &todo)
{
    const unsigned len = segMinLength(sh, seg);
    if (len) {
        LDP_INIT(symabstract, "spliceOutSegmentIfNeeded");
        LDP_PLOT(symabstract, sh);

        // drop any existing Neq predicates
        segSetMinLength(sh, seg, 0);

        LDP_PLOT(symabstract, sh);
        return len - 1;
    }

    // possibly empty LS
    SymHeap sh0(sh);
    spliceOutListSegmentCore(sh0, seg, peer);
    todo.push_back(sh0);
    return /* LS 0+ */ 0;
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

    LDP_INIT(symabstract, "concretizeObj");
    LDP_PLOT(symabstract, sh);

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

    if (OK_DLS == kind) {
        // update DLS back-link
        const TObjId backLink = sh.ptrAt(sh.valByOffset(dup, off.next));
        const TValId headAddr = sh.valByOffset(seg, off.head);
        sh.objSetValue(backLink, headAddr);
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
    }

    segSetMinLength(sh, dup, lenRemains);

    LDP_PLOT(symabstract, sh);
}

bool spliceOutListSegment(SymHeap &sh, TValId atAddr, TValId pointingTo) {
    const TValId seg = sh.valRoot(atAddr);
    const TValId peer = segPeer(sh, seg);

    if (pointingTo == peer && peer != seg) {
        // assume identity over the two parts of a DLS
        dlSegReplaceByConcrete(sh, seg, peer);
        return true;
    }

    const TValId valNext = sh.valueOf(nextPtrFromSeg(sh, peer));
    if (pointingTo == valNext) {
        // assume empty segment
        spliceOutListSegmentCore(sh, seg, peer);
        return true;
    }

    // giving up
    return false;
}
