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

#define REQUIRE_GC_ACTIVITY(sh, val, fnc) do {                                 \
    if (collectJunk(sh, val))                                                  \
        break;                                                                 \
    CL_ERROR(#fnc "() failed to collect garbage, " #val " still referenced");  \
    CL_BREAK_IF("REQUIRE_GC_ACTIVITY has not been successful");                \
} while (0)

#if DEBUG_SYMABSTRACT
#   include "symdump.hh"
#   define FIXW(w) std::fixed << std::setfill('0') << std::setw(w)
namespace {
    static int cntAbstraction = -1;
    static int cntAbstractionStep;
    static std::string abstractionName;

    void debugPlotInit(std::string name) {
        ++::cntAbstraction;
        ::cntAbstractionStep = 0;
        ::abstractionName = name;
    }

    std::string debugPlotName() {
        std::ostringstream str;

        str << "symabstract-" << FIXW(4) << ::cntAbstraction
            << "-" << ::abstractionName
            << "-" << FIXW(4) << (::cntAbstractionStep++);

        return str.str();
    }

    void debugPlot(const SymHeap &sh) {
        std::string name = debugPlotName();
        dump_plot(sh, name.c_str());
    }
} // namespace

#else // DEBUG_SYMABSTRACT
namespace {
    void debugPlotInit(std::string) { }
    void debugPlot(const SymHeap &) { }
}
#endif // DEBUG_SYMABSTRACT

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

        const TValueId valOld = sh.valueOf(obj);
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
        const TValueId valNew = sh.valDuplicateUnknown(valOld);
        sh.objSetValue(obj, valNew);

        return /* continue */ true;
    }
};

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TObjId obj) {
    UnknownValuesDuplicator visitor;
    buildIgnoreList(visitor.ignoreList, sh, obj);

    // traverse all sub-objects
    traverseSubObjs(sh, obj, visitor, /* leavesOnly */ true);
}

void detachClonedPrototype(
        SymHeap                 &sh,
        const TObjId            proto,
        const TObjId            clone,
        const TObjId            rootDst,
        const TObjId            rootSrc)
{
    const bool isRootDls = (OK_DLS == sh.objKind(rootDst));
    CL_BREAK_IF(isRootDls && (OK_DLS != sh.objKind(rootSrc)));

    TObjId rootSrcPeer = OBJ_INVALID;
    if (isRootDls) {
        rootSrcPeer = dlSegPeer(sh, rootSrc);
        CL_BREAK_IF(dlSegPeer(sh, rootDst) != rootSrcPeer);
    }

    redirectInboundEdges(sh, rootDst, proto, clone);
    redirectInboundEdges(sh, proto, rootDst, rootSrc);
    if (isRootDls)
        redirectInboundEdges(sh, clone, rootSrcPeer, rootDst);

    if (OK_DLS == sh.objKind(proto)) {
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
        clone = sh.objDup(proto);
        sh.objSetProto(clone, false);
    }

    duplicateUnknownValues(sh, clone);

    return clone;
}

struct ProtoFinder {
    std::set<TObjId> protos;

    bool operator()(SymHeap &sh, TObjId sub) {
        const TObjId target = objRootByPtr(sh, sub);
        if (target <= 0)
            return /* continue */ true;

        if (sh.objIsProto(target))
            protos.insert(target);

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
        traverseSubObjs(sh, proto, visitor, /* leavesOnly */ true);
        BOOST_FOREACH(const TObjId obj, visitor.protos) {
            if (!insertOnce(haveSeen, obj))
                continue;

            if (OK_DLS == sh.objKind(obj) &&
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
            lengthList[i] = segMinLength(sh, proto);
            segSetMinLength(sh, proto, /* LS 0+ */ 0);
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

        segSetMinLength(sh, proto, len);
        segSetMinLength(sh, clone, len);
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

        const TValueId valOld = sh.valueOf(obj);
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
        const TObjId target = objRootByVal(sh, valOld);
        if (sh.objIsProto(target))
            cloneGenericPrototype(sh, target, rootDst, rootSrc);

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
        TValueId valSrc = sh.valueOf(src);
        TValueId valDst = sh.valueOf(dst);
        sh.objSetValue(dst, valSrc);

        // if the last reference is gone, we have a problem
        if (collectJunk(sh, valDst))
            CL_DEBUG("    ValueSynchronizer drops a sub-heap (dlSegPeer)");

        return /* continue */ true;
    }
};

void dlSegSyncPeerData(SymHeap &sh, const TObjId dls) {
    const TObjId peer = dlSegPeer(sh, dls);
    ValueSynchronizer visitor;
    buildIgnoreList(visitor.ignoreList, sh, dls);

    // if there was "a pointer to self", it should remain "a pointer to self";
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, dls, /* toInsideOnly */ false);
    std::copy(refs.begin(), refs.end(),
              std::inserter(visitor.ignoreList, visitor.ignoreList.begin()));

    const TObjPair item(dls, peer);
    traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
}

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(SymHeap &sh, TObjId src, TObjId dst,
                               bool bidir = false)
{
    if (!joinData(sh, dst, src, bidir))
        CL_BREAK_IF("joinData() failed, failure of segDiscover()?");

    if (OK_DLS == sh.objKind(dst))
        dlSegSyncPeerData(sh, dst);

    if (bidir && OK_DLS == sh.objKind(src))
        dlSegSyncPeerData(sh, src);

    // drop any dangling Neq predicates
    sh.pack();
}

void clonePrototypes(SymHeap &sh, TObjId obj, TObjId dup) {
    duplicateUnknownValues(sh, obj);

    ProtoCloner visitor;
    visitor.rootDst = obj;
    visitor.rootSrc = dup;
    buildIgnoreList(visitor.ignoreList, sh, obj);

    // traverse all sub-objects
    traverseSubObjs(sh, obj, visitor, /* leavesOnly */ true);

    // if there was "a pointer to self", it should remain "a pointer to self";
    // however "self" has been changed, so that a redirection is necessary
    redirectInboundEdges(sh, dup, obj, dup);
}

void slSegAbstractionStep(SymHeap &sh, TObjId *pObj, const SegBindingFields &bf)
{
    const TObjId obj = *pObj;
    const TObjId objPtrNext = subObjByChain(sh, obj, bf.next);
    const TValueId valNext = sh.valueOf(objPtrNext);
    CL_BREAK_IF(valNext <= 0);

    // read minimal length of 'obj' and set it temporarily to zero
    unsigned len = objMinLength(sh, obj);
    if (objIsSeg(sh, obj))
        segSetMinLength(sh, obj, /* SLS 0+ */ 0);

    // jump to the next object
    const TObjId objNext = subObjByInvChain(sh, sh.pointsTo(valNext), bf.head);
    len += objMinLength(sh, objNext);
    if (OK_SLS == sh.objKind(objNext))
        segSetMinLength(sh, objNext, /* SLS 0+ */ 0);
    else
        // abstract the _next_ object
        sh.objSetAbstract(objNext, OK_SLS, bf);

    // merge data
    CL_BREAK_IF(OK_SLS != sh.objKind(objNext));
    abstractNonMatchingValues(sh, obj, objNext);

    // replace all references to 'head'
    const TFieldIdxChain icHead = sh.objBinding(objNext).head;
    const TValueId headAt = sh.placedAt(subObjByChain(sh, obj, icHead));
    sh.valReplace(headAt, segHeadAddr(sh, objNext));

    // destroy self, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, headAt, slSegAbstractionStep);

    if (len)
        // declare resulting segment's minimal length
        segSetMinLength(sh, objNext, len);

    // move to the next object
    *pObj = objNext;
}

void dlSegCreate(SymHeap &sh, TObjId o1, TObjId o2, SegBindingFields bf) {
    // validate call of dlSegCreate()
    CL_BREAK_IF(OK_CONCRETE != sh.objKind(o1) || OK_CONCRETE != sh.objKind(o2));

    swapValues(bf.next, bf.peer);
    sh.objSetAbstract(o1, OK_DLS, bf);

    swapValues(bf.next, bf.peer);
    sh.objSetAbstract(o2, OK_DLS, bf);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, o1, o2, /* bidir */ true);

    // a just created DLS is said to be 2+
    dlSegSetMinLength(sh, o1, /* DLS 2+ */ 2);

    dlSegSyncPeerData(sh, o1);
}

void dlSegGobble(SymHeap &sh, TObjId dls, TObjId var, bool backward) {
    CL_BREAK_IF(OK_DLS != sh.objKind(dls) || OK_CONCRETE != sh.objKind(var));

    // handle DLS Neq predicates
    const unsigned len = dlSegMinLength(sh, dls) + /* OK_CONCRETE */ 1;
    dlSegSetMinLength(sh, dls, /* DLS 0+ */ 0);

    if (!backward)
        // jump to peer
        dls = dlSegPeer(sh, dls);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, var, dls);

    // store the pointer DLS -> VAR
    const SegBindingFields &bf = sh.objBinding(dls);
    const TObjId dlsNextPtr = subObjByChain(sh, dls, bf.next);
    const TObjId varNextPtr = subObjByChain(sh, var, bf.next);
    sh.objSetValue(dlsNextPtr, sh.valueOf(varNextPtr));

    // replace VAR by DLS
    const TObjId varHead = subObjByChain(sh, var, bf.head);
    const TValueId headAt = sh.placedAt(varHead);
    sh.valReplace(headAt, segHeadAddr(sh, dls));
    REQUIRE_GC_ACTIVITY(sh, headAt, dlSegGobble);

    // handle DLS Neq predicates
    dlSegSetMinLength(sh, dls, len);

    dlSegSyncPeerData(sh, dls);
}

void dlSegMerge(SymHeap &sh, TObjId seg1, TObjId seg2) {
    // handle DLS Neq predicates
    const unsigned len = dlSegMinLength(sh, seg1) + dlSegMinLength(sh, seg2);
    dlSegSetMinLength(sh, seg1, /* DLS 0+ */ 0);
    dlSegSetMinLength(sh, seg2, /* DLS 0+ */ 0);

    // check for a failure of segDiscover()
    CL_BREAK_IF(sh.objBinding(seg1) != sh.objBinding(seg2));

    const TObjId peer1 = dlSegPeer(sh, seg1);
#ifndef NDEBUG
    const TObjId nextPtr = nextPtrFromSeg(sh, peer1);
    const TValueId valNext = sh.valueOf(nextPtr);
    CL_BREAK_IF(valNext != segHeadAddr(sh, seg2));
#endif

    const TObjId peer2 = dlSegPeer(sh, seg2);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh,  seg1,  seg2, /* bidir */ true);

    // preserve backLink
    const TValueId valNext2 = sh.valueOf(nextPtrFromSeg(sh, seg1));
    sh.objSetValue(nextPtrFromSeg(sh, seg2), valNext2);

    // replace both parts point-wise
    const TValueId  segAt = segHeadAddr(sh,  seg1);
    const TValueId peerAt = segHeadAddr(sh, peer1);

    sh.valReplace( segAt, segHeadAddr(sh,  seg2));
    sh.valReplace(peerAt, segHeadAddr(sh, peer2));

    REQUIRE_GC_ACTIVITY(sh,  segAt, dlSegMerge);
    REQUIRE_GC_ACTIVITY(sh, peerAt, dlSegMerge);

    if (len)
        // handle DLS Neq predicates
        dlSegSetMinLength(sh, seg2, len);

    dlSegSyncPeerData(sh, seg2);
}

void dlSegAbstractionStep(SymHeap &sh, TObjId *pObj, const SegBindingFields &bf)
{
    // the first object is clear
    const TObjId o1 = *pObj;

    // we'll find the next one later on
    TObjId o2 = o1;

    EObjKind kind = sh.objKind(o1);
    switch (kind) {
        case OK_MAY_EXIST:
            // TODO

        case OK_SLS:
        case OK_HEAD:
        case OK_PART:
            // *** segDiscover() failure detected ***
            CL_TRAP;

        case OK_DLS:
            // jump to peer
            o2 = dlSegPeer(sh, o2);

            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, sh.objBinding(o2).head, sh.objBinding(o2).next);
            if (OK_CONCRETE == sh.objKind(o2)) {
                // DLS + VAR
                dlSegGobble(sh, o1, o2, /* backward */ false);
                return;
            }

            // DLS + DLS
            dlSegMerge(sh, o1, o2);
            break;

        case OK_CONCRETE:
            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, bf.head, bf.next);
            if (OK_CONCRETE == sh.objKind(o2)) {
                // VAR + VAR
                dlSegCreate(sh, o1, o2, bf);
                return;
            }

            // VAR + DLS
            dlSegGobble(sh, o2, o1, /* backward */ true);
            break;
    }

    // the current object has been just consumed, move to the next one
    *pObj = o2;

#ifndef NDEBUG
    // just check if the Neq predicates work well so far
    dlSegMinLength(sh, o2);
#endif
}

void segAbstractionStep(SymHeap                     &sh,
                        const SegBindingFields      &bf,
                        TObjId                      *pObj)
{
    if (bf.peer.empty()) {
        // SLS
        slSegAbstractionStep(sh, pObj, bf);
        return;
    }

    // DLS
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
    dlSegAbstractionStep(sh, pObj, bf);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
}

bool considerAbstraction(SymHeap                    &sh,
                         const SegBindingFields     &bf,
                         const TObjId               entry,
                         const unsigned             lenTotal)
{
    const bool isSls = bf.peer.empty();
    const AbstractionThreshold &at = (isSls)
        ? slsThreshold
        : dlsThreshold;

    // check whether the threshold is satisfied or not
    const unsigned threshold = at.sparePrefix + at.innerSegLen + at.spareSuffix;
    if (lenTotal < threshold) {
        CL_DEBUG("<-- length (" << lenTotal
                << ") of the longest segment is under the threshold ("
                << threshold << ")");
        return false;
    }

    CL_DEBUG("    --- length of the longest segment is " << lenTotal
            << ", prefix=" << at.sparePrefix
            << ", suffix=" << at.spareSuffix);

    // cursor
    TObjId obj = entry;

    // handle sparePrefix/spareSuffix
    int len = lenTotal - at.sparePrefix - at.spareSuffix;
    for (unsigned i = 0; i < at.sparePrefix; ++i)
        skipObj(sh, &obj, bf.head, bf.next);

    const char *name = (isSls)
        ? "SLS"
        : "DLS";
    CL_DEBUG("    AAA initiating " << name
             << " abstraction of length " << len);

    debugPlotInit(name);
    debugPlot(sh);

    for (int i = 0; i < len; ++i) {
        segAbstractionStep(sh, bf, &obj);
        debugPlot(sh);
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void segReplaceRefs(SymHeap &sh, TObjId seg, TValueId valNext) {
    const TObjId head = segHead(sh, seg);
    const TValueId segAt = sh.placedAt(head);
    sh.valReplace(segAt, valNext);

    const int offHead = subOffsetIn(sh, seg, head);
    CL_BREAK_IF(offHead < 0);

    const TObjId next = sh.pointsTo(valNext);
    switch (next) {
        case OBJ_DELETED:
        case OBJ_LOST:
            CL_DEBUG("WARNING: OBJ_DELETED/OBJ_LOST handling not optimal"
                     " in segReplaceRefs()");
            return;

        default:
            CL_BREAK_IF(0 < valNext && next < 0);
    }

    // TODO: check types in debug build
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, seg, /* toInsideOnly */ false);
    BOOST_FOREACH(const TObjId obj, refs) {
        const TObjId target = sh.pointsTo(sh.valueOf(obj));
        const struct cl_type *cltPtr = sh.objType(obj);
        CL_BREAK_IF(target <= 0 || !cltPtr);

        const TObjId root = objRoot(sh, target);
        const int off = subOffsetIn(sh, root, target) - offHead;
        const TValueId val = (0 < next)
            ? addrQueryByOffset(sh, next, off, cltPtr)
            : valNext;

        // redirect!
        sh.objSetValue(obj, val);
    }
}

bool dlSegReplaceByConcrete(SymHeap &sh, TObjId obj, TObjId peer) {
    debugPlotInit("dlSegReplaceByConcrete");
    debugPlot(sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));

    // first kill any related Neq predicates, we're going to concretize anyway
    dlSegSetMinLength(sh, obj, /* DLS 0+ */ 0);

    // take the value of 'next' pointer from peer
    const TFieldIdxChain icPeer = sh.objBinding(obj).peer;
    const TObjId peerPtr = subObjByChain(sh, obj, icPeer);
    const TValueId valNext = sh.valueOf(nextPtrFromSeg(sh, peer));
    sh.objSetValue(peerPtr, valNext);

    // redirect all references originally pointing to peer to the current object
    redirectInboundEdges(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ peer,
            /* redirectTo   */ obj);

    // destroy peer, including all prototypes
    const TValueId addrPeer = sh.placedAt(peer);
    REQUIRE_GC_ACTIVITY(sh, addrPeer, dlSegReplaceByConcrete);

    // concretize self
    sh.objSetConcrete(obj);

    // this can't fail (at least I hope so...)
    debugPlot(sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
    return true;
}

void spliceOutListSegmentCore(SymHeap &sh, TObjId seg, TObjId peer) {
    debugPlotInit("spliceOutListSegmentCore");
    debugPlot(sh);

    // read valNext now as we may overwrite it during unlink of peer
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);

    TValueId peerAt = VAL_INVALID;
    if (seg != peer) {
        peerAt = sh.placedAt(peer);

        // OK_DLS --> unlink peer
        const TObjId prevPtr = nextPtrFromSeg(sh, seg);
        const TValueId valPrev = sh.valueOf(prevPtr);
        segReplaceRefs(sh, peer, valPrev);
    }

    // unlink self
    segReplaceRefs(sh, seg, valNext);

    // destroy peer in case of DLS
    if (VAL_INVALID != peerAt && collectJunk(sh, peerAt))
        CL_DEBUG("spliceOutSegmentIfNeeded() drops a sub-heap (peerAt)");

    // destroy self, including all nested prototypes
    const TValueId segAt = sh.placedAt(seg);
    if (collectJunk(sh, segAt))
        CL_DEBUG("spliceOutSegmentIfNeeded() drops a sub-heap (segAt)");

    debugPlot(sh);
}

unsigned /* len */ spliceOutSegmentIfNeeded(SymHeap &sh, TObjId ao, TObjId peer,
                                            TSymHeapList &todo)
{
    const unsigned len = segMinLength(sh, ao);
    if (len) {
        debugPlotInit("spliceOutSegmentIfNeeded");
        debugPlot(sh);

        // drop any existing Neq predicates
        segSetMinLength(sh, ao, 0);

        debugPlot(sh);
        return len - 1;
    }

    // possibly empty LS
    SymHeap sh0(sh);
    spliceOutListSegmentCore(sh0, ao, peer);
    todo.push_back(sh0);
    return /* LS 0+ */ 0;
}

void abstractIfNeeded(SymHeap &sh) {
#if SE_DISABLE_SLS && SE_DISABLE_DLS
    return;
#endif
    SegBindingFields    bf;
    TObjId              entry;
    unsigned            len;

    while ((len = discoverBestAbstraction(sh, &bf, &entry))) {
        if (!considerAbstraction(sh, bf, entry, len))
            // the best abstraction given is unfortunately not good enough
            break;

        // some part of the symbolic heap has just been successfully abstracted,
        // let's look if there remains anything else suitable for abstraction
    }
}

void concretizeObj(SymHeap &sh, TValueId addr, TSymHeapList &todo) {
    TObjId obj = sh.pointsTo(addr);
    if (OK_HEAD == sh.objKind(obj))
        obj = objRoot(sh, obj);

    TObjId peer = obj;

    // branch by SLS/DLS
    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
        case OK_HEAD:
        case OK_PART:
            // invalid call of concretizeObj()
            CL_TRAP;

        case OK_MAY_EXIST:
        case OK_SLS:
            break;

        case OK_DLS:
            // jump to peer
            CL_BREAK_IF(!dlSegCheckConsistency(sh));
            peer = dlSegPeer(sh, obj);
            break;
    }

    // handle the possibly empty variant (if exists)
    const unsigned lenRemains = spliceOutSegmentIfNeeded(sh, obj, peer, todo);

    debugPlotInit("concretizeObj");
    debugPlot(sh);

    if (OK_MAY_EXIST == kind) {
        // this kind is much easier than regular list segments
        sh.objSetConcrete(obj);
        debugPlot(sh);
        return;
    }

    // duplicate self as abstract object
    const TObjId aoDup = sh.objDup(obj);
    const TValueId aoDupHeadAddr = segHeadAddr(sh, aoDup);
    if (OK_DLS == kind) {
        // DLS relink
        const TFieldIdxChain icPeer = sh.objBinding(peer).peer;
        const TObjId peerField = subObjByChain(sh, peer, icPeer);
        sh.objSetValue(peerField, aoDupHeadAddr);
    }

    // duplicate all unknown values, to keep the prover working
    clonePrototypes(sh, obj, aoDup);

    // concretize self and recover the list
    const TObjId ptrNext = subObjByChain(sh, obj, (OK_SLS == kind)
            ? sh.objBinding(obj).next
            : sh.objBinding(obj).peer);
    sh.objSetConcrete(obj);
    sh.objSetValue(ptrNext, aoDupHeadAddr);

    if (OK_DLS == kind) {
        // update DLS back-link
        const SegBindingFields &bf = sh.objBinding(aoDup);
        const TObjId backLink = subObjByChain(sh, aoDup, bf.next);
        const TValueId headAddr = sh.placedAt(subObjByChain(sh, obj, bf.head));
        sh.objSetValue(backLink, headAddr);
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
    }

    segSetMinLength(sh, aoDup, lenRemains);

    debugPlot(sh);
}

bool spliceOutListSegment(SymHeap &sh, TValueId atAddr, TValueId pointingTo)
{
    const TObjId obj = objRootByVal(sh, atAddr);
    const EObjKind kind = sh.objKind(obj);
    const TObjId peer = (OK_DLS == kind)
        ? dlSegPeer(sh, obj)
        : obj;

    if (OK_DLS == sh.objKind(obj)) {
        CL_BREAK_IF(!dlSegCheckConsistency(sh));

        const TObjId peer = dlSegPeer(sh, obj);
        if (sh.placedAt(peer) == pointingTo)
            // assume identity over the two parts of DLS
            return dlSegReplaceByConcrete(sh, obj, peer);
    }

    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);
    if (valNext != pointingTo)
        return false;

    spliceOutListSegmentCore(sh, obj, peer);
    return true;
}
