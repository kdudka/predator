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

#include "prototype.hh"
#include "symcmp.hh"
#include "symdebug.hh"
#include "symjoin.hh"
#include "symdiscover.hh"
#include "symgc.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "util.hh"

#include <iomanip>
#include <set>
#include <sstream>

#include <boost/foreach.hpp>

LOCAL_DEBUG_PLOTTER(symabstract, DEBUG_SYMABSTRACT)

void debugSymAbstract(const bool enable)
{
    if (enable == __ldp_enabled_symabstract)
        return;

    CL_DEBUG("symabstract: debugSymAbstract(" << enable << ") takes effect");
    __ldp_enabled_symabstract = enable;
}

#define REQUIRE_GC_ACTIVITY(sh, obj, fnc) do {                                 \
    if (collectJunk(sh, obj))                                                  \
        break;                                                                 \
    CL_ERROR(#fnc "() failed to collect garbage, " #obj " still referenced");  \
    CL_BREAK_IF("REQUIRE_GC_ACTIVITY has not been successful");                \
} while (0)

// visitor
struct UnknownValuesDuplicator {
    TFldSet ignoreList;

    bool operator()(const FldHandle &fld) const {
        if (hasKey(ignoreList, fld))
            return /* continue */ true;

        const TValId valOld = fld.value();
        if (valOld <= 0)
            return /* continue */ true;

        SymHeapCore *sh = fld.sh();
        const EValueTarget code = sh->valTarget(valOld);
        if (isPossibleToDeref(*sh, valOld) || (VT_CUSTOM == code))
            return /* continue */ true;

        // duplicate unknown value
        const TValId valNew = sh->valClone(valOld);
        fld.setValue(valNew);

        return /* continue */ true;
    }
};

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TValId at)
{
    UnknownValuesDuplicator visitor;
    buildIgnoreList(visitor.ignoreList, sh, at);

    // traverse all sub-objects
    traverseLiveFields(sh, sh.objByAddr(at), visitor);
}

void detachClonedPrototype(
        SymHeap                 &sh,
        const TValId            proto,
        const TValId            clone,
        const TValId            rootDst,
        const TValId            rootSrc,
        const bool              uplink)
{
    const bool isRootDls = (OK_DLS == sh.objKind(sh.objByAddr(rootDst)));
    CL_BREAK_IF(isRootDls && (OK_DLS != sh.objKind(sh.objByAddr(rootSrc))));

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

    if (OK_DLS == sh.objKind(sh.objByAddr(proto))) {
        const TValId protoPeer = dlSegPeer(sh, proto);
        const TValId clonePeer = dlSegPeer(sh, clone);
        redirectRefs(sh, rootDst, protoPeer, clonePeer);
        redirectRefs(sh, protoPeer, rootDst, rootSrc);
        if (isRootDls && uplink)
            redirectRefs(sh, clonePeer, rootSrcPeer, rootDst);
    }
}

TValId protoClone(SymHeap &sh, const TValId proto)
{
    const TValId clone = segClone(sh, proto);
    objDecrementProtoLevel(sh, sh.objByAddr(clone));

    if (!isAbstractValue(sh, proto))
        // clone all unknown values in order to keep prover working
        duplicateUnknownValues(sh, clone);

    return clone;
}

void clonePrototypes(
        SymHeap                &sh,
        const TValId            rootDst,
        const TValId            rootSrc,
        const TObjList         &protoList)
{
    // allocate some space for clone IDs
    const unsigned cnt = protoList.size();
    TValList cloneList(cnt);

    // clone the prototypes and reconnect them to the new root
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = sh.legacyAddrOfAny_XXX(protoList[i]);
        const TValId clone = protoClone(sh,
                sh.legacyAddrOfAny_XXX(protoList[i]));

        detachClonedPrototype(sh, proto, clone, rootDst, rootSrc,
                /* uplink */ true);

        cloneList[i] = clone;
    }

    // FIXME: works, but likely to kill the CPU
    for (unsigned i = 0; i < cnt; ++i) {
        const TValId proto = sh.legacyAddrOfAny_XXX(protoList[i]);
        const TValId clone = cloneList[i];

        for (unsigned j = 0; j < cnt; ++j) {
            if (i == j)
                continue;

            const TValId otherProto = sh.legacyAddrOfAny_XXX(protoList[j]);
            const TValId otherClone = cloneList[j];
            detachClonedPrototype(sh, proto, clone, otherClone, otherProto,
                    /* uplink */ false);
        }
    }
}

struct ValueSynchronizer {
    SymHeap            &sh;
    TFldSet             ignoreList;

    ValueSynchronizer(SymHeap &sh_): sh(sh_) { }

    bool operator()(FldHandle item[2]) const {
        const FldHandle &src = item[0];
        const FldHandle &dst = item[1];
        if (hasKey(ignoreList, src))
            return /* continue */ true;

        // store value of 'src' into 'dst'
        const TValId valSrc = src.value();
        const TValId valDst = dst.value();
        dst.setValue(valSrc);

        // if the last reference is gone, we have a problem
        const TObjId objDst = sh.objByAddr(valDst);
        if (collectJunk(sh, objDst))
            CL_DEBUG("    ValueSynchronizer drops a sub-heap (dlSegPeerData)");

        return /* continue */ true;
    }
};

void dlSegSyncPeerData(SymHeap &sh, const TValId dlsAt)
{
    const TObjId dls = sh.objByAddr(dlsAt);

    ValueSynchronizer visitor(sh);
    buildIgnoreList(visitor.ignoreList, sh, dls);

    // if there was "a pointer to self", it should remain "a pointer to self";
    FldList refs;
    sh.pointedBy(refs, dls);
    BOOST_FOREACH(const FldHandle &fld, refs)
        visitor.ignoreList.insert(fld);

    const TObjId peer = dlSegPeer(sh, dls);
    const TObjId objs[] = { dls, peer };
    traverseLiveFields<2>(sh, objs, visitor);
}

// FIXME: the semantics of this function is quite contra-intuitive
TValId segDeepCopy(SymHeap &sh, TValId seg)
{
    // collect the list of prototypes
    TObjList protoList;
    collectPrototypesOf(protoList, sh, sh.objByAddr(seg), /* skipPeers */ true);

    // clone the root itself
    const TValId dup = objClone(sh, seg);

    // clone all unknown values in order to keep prover working
    duplicateUnknownValues(sh, dup);

    // clone all prototypes originally owned by seg
    clonePrototypes(sh, seg, dup, protoList);

    return dup;
}

void enlargeMayExist(SymHeap &sh, const TValId at)
{
    const TObjId obj = sh.objByAddr(at);
    const EObjKind kind = sh.objKind(obj);
    if (!isMayExistObj(kind))
        return;

    decrementProtoLevel(sh, obj);
    sh.objSetConcrete(obj);
}

void slSegAbstractionStep(
        SymHeap                     &sh,
        const TValId                at,
        const TValId                nextAt,
        const BindingOff            &off)
{
    const TObjId obj = sh.objByAddr(at);

    // compute the resulting minimal length
    const TMinLen len = objMinLength(sh, at) + objMinLength(sh, nextAt);

    enlargeMayExist(sh, at);
    enlargeMayExist(sh, nextAt);

    // merge data
    joinData(sh, off, nextAt, at, /* bidir */ false);

    const TObjId next = sh.objByAddr(nextAt);
    if (OK_SLS != sh.objKind(next))
        // abstract the _next_ object
        sh.objSetAbstract(next, OK_SLS, off);

    // replace all references to 'head'
    const TOffset offHead = sh.segBinding(next).head;
    const TValId headAt = sh.valByOffset(at, offHead);
    sh.valReplace(headAt, segHeadAt(sh, nextAt));

    // destroy self, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, obj, slSegAbstractionStep);

    if (len)
        // declare resulting segment's minimal length
        sh.segSetMinLength(next, len);
}

void dlSegCreate(SymHeap &sh, TValId a1, TValId a2, BindingOff off)
{
    // compute resulting segment's length
    const TMinLen len = objMinLength(sh, a1) + objMinLength(sh, a2);

    // OK_SEE_THROUGH -> OK_CONCRETE if necessary
    enlargeMayExist(sh, a1);
    enlargeMayExist(sh, a2);

    // merge data
    joinData(sh, off, a2, a1, /* bidir */ true);

    const TObjId seg1 = sh.objByAddr(a1);
    const TObjId seg2 = sh.objByAddr(a2);

    swapValues(off.next, off.prev);
    sh.objSetAbstract(seg1, OK_DLS, off);

    swapValues(off.next, off.prev);
    sh.objSetAbstract(seg2, OK_DLS, off);

    // just created DLS is said to be 2+ as long as no OK_SEE_THROUGH are involved
    sh.segSetMinLength(seg1, len);
    sh.segSetMinLength(seg2, len);
}

void dlSegGobble(SymHeap &sh, TValId dlsAt, TValId regAt, bool backward)
{
    TObjId dls = sh.objByAddr(dlsAt);
    CL_BREAK_IF(OK_DLS != sh.objKind(dls));

    const TObjId reg = sh.objByAddr(regAt);

    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(dls) + objMinLength(sh, regAt);

    // we allow to gobble OK_SEE_THROUGH objects (if compatible)
    enlargeMayExist(sh, regAt);

    if (!backward) {
        // jump to peer
        dlsAt = dlSegPeer(sh, dlsAt);
        dls = sh.objByAddr(dlsAt);
    }

    // merge data
    const BindingOff &off = sh.segBinding(dls);
    joinData(sh, off, dlsAt, regAt, /* bidir */ false);
    dlSegSyncPeerData(sh, dlsAt);

    // store the pointer DLS -> VAR
    const PtrHandle nextPtr(sh, sh.valByOffset(dlsAt, off.next));
    const TValId valNext = valOfPtrAt(sh, regAt, off.next);
    nextPtr.setValue(valNext);

    // replace VAR by DLS
    const TValId headAt = sh.valByOffset(regAt, off.head);
    sh.valReplace(headAt, segHeadAt(sh, dlsAt));
    REQUIRE_GC_ACTIVITY(sh, reg, dlSegGobble);

    // handle DLS Neq predicates
    sh.segSetMinLength(dls, len);
    sh.segSetMinLength(sh.objByAddr(segPeer(sh, dlsAt)), len);

    dlSegSyncPeerData(sh, dlsAt);
}

void dlSegMerge(SymHeap &sh, TValId seg1At, TValId seg2At)
{
    const TObjId seg1 = sh.objByAddr(seg1At);
    const TObjId seg2 = sh.objByAddr(seg2At);

    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(seg1) + sh.segMinLength(seg2);

    // TODO: drop this!
    const TObjId peer1   = dlSegPeer(sh, seg1);
    const TValId peer1At = dlSegPeer(sh, seg1At);
    const TValId peer2At = dlSegPeer(sh, seg2At);

    // check for a failure of segDiscover()
    CL_BREAK_IF(nextValFromSeg(sh, peer1At) != segHeadAt(sh, seg2At));

    // merge data
    const BindingOff &bf2 = sh.segBinding(seg2);
    joinData(sh, bf2, seg2At, seg1At, /* bidir */ true);

    // preserve backLink
    const TValId valNext1 = nextValFromSeg(sh, seg1At);
    const PtrHandle ptrNext2 = nextPtrFromSeg(sh, seg2At);
    ptrNext2.setValue(valNext1);

    // replace both parts point-wise
    const TValId headAt = segHeadAt(sh,  seg1At);
    const TValId peerAt = segHeadAt(sh, peer1At);

    sh.valReplace(headAt, segHeadAt(sh,  seg2At));
    sh.valReplace(peerAt, segHeadAt(sh, peer2At));

    // destroy headAt and peerAt, including all prototypes -- either at once, or
    // one by one (depending on the shape of subgraph)
    REQUIRE_GC_ACTIVITY(sh, seg1, dlSegMerge);
    if (!collectJunk(sh, peer1) && sh.isValid(peer1)) {
        CL_ERROR("dlSegMerge() failed to collect garbage"
                 ", peer1 still referenced");
        CL_BREAK_IF("collectJunk() has not been successful");
    }

    if (len) {
        // assign the resulting minimal length
        sh.segSetMinLength(seg2, len);
        sh.segSetMinLength(sh.objByAddr(segPeer(sh, seg2At)), len);
    }

    dlSegSyncPeerData(sh, seg2At);
}

bool /* jump next */ dlSegAbstractionStep(
        SymHeap                     &sh,
        const TValId                at,
        const TValId                next,
        const BindingOff            &off)
{
    const EObjKind kind = sh.objKind(sh.objByAddr(at));
    const EObjKind kindNext = sh.objKind(sh.objByAddr(next));
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
    if (OK_DLS == sh.objKind(sh.objByAddr(at)))
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

bool applyAbstraction(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TObjId                entry,
        const unsigned              len)
{
    EObjKind kind;
    const char *name;

    if (isDlsBinding(off)) {
        kind = OK_DLS;
        name = "DLS";
    }
    else {
        kind = OK_SLS;
        name = "SLS";
    }

    CL_DEBUG("    AAA initiating " << name << " abstraction of length " << len);

    // cursor
    TValId cursor = sh.legacyAddrOfAny_XXX(entry);

    LDP_INIT(symabstract, name);
    LDP_PLOT(symabstract, sh);

    for (unsigned i = 0; i < len; ++i) {
        CL_BREAK_IF(!protoCheckConsistency(sh));

        if (!segAbstractionStep(sh, off, &cursor)) {
            CL_DEBUG("<-- validity of next " << (len - i - 1)
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

void dlSegReplaceByConcrete(SymHeap &sh, TValId seg, TValId peerAt)
{
    LDP_INIT(symabstract, "dlSegReplaceByConcrete");
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));

    const TObjId peer = sh.objByAddr(peerAt);

    // take the value of 'next' pointer from peer
    const PtrHandle peerPtr = prevPtrFromSeg(sh, seg);
    const TValId valNext = nextValFromSeg(sh, peerAt);
    peerPtr.setValue(valNext);

    // redirect all references originally pointing to peer to the current object
    redirectRefs(sh,
            /* pointingFrom */ VAL_INVALID,
            /* pointingTo   */ peerAt,
            /* redirectTo   */ seg);

    // destroy peer, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, peer, dlSegReplaceByConcrete);

    // concretize self
    sh.objSetConcrete(sh.objByAddr(seg));
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));
}

void spliceOutListSegment(
        SymHeap                &sh,
        const TValId            segAt,
        const TValId            peerAt,
        const TValId            valNext,
        TValList               *leakList)
{
    LDP_INIT(symabstract, "spliceOutListSegment");
    LDP_PLOT(symabstract, sh);

    CL_BREAK_IF(objMinLength(sh, segAt));

    const TObjId seg = sh.objByAddr(segAt);
    const EObjKind kind = sh.objKind(seg);

    TOffset offHead = 0;
    if (OK_OBJ_OR_NULL != kind)
        offHead = sh.segBinding(seg).head;

    const TObjId peer = sh.objByAddr(peerAt);

    if (OK_DLS == kind) {
        // OK_DLS --> unlink peer
        CL_BREAK_IF(seg == peer);
        CL_BREAK_IF(offHead != sh.segBinding(peer).head);
        const TValId valPrev = nextValFromSeg(sh, segAt);
        redirectRefs(sh,
                /* pointingFrom */ VAL_INVALID,
                /* pointingTo   */ peerAt,
                /* redirectTo   */ valPrev,
                /* offHead      */ offHead);
    }

    // unlink self
    redirectRefs(sh,
            /* pointingFrom */ VAL_INVALID,
            /* pointingTo   */ segAt,
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

        // append a trace node for this operation
        Trace::Node *tr = new Trace::SpliceOutNode(sh.traceNode());

        todo.push_back(sh0);
        todo.back().traceUpdate(tr);
    }
    else
        // we are going to detach one node
        --(*pLen);

    LDP_INIT(symabstract, "concretizeObj");
    LDP_PLOT(symabstract, sh);
}

void abstractIfNeeded(SymHeap &sh)
{
#if SE_DISABLE_SLS && SE_DISABLE_DLS
    return;
#endif
    BindingOff          off;
    TObjId              entry;
    unsigned            len;

    while ((len = discoverBestAbstraction(sh, &off, &entry))) {
        if (!applyAbstraction(sh, off, entry, len))
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

    const TObjId seg = sh.objByAddr(addr);
    const TValId segAt = sh.valRoot(addr);
    const TValId peer = segPeer(sh, segAt);

    // handle the possibly empty variant (if exists)
    TMinLen len = sh.segMinLength(seg);
    spliceOutSegmentIfNeeded(&len, sh, segAt, peer, todo, leakList);

    const EObjKind kind = sh.objKind(seg);
    sh.traceUpdate(new Trace::ConcretizationNode(sh.traceNode(), kind));

    if (isMayExistObj(kind)) {
        // these kinds are much easier than regular list segments
        sh.objSetConcrete(seg);
        decrementProtoLevel(sh, seg);
        LDP_PLOT(symabstract, sh);
        CL_BREAK_IF(!protoCheckConsistency(sh));
        return;
    }

    // duplicate self as abstract object (including all prototypes)
    const TValId dup = segDeepCopy(sh, segAt);

    // resolve the relative placement of the 'next' pointer
    const BindingOff off = sh.segBinding(seg);
    const TOffset offNext = (OK_SLS == kind)
        ? off.next
        : off.prev;

    // concretize self
    sh.objSetConcrete(seg);

    // update 'next' pointer
    const PtrHandle nextPtr(sh, sh.valByOffset(segAt, offNext));
    const TValId dupHead = segHeadAt(sh, dup);
    nextPtr.setValue(dupHead);

    if (OK_DLS == kind) {
        // update 'prev' pointer going from peer to the cloned object
        const PtrHandle prev1 = prevPtrFromSeg(sh, peer);
        prev1.setValue(dupHead);

        // update 'prev' pointer going from the cloned object to the concrete
        const PtrHandle prev2(sh, sh.valByOffset(dup, off.next));
        const TValId headAddr = sh.valByOffset(segAt, off.head);
        prev2.setValue(headAddr);

        CL_BREAK_IF(!dlSegCheckConsistency(sh));
    }

    // if there was a self loop from 'next' to the segment itself, recover it
    const PtrHandle nextNextPtr = nextPtrFromSeg(sh, dup);
    const TValId nextNextVal = nextNextPtr.value();
    const TValId nextNextRoot = sh.valRoot(nextNextVal);
    if (nextNextRoot == dup)
        // FIXME: we should do this also the other way around for OK_DLS
        nextNextPtr.setValue(sh.valByOffset(segAt, sh.valOffset(nextNextVal)));

    sh.segSetMinLength(sh.objByAddr(dup), len);
    sh.segSetMinLength(sh.objByAddr(segPeer(sh, dup)), len);

    LDP_PLOT(symabstract, sh);

    CL_BREAK_IF(!protoCheckConsistency(sh));
}
