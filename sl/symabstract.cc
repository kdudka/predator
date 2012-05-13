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
#include "symtrace.hh"
#include "util.hh"
#include "worklist.hh"

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
    TObjSet ignoreList;

    bool operator()(const ObjHandle &obj) const {
        if (hasKey(ignoreList, obj))
            return /* continue */ true;

        const TValId valOld = obj.value();
        if (valOld <= 0)
            return /* continue */ true;

        SymHeapCore *sh = obj.sh();
        const EValueTarget code = sh->valTarget(valOld);
        if (isPossibleToDeref(code) || (VT_CUSTOM == code))
            return /* continue */ true;

        // duplicate unknown value
        const TValId valNew = sh->valClone(valOld);
        obj.setValue(valNew);

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
        const TValId            rootSrc,
        const bool              uplink)
{
    const bool isRootDls = (OK_DLS == sh.valTargetKind(rootDst));
    CL_BREAK_IF(isRootDls && (OK_DLS != sh.valTargetKind(rootSrc)));

    TValId rootDstPeer = VAL_INVALID;
    TValId rootSrcPeer = VAL_INVALID;
    if (isRootDls) {
        rootDstPeer = dlSegPeer(sh, rootDst);
        rootSrcPeer = dlSegPeer(sh, rootSrc);
        CL_BREAK_IF(uplink && (rootDstPeer != rootSrcPeer));
    }

    redirectRefs(sh, rootDst, proto, clone);
    redirectRefs(sh, proto, rootDst, rootSrc);

    if (isRootDls) {
        if (uplink)
            redirectRefs(sh, clone, rootSrcPeer, rootDst);
        else
            redirectRefs(sh, rootDstPeer, proto, clone);
    }

    if (OK_DLS == sh.valTargetKind(proto)) {
        const TValId protoPeer = dlSegPeer(sh, proto);
        const TValId clonePeer = dlSegPeer(sh, clone);
        redirectRefs(sh, rootDst, protoPeer, clonePeer);
        redirectRefs(sh, protoPeer, rootDst, rootSrc);
        if (isRootDls && uplink)
            redirectRefs(sh, clonePeer, rootSrcPeer, rootDst);
    }
}

TValId protoClone(SymHeap &sh, const TValId proto) {
    const TValId clone = segClone(sh, proto);
    objDecrementProtoLevel(sh, clone);

    const EValueTarget code = sh.valTarget(proto);
    if (!isAbstract(code))
        // clone all unknown values in order to keep prover working
        duplicateUnknownValues(sh, clone);

    return clone;
}

struct ProtoFinder {
    std::set<TValId> protos;

    bool operator()(const ObjHandle &sub) {
        const TValId val = sub.value();
        if (val <= 0)
            return /* continue */ true;

        SymHeapCore *sh = sub.sh();
        const TValId root = sh->valRoot(val);
        if (sh->valTargetProtoLevel(root))
            protos.insert(root);

        return /* continue */ true;
    }
};

void clonePrototypes(
        SymHeap                &sh,
        const TValId            rootDst,
        const TValId            rootSrc,
        const TValList         &protoList)
{
    // allocate some space for clone IDs
    const unsigned cnt = protoList.size();
    TValList cloneList(cnt);

    // clone the prototypes and reconnect them to the new root
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = protoList[i];
        const TValId clone = protoClone(sh, protoList[i]);
        detachClonedPrototype(sh, proto, clone, rootDst, rootSrc,
                /* uplink */ true);

        cloneList[i] = clone;
    }

    // FIXME: works, but likely to kill the CPU
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = protoList[i];
        const TValId clone = cloneList[i];

        for (unsigned j = 0; j < cnt; ++j) {
            if (i == j)
                continue;

            const TValId otherProto = protoList[j];
            const TValId otherClone = cloneList[j];
            detachClonedPrototype(sh, proto, clone, otherClone, otherProto,
                    /* uplink */ false);
        }
    }
}

// visitor
class ProtoCollector {
    private:
        TValList               &protoList_;
        TObjSet                 ignoreList_;
        WorkList<TValId>        wl_;

    public:
        ProtoCollector(TValList &dst):
            protoList_(dst)
        {
        }

        TObjSet& ignoreList() {
            return ignoreList_;
        }

        bool operator()(const ObjHandle &obj);
};

bool ProtoCollector::operator()(const ObjHandle &obj) {
    if (hasKey(ignoreList_, obj))
        return /* continue */ true;

    const TValId val = obj.value();
    if (val <= 0)
        return /* continue */ true;

    SymHeap &sh = *static_cast<SymHeap *>(obj.sh());
    if (!isPossibleToDeref(sh.valTarget(val)))
        return /* continue */ true;

    // check if we point to prototype, or shared data
    if (!sh.valTargetProtoLevel(val))
        return /* continue */ true;

    TValId proto = sh.valRoot(val);
    wl_.schedule(proto);
    while (wl_.next(proto)) {
        ProtoFinder visitor;
        traverseLivePtrs(sh, proto, visitor);
        BOOST_FOREACH(const TValId protoAt, visitor.protos)
            wl_.schedule(protoAt);

            if (isDlSegPeer(sh, proto))
                // it is sufficient to process just one part of a DLS
                continue;

        protoList_.push_back(proto);
    }

    return /* continue */ true;
}

bool collectPrototypesOf(TValList &dst, SymHeap &sh, const TValId root)
{
    ProtoCollector collector(dst);
    buildIgnoreList(collector.ignoreList(), sh, root);
    return traverseLivePtrs(sh, root, collector);
}

void decrementProtoLevel(SymHeap &sh, const TValId at) {
    TValList protoList;
    collectPrototypesOf(protoList, sh, at);
    BOOST_FOREACH(const TValId proto, protoList)
        objDecrementProtoLevel(sh, proto);
}

struct ValueSynchronizer {
    SymHeap            &sh;
    TObjSet             ignoreList;

    ValueSynchronizer(SymHeap &sh_): sh(sh_) { }

    bool operator()(ObjHandle item[2]) const {
        const ObjHandle &src = item[0];
        const ObjHandle &dst = item[1];
        if (hasKey(ignoreList, src))
            return /* continue */ true;

        // store value of 'src' into 'dst'
        const TValId valSrc = src.value();
        const TValId valDst = dst.value();
        dst.setValue(valSrc);

        // if the last reference is gone, we have a problem
        const TValId rootDst = sh.valRoot(valDst);
        if (collectJunk(sh, rootDst))
            CL_DEBUG("    ValueSynchronizer drops a sub-heap (dlSegPeerData)");

        return /* continue */ true;
    }
};

void dlSegSyncPeerData(SymHeap &sh, const TValId dls) {
    const TValId peer = dlSegPeer(sh, dls);
    ValueSynchronizer visitor(sh);
    buildIgnoreList(visitor.ignoreList, sh, dls);

    // if there was "a pointer to self", it should remain "a pointer to self";
    ObjList refs;
    sh.pointedBy(refs, dls);
    BOOST_FOREACH(const ObjHandle &obj, refs) {
        visitor.ignoreList.insert(obj);
    }

    const TValId roots[] = { dls, peer };
    traverseLiveObjs<2>(sh, roots, visitor);
}

// FIXME: this works only for nullified blocks anyway
void killUniBlocksUnderBindingPtrs(SymHeap &sh, const TValId seg) {
    // go through next/prev pointers
    TObjSet blackList;
    buildIgnoreList(blackList, sh, seg);
    BOOST_FOREACH(const ObjHandle &obj, blackList) {
        if (VAL_NULL != obj.value())
            continue;

        // if there is a nullified block under next/prev pointer, kill it now
        obj.setValue(VAL_TRUE);
        obj.setValue(VAL_NULL);
    }
}

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(
        SymHeap                     &sh,
        const TValId                srcAt,
        const TValId                dstAt,
        const bool                  bidir = false)
{
    killUniBlocksUnderBindingPtrs(sh, dstAt);
    if (bidir)
        killUniBlocksUnderBindingPtrs(sh, srcAt);

    if (!joinData(sh, dstAt, srcAt, bidir))
        CL_BREAK_IF("joinData() failed, failure of segDiscover()?");

    if (bidir)
        // already done as side-effect of joinData()
        return;

    if (OK_DLS == sh.valTargetKind(dstAt))
        dlSegSyncPeerData(sh, dstAt);
}

// FIXME: the semantics of this function is quite contra-intuitive
TValId segDeepCopy(SymHeap &sh, TValId seg) {
    // collect the list of prototypes
    TValList protoList;
    collectPrototypesOf(protoList, sh, seg);

    // clone the root itself
    const TValId dup = objClone(sh, seg);

    // clone all unknown values in order to keep prover working
    duplicateUnknownValues(sh, dup);

    // clone all prototypes originally owned by seg
    clonePrototypes(sh, seg, dup, protoList);

    return dup;
}

void enlargeMayExist(SymHeap &sh, const TValId at) {
    const EObjKind kind = sh.valTargetKind(at);
    switch (kind) {
        case OK_SEE_THROUGH:
            decrementProtoLevel(sh, at);
            sh.valTargetSetConcrete(at);
            // fall through!

        case OK_CONCRETE:
        case OK_SLS:
            return;

        default:
            CL_BREAK_IF("invalid call of enlargeMayExist()");
    }
}

void slSegAbstractionStep(
        SymHeap                     &sh,
        const TValId                at,
        const TValId                nextAt,
        const BindingOff            &off)
{
    // compute the resulting minimal length
    const TMinLen len = objMinLength(sh, at) + objMinLength(sh, nextAt);

    enlargeMayExist(sh, at);
    enlargeMayExist(sh, nextAt);

    if (OK_SLS == sh.valTargetKind(at))
        decrementProtoLevel(sh, at);

    if (OK_SLS == sh.valTargetKind(nextAt))
        decrementProtoLevel(sh, nextAt);
    else
        // abstract the _next_ object
        sh.valTargetSetAbstract(nextAt, OK_SLS, off);

    // merge data
    abstractNonMatchingValues(sh, at, nextAt);

    // replace all references to 'head'
    const TOffset offHead = sh.segBinding(nextAt).head;
    const TValId headAt = sh.valByOffset(at, offHead);
    sh.valReplace(headAt, segHeadAt(sh, nextAt));

    // destroy self, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, at, slSegAbstractionStep);

    if (len)
        // declare resulting segment's minimal length
        sh.segSetMinLength(nextAt, len);
}

void dlSegCreate(SymHeap &sh, TValId a1, TValId a2, BindingOff off) {
    // compute resulting segment's length
    const TMinLen len = objMinLength(sh, a1) + objMinLength(sh, a2);

    // OK_SEE_THROUGH -> OK_CONCRETE if necessary
    enlargeMayExist(sh, a1);
    enlargeMayExist(sh, a2);

    swapValues(off.next, off.prev);
    sh.valTargetSetAbstract(a1, OK_DLS, off);

    swapValues(off.next, off.prev);
    sh.valTargetSetAbstract(a2, OK_DLS, off);

    // merge data
    abstractNonMatchingValues(sh, a1, a2, /* bidir */ true);

    // just created DLS is said to be 2+ as long as no OK_SEE_THROUGH are involved
    sh.segSetMinLength(a1, len);
}

void dlSegGobble(SymHeap &sh, TValId dls, TValId var, bool backward) {
    CL_BREAK_IF(OK_DLS != sh.valTargetKind(dls));

    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(dls) + objMinLength(sh, var);

    // we allow to gobble OK_SEE_THROUGH objects (if compatible)
    enlargeMayExist(sh, var);

    if (!backward)
        // jump to peer
        dls = dlSegPeer(sh, dls);

    // merge data
    decrementProtoLevel(sh, dls);
    abstractNonMatchingValues(sh, var, dls);

    // store the pointer DLS -> VAR
    const BindingOff &off = sh.segBinding(dls);
    const PtrHandle nextPtr(sh, sh.valByOffset(dls, off.next));
    const TValId valNext = valOfPtrAt(sh, var, off.next);
    nextPtr.setValue(valNext);

    // replace VAR by DLS
    const TValId headAt = sh.valByOffset(var, off.head);
    sh.valReplace(headAt, segHeadAt(sh, dls));
    REQUIRE_GC_ACTIVITY(sh, var, dlSegGobble);

    // handle DLS Neq predicates
    sh.segSetMinLength(dls, len);

    dlSegSyncPeerData(sh, dls);
}

void dlSegMerge(SymHeap &sh, TValId seg1, TValId seg2) {
    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(seg1) + sh.segMinLength(seg2);

    // dig peers
    const TValId peer1 = dlSegPeer(sh, seg1);
    const TValId peer2 = dlSegPeer(sh, seg2);

    // check for a failure of segDiscover()
    CL_BREAK_IF(sh.segBinding(seg1) != sh.segBinding(seg2));
    CL_BREAK_IF(nextValFromSeg(sh, peer1) != segHeadAt(sh, seg2));

    // merge data
    decrementProtoLevel(sh, seg1);
    decrementProtoLevel(sh, seg2);
    abstractNonMatchingValues(sh, seg1, seg2, /* bidir */ true);

    // preserve backLink
    const TValId valNext1 = nextValFromSeg(sh, seg1);
    const PtrHandle ptrNext2 = nextPtrFromSeg(sh, seg2);
    ptrNext2.setValue(valNext1);

    // replace both parts point-wise
    const TValId headAt = segHeadAt(sh,  seg1);
    const TValId peerAt = segHeadAt(sh, peer1);

    sh.valReplace(headAt, segHeadAt(sh,  seg2));
    sh.valReplace(peerAt, segHeadAt(sh, peer2));

    // destroy headAt and peerAt, including all prototypes -- either at once, or
    // one by one (depending on the shape of subgraph)
    REQUIRE_GC_ACTIVITY(sh, seg1, dlSegMerge);
    if (!collectJunk(sh, peer1) && isPossibleToDeref(sh.valTarget(peer1))) {
        CL_ERROR("dlSegMerge() failed to collect garbage"
                 ", peer1 still referenced");
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
    EObjKind kind;
    unsigned thr;
    const char *name;

    if (isDlsBinding(off)) {
        kind = OK_DLS;
        thr  = dlsThreshold;
        name = "DLS";
    }
    else {
        kind = OK_SLS;
        thr  = slsThreshold;
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
        CL_BREAK_IF(!protoCheckConsistency(sh));

        if (!segAbstractionStep(sh, off, &cursor)) {
            CL_DEBUG("<-- validity of next " << (lenTotal - i - 1)
                    << " abstraction step(s) broken, forcing re-discovery...");

            if (i)
                return true;

            CL_BREAK_IF("segAbstractionStep() failed, nothing has been done");
            return false;
        }

        Trace::Node *trAbs = new Trace::AbstractionNode(sh.traceNode(), kind);
        sh.traceUpdate(trAbs);

        LDP_PLOT(symabstract, sh);

        CL_BREAK_IF(!protoCheckConsistency(sh));
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void dlSegReplaceByConcrete(SymHeap &sh, TValId seg, TValId peer) {
    LDP_INIT(symabstract, "dlSegReplaceByConcrete");
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));

    // take the value of 'next' pointer from peer
    const PtrHandle peerPtr = prevPtrFromSeg(sh, seg);
    const TValId valNext = nextValFromSeg(sh, peer);
    peerPtr.setValue(valNext);

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
    CL_BREAK_IF(!protoCheckConsistency(sh));
}

void spliceOutListSegment(
        SymHeap                &sh,
        const TValId            seg,
        const TValId            peer,
        const TValId            valNext,
        TValList               *leakList)
{
    LDP_INIT(symabstract, "spliceOutListSegment");
    LDP_PLOT(symabstract, sh);

    CL_BREAK_IF(objMinLength(sh, seg));

    TOffset offHead = 0;
    if (OK_OBJ_OR_NULL != sh.valTargetKind(seg))
        offHead = sh.segBinding(seg).head;

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

    collectSharedJunk(sh, seg, leakList);

    // destroy peer in case of DLS
    if (peer != seg && collectJunk(sh, peer))
        CL_DEBUG("spliceOutListSegment() drops a sub-heap (peer)");

    // destroy self, including all nested prototypes
    if (collectJunk(sh, seg))
        CL_DEBUG("spliceOutListSegment() drops a sub-heap (seg)");

    LDP_PLOT(symabstract, sh);
}

void spliceOutSegmentIfNeeded(
        TMinLen                *pLen,
        SymHeap                &sh,
        const TValId            seg,
        const TValId            peer,
        TSymHeapList           &todo,
        TValList               *leakList)
{
    if (!*pLen) {
        // possibly empty LS
        SymHeap sh0(sh);
        const TValId valNext = nextValFromSeg(sh0, peer);
        spliceOutListSegment(sh0, seg, peer, valNext, leakList);

        const EObjKind kind = sh.valTargetKind(seg);
        Trace::Node *tr = new Trace::SpliceOutNode(sh.traceNode(), kind, true);

        todo.push_back(sh0);
        todo.back().traceUpdate(tr);
    }
    else
        // we are going to detach one node
        --(*pLen);

    LDP_INIT(symabstract, "concretizeObj");
    LDP_PLOT(symabstract, sh);
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

void concretizeObj(
        SymHeap                     &sh,
        const TValId                 addr,
        TSymHeapList                &todo,
        TValList                    *leakList)
{
    CL_BREAK_IF(!protoCheckConsistency(sh));

    const TValId seg = sh.valRoot(addr);
    const TValId peer = segPeer(sh, seg);

    // handle the possibly empty variant (if exists)
    TMinLen len = sh.segMinLength(seg);
    spliceOutSegmentIfNeeded(&len, sh, seg, peer, todo, leakList);

    const EObjKind kind = sh.valTargetKind(seg);
    sh.traceUpdate(new Trace::ConcretizationNode(sh.traceNode(), kind));

    switch (kind) {
        case OK_OBJ_OR_NULL:
        case OK_SEE_THROUGH:
            // these kinds are much easier than regular list segments
            sh.valTargetSetConcrete(seg);
            decrementProtoLevel(sh, seg);
            LDP_PLOT(symabstract, sh);
            CL_BREAK_IF(!protoCheckConsistency(sh));
            return;

        default:
            break;
    }

    // duplicate self as abstract object (including all prototypes)
    const TValId dup = segDeepCopy(sh, seg);

    // resolve the relative placement of the 'next' pointer
    const BindingOff off = sh.segBinding(seg);
    const TOffset offNext = (OK_SLS == kind)
        ? off.next
        : off.prev;

    // concretize self
    sh.valTargetSetConcrete(seg);

    // update 'next' pointer
    const PtrHandle nextPtr(sh, sh.valByOffset(seg, offNext));
    const TValId dupHead = segHeadAt(sh, dup);
    nextPtr.setValue(dupHead);

    if (OK_DLS == kind) {
        // update 'prev' pointer going from peer to the cloned object
        const PtrHandle prev1 = prevPtrFromSeg(sh, peer);
        prev1.setValue(dupHead);

        // update 'prev' pointer going from the cloned object to the conrete one
        const PtrHandle prev2(sh, sh.valByOffset(dup, off.next));
        const TValId headAddr = sh.valByOffset(seg, off.head);
        prev2.setValue(headAddr);

        CL_BREAK_IF(!dlSegCheckConsistency(sh));
    }

    sh.segSetMinLength(dup, len);

    LDP_PLOT(symabstract, sh);

    CL_BREAK_IF(!protoCheckConsistency(sh));
}
