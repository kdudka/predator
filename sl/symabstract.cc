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

#include <algorithm>
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
        switch (code) {
            case VT_INVALID:
            case VT_COMPOSITE:
                CL_BREAK_IF("UnknownValuesDuplicator sees an invalid target");
                // fall through!

            case VT_CUSTOM:
            case VT_OBJECT:
            case VT_RANGE:
                return /* continue */ true;

            case VT_UNKNOWN:
                break;
        }

        // duplicate unknown value
        const TValId valNew = translateValProto(*sh, *sh, valOld);
        fld.setValue(valNew);

        return /* continue */ true;
    }
};

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TObjId obj)
{
    UnknownValuesDuplicator visitor;
    buildIgnoreList(visitor.ignoreList, sh, obj);

    // traverse all sub-objects
    traverseLiveFields(sh, obj, visitor);
}

void detachClonedPrototype(
        SymHeap                &sh,
        const TObjId            proto,
        const TObjId            clone,
        const TObjId            ownerDst,
        const TObjId            ownerSrc,
        const bool              uplink)
{
    const bool isOwnerDls = (OK_DLS == sh.objKind(ownerDst));
    CL_BREAK_IF(isOwnerDls && (OK_DLS != sh.objKind(ownerSrc)));

    TObjId ownerDstPeer = OBJ_INVALID;
    TObjId ownerSrcPeer = OBJ_INVALID;
    if (isOwnerDls) {
        ownerDstPeer = dlSegPeer(sh, ownerDst);
        ownerSrcPeer = dlSegPeer(sh, ownerSrc);
        CL_BREAK_IF(uplink && (ownerDstPeer != ownerSrcPeer));
    }

    redirectRefs(sh, ownerDst, proto, TS_INVALID, clone,    TS_REGION);
    redirectRefs(sh, clone, ownerSrc, TS_INVALID, ownerDst, TS_REGION);

    if (isOwnerDls) {
        if (uplink)
            redirectRefs(sh, clone, ownerSrcPeer, TS_INVALID,
                    ownerDst, TS_REGION);
        else
            redirectRefs(sh, ownerDstPeer, proto, TS_INVALID,
                    clone, TS_INVALID);
    }

    if (OK_DLS == sh.objKind(proto)) {
        const TObjId protoPeer = dlSegPeer(sh, proto);
        const TObjId clonePeer = dlSegPeer(sh, clone);

        redirectRefs(sh, ownerDst, protoPeer, TS_INVALID,
                clonePeer, TS_REGION);

        redirectRefs(sh, clonePeer, ownerSrc, TS_INVALID,
                ownerDst, TS_REGION);

        if (isOwnerDls && uplink)
            redirectRefs(sh, clonePeer, ownerSrcPeer, TS_INVALID,
                    ownerDst, TS_REGION);
    }
}

TObjId protoClone(SymHeap &sh, const TObjId proto)
{
    const TObjId clone = segClone(sh, proto);
    objDecrementProtoLevel(sh, clone);

    if (OK_REGION == sh.objKind(proto))
        // clone all unknown values in order to keep prover working
        duplicateUnknownValues(sh, clone);

    return clone;
}

void clonePrototypes(
        SymHeap                &sh,
        const TObjId            objDst,
        const TObjId            objSrc,
        const TObjList         &protoList)
{
    // allocate some space for clone IDs
    const unsigned cnt = protoList.size();
    TObjList cloneList(cnt);

    // clone the prototypes and reconnect them to the new owner
    for (unsigned i = 0; i < cnt; ++i) {
        const TObjId proto = protoList[i];
        const TObjId clone = protoClone(sh, protoList[i]);

        detachClonedPrototype(sh, proto, clone, objDst, objSrc,
                /* uplink */ true);

        cloneList[i] = clone;
    }

    // FIXME: works, but likely to kill the CPU
    for (unsigned i = 0; i < cnt; ++i) {
        const TObjId proto = protoList[i];
        const TObjId clone = cloneList[i];

        for (unsigned j = 0; j < cnt; ++j) {
            if (i == j)
                continue;

            const TObjId otherProto = protoList[j];
            const TObjId otherClone = cloneList[j];
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

void dlSegSyncPeerData(SymHeap &sh, const TObjId dls)
{
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

// clone a segment including its prototypes and decrement their nesting level
TObjId segDeepCopy(SymHeap &sh, TObjId seg)
{
    // collect the list of prototypes
    TObjList protoList;
    collectPrototypesOf(protoList, sh, seg, /* skipPeers */ true);

    // clone the object itself
    const TObjId dup = objClone(sh, seg);

    // clone all unknown values in order to keep prover working
    duplicateUnknownValues(sh, dup);

    // clone all prototypes originally owned by seg
    clonePrototypes(sh, dup, seg, protoList);

    return dup;
}

void enlargeMayExist(SymHeap &sh, const TObjId obj)
{
    const EObjKind kind = sh.objKind(obj);
    if (!isMayExistObj(kind))
        return;

    decrementProtoLevel(sh, obj);
    sh.objSetConcrete(obj);
}

void slSegAbstractionStep(
        SymHeap                    &sh,
        const TObjId                obj,
        const TObjId                next,
        const BindingOff           &off)
{
    // compute the resulting minimal length
    const TMinLen len = objMinLength(sh, obj) + objMinLength(sh, next);

    enlargeMayExist(sh, obj);
    enlargeMayExist(sh, next);

    // merge data
    joinData(sh, off, next, obj, /* bidir */ false);

    if (OK_SLS != sh.objKind(next))
        // abstract the _next_ object
        sh.objSetAbstract(next, OK_SLS, off);

    // replace all references to 'head'
    const TOffset offHead = sh.segBinding(next).head;
    const TValId headAt = sh.addrOfTarget(obj, /* XXX */ TS_REGION, offHead);
    sh.valReplace(headAt, segHeadAt(sh, next));

    // destroy self, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, obj, slSegAbstractionStep);

    if (len)
        // declare resulting segment's minimal length
        sh.segSetMinLength(next, len);
}

void dlSegCreate(SymHeap &sh, TObjId obj1, TObjId obj2, BindingOff off)
{
    // compute resulting segment's length
    const TMinLen len = objMinLength(sh, obj1) + objMinLength(sh, obj2);

    // OK_SEE_THROUGH -> OK_CONCRETE if necessary
    enlargeMayExist(sh, obj1);
    enlargeMayExist(sh, obj2);

    // merge data
    joinData(sh, off, obj2, obj1, /* bidir */ true);

    swapValues(off.next, off.prev);
    sh.objSetAbstract(obj1, OK_DLS, off);

    swapValues(off.next, off.prev);
    sh.objSetAbstract(obj2, OK_DLS, off);

    // just created DLS is said to be 2+ as long as no OK_SEE_THROUGH are involved
    sh.segSetMinLength(obj1, len);
    sh.segSetMinLength(obj2, len);
}

void dlSegGobble(SymHeap &sh, TObjId dls, TObjId reg, bool backward)
{
    CL_BREAK_IF(OK_DLS != sh.objKind(dls));

    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(dls) + objMinLength(sh, reg);

    // we allow to gobble OK_SEE_THROUGH objects (if compatible)
    enlargeMayExist(sh, reg);

    if (!backward)
        // jump to peer
        dls = dlSegPeer(sh, dls);

    // merge data
    const BindingOff &off = sh.segBinding(dls);
    joinData(sh, off, dls, reg, /* bidir */ false);
    dlSegSyncPeerData(sh, dls);

    // store the pointer DLS -> VAR
    const PtrHandle nextPtr(sh, dls, off.next);
    const TValId valNext = valOfPtr(sh, reg, off.next);
    nextPtr.setValue(valNext);

    // replace VAR by DLS
    const TValId headAt = sh.addrOfTarget(reg, /* XXX */ TS_REGION, off.head);
    sh.valReplace(headAt, segHeadAt(sh, dls));
    REQUIRE_GC_ACTIVITY(sh, reg, dlSegGobble);

    // handle DLS Neq predicates
    sh.segSetMinLength(dls, len);
    sh.segSetMinLength(segPeer(sh, dls), len);

    dlSegSyncPeerData(sh, dls);
}

void dlSegMerge(SymHeap &sh, TObjId seg1, TObjId seg2)
{
    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(seg1) + sh.segMinLength(seg2);

    const TObjId peer1 = dlSegPeer(sh, seg1);
    const TObjId peer2 = dlSegPeer(sh, seg2);

    // check for a failure of segDiscover()
    CL_BREAK_IF(nextValFromSeg(sh, peer1) != segHeadAt(sh, seg2));

    // merge data
    const BindingOff &bf2 = sh.segBinding(seg2);
    joinData(sh, bf2, seg2, seg1, /* bidir */ true);

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
    if (!collectJunk(sh, peer1) && sh.isValid(peer1)) {
        CL_ERROR("dlSegMerge() failed to collect garbage"
                 ", peer1 still referenced");
        CL_BREAK_IF("collectJunk() has not been successful");
    }

    if (len) {
        // assign the resulting minimal length
        sh.segSetMinLength(seg2, len);
        sh.segSetMinLength(segPeer(sh, seg2), len);
    }

    dlSegSyncPeerData(sh, seg2);
}

bool /* jump next */ dlSegAbstractionStep(
        SymHeap                    &sh,
        const TObjId                obj,
        const TObjId                next,
        const BindingOff            &off)
{
    const EObjKind kind = sh.objKind(obj);
    const EObjKind kindNext = sh.objKind(next);
    CL_BREAK_IF(OK_SLS == kind || OK_SLS == kindNext);

    if (OK_DLS == kindNext) {
        if (OK_DLS == kind)
            // DLS + DLS
            dlSegMerge(sh, obj, next);
        else
            // CONCRETE + DLS
            dlSegGobble(sh, next, obj, /* backward */ true);

        return /* jump next */ true;
    }
    else {
        if (OK_DLS == kind)
            // DLS + CONCRETE
            dlSegGobble(sh, obj, next, /* backward */ false);
        else
            // CONCRETE + CONCRETE
            dlSegCreate(sh, obj, next, off);

        return /* nobody moves */ false;
    }
}

bool segAbstractionStep(
        SymHeap                     &sh,
        const BindingOff            &off,
        TObjId                      *pCursor)
{
    const TObjId obj = *pCursor;

    // jump to peer in case of DLS
    TObjId peer = obj;
    if (OK_DLS == sh.objKind(obj))
        peer = dlSegPeer(sh, obj);

    // jump to the next object (as we know such an object exists)
    const TObjId next = nextObj(sh, peer, off.next);

    // check wheter he upcoming abstraction step is still doable
    EJoinStatus status;
    if (!joinDataReadOnly(&status, sh, off, obj, next, 0))
        return false;

    if (isDlsBinding(off)) {
        // DLS
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
        const bool jumpNext = dlSegAbstractionStep(sh, obj, next, off);
        CL_BREAK_IF(!dlSegCheckConsistency(sh));
        if (!jumpNext)
            // stay in place
            return true;
    }
    else {
        // SLS
        slSegAbstractionStep(sh, obj, next, off);
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
    TObjId cursor = entry;

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

        std::string pName;
        LDP_PLOTN(symabstract, sh, &pName);

        Trace::Node *trAbs =
            new Trace::AbstractionNode(sh.traceNode(), kind, pName);
        sh.traceUpdate(trAbs);

        CL_BREAK_IF(!protoCheckConsistency(sh));
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void dlSegReplaceByConcrete(SymHeap &sh, TObjId seg, TObjId peer)
{
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
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ peer,
            /* pointingWith */ /* XXX */ TS_INVALID,
            /* redirectTo   */ seg,
            /* redirectWith */ /* XXX */ TS_REGION);

    // destroy peer, including all prototypes
    REQUIRE_GC_ACTIVITY(sh, peer, dlSegReplaceByConcrete);

    // concretize self
    sh.objSetConcrete(seg);
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!dlSegCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));
}

void spliceOutListSegment(
        SymHeap                &sh,
        const TObjId            seg,
        const TObjId            peer,
        const TValId            valNext,
        TObjSet                *leakObjs)
{
    LDP_INIT(symabstract, "spliceOutListSegment");
    LDP_PLOT(symabstract, sh);

    const EObjKind kind = sh.objKind(seg);

    CL_BREAK_IF(objMinLength(sh, seg));

    TOffset offHead = 0;
    if (OK_OBJ_OR_NULL != kind)
        offHead = sh.segBinding(seg).head;

    if (OK_DLS == kind) {
        // OK_DLS --> unlink peer
        CL_BREAK_IF(seg == peer);
        CL_BREAK_IF(offHead != sh.segBinding(peer).head);
        const TValId valPrev = nextValFromSeg(sh, seg);
        redirectRefs(sh,
                /* pointingFrom */ OBJ_INVALID,
                /* pointingTo   */ peer,
                /* pointingWith */ TS_INVALID,
                /* redirectTo   */ sh.objByAddr(valPrev),
                /* redirectWith */ TS_INVALID,
                /* offHead      */ sh.valOffset(valPrev) - offHead);
    }

    // unlink self
    redirectRefs(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ seg,
            /* pointingWith */ TS_INVALID,
            /* redirectTo   */ sh.objByAddr(valNext),
            /* redirectWith */ TS_INVALID,
            /* offHead      */ sh.valOffset(valNext) - offHead);

    collectSharedJunk(sh, seg, leakObjs);

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
        const TObjId            seg,
        const TObjId            peer,
        TSymHeapList           &todo,
        TObjSet                *leakObjs)
{
    if (!*pLen) {
        // possibly empty LS
        SymHeap sh0(sh);
        const TValId valNext = nextValFromSeg(sh0, peer);
        spliceOutListSegment(sh0, seg, peer, valNext, leakObjs);

        // append a trace node for this operation
        Trace::Node *tr = new Trace::SpliceOutNode(sh.traceNode());

        todo.push_back(sh0);
        todo.back().traceUpdate(tr);
    }
    else
        // we are going to detach one node
        --(*pLen);
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

void redirectRefsNotFrom(
        SymHeap                &sh,
        const TObjList         &pointingNotFrom,
        const TObjId            pointingTo,
        const TObjId            redirectTo)
{
    // go through all objects pointing at/inside pointingTo
    FldList refs;
    sh.pointedBy(refs, pointingTo);
    BOOST_FOREACH(const FldHandle &fld, refs) {
        const TObjId refObj = fld.obj();
        if (pointingNotFrom.end() != std::find(
                    pointingNotFrom.begin(),
                    pointingNotFrom.end(),
                    refObj))
            continue;

        // check the current link
        const TValId nowAt = fld.value();

        const ETargetSpecifier ts = sh.targetSpec(nowAt);
        const TValId baseAddr = sh.addrOfTarget(redirectTo, ts);
        TValId result;

        // TODO
        CL_BREAK_IF(VT_RANGE == sh.valTarget(nowAt));

        // shift the base address by calar offset
        const TOffset offToRoot = sh.valOffset(nowAt);
        result = sh.valByOffset(baseAddr, offToRoot);

        // store the redirected value
        fld.setValue(result);
    }
}

void concretizeObj(
        SymHeap                     &sh,
        const TObjId                 seg,
        TSymHeapList                &todo,
        TObjSet                     *leakObjs)
{
    CL_BREAK_IF(!protoCheckConsistency(sh));

    const TObjId peer = segPeer(sh, seg);

    // handle the possibly empty variant (if exists)
    TMinLen len = sh.segMinLength(seg);
    spliceOutSegmentIfNeeded(&len, sh, seg, peer, todo, leakObjs);

    std::string pName;
    LDP_INIT(symabstract, "concretizeObj");
    LDP_PLOTN(symabstract, sh, &pName);

    const EObjKind kind = sh.objKind(seg);
    sh.traceUpdate(new Trace::ConcretizationNode(sh.traceNode(), kind, pName));

    if (isMayExistObj(kind)) {
        // these kinds are much easier than regular list segments
        sh.objSetConcrete(seg);
        decrementProtoLevel(sh, seg);
        LDP_PLOT(symabstract, sh);
        CL_BREAK_IF(!protoCheckConsistency(sh));
        return;
    }

    // duplicate seg (including all prototypes) and convert to OK_REGION
    const TObjId dup = segDeepCopy(sh, seg);
    /* XXX */ TObjList ignoreList;
    /* XXX */ collectPrototypesOf(ignoreList, sh, seg, /* skipPeers */ false);
    /* XXX */ collectPrototypesOf(ignoreList, sh, dup, /* skipPeers */ false);
    sh.objSetConcrete(dup);

    // unless we use TS_FIRST/TS_LAST properly, we cannot use redirectRefs()
    /* XXX */ ignoreList.push_back(seg);
    /* XXX */ ignoreList.push_back(dup);
    /* XXX */ ignoreList.push_back(peer);
    redirectRefsNotFrom(sh, ignoreList, seg, dup);

    // resolve the relative placement of the 'next' pointer
    const BindingOff off = sh.segBinding(seg);
    const TOffset offNext = (OK_SLS == kind)
        ? off.next
        : off.prev;

    // update 'next' pointer
    const PtrHandle nextPtr(sh, dup, offNext);
    const TValId segHead = segHeadAt(sh, seg);
    nextPtr.setValue(segHead);

    if (OK_DLS == kind) {
        // redirect 'prev' pointer from seg to the cloned (concrete) object
        const PtrHandle prev = nextPtrFromSeg(sh, seg);
        const TValId headAddr = sh.addrOfTarget(dup,
                /* XXX */ TS_REGION, off.head);
        prev.setValue(headAddr);

        CL_BREAK_IF(!dlSegCheckConsistency(sh));
    }

    // if there was a self loop from 'next' to the segment itself, recover it
    const PtrHandle nextNextPtr = nextPtrFromSeg(sh, peer);
    const TValId nextNextVal = nextNextPtr.value();
    const TObjId nextNextObj = sh.objByAddr(nextNextVal);
    if (nextNextObj == seg)
        // FIXME: we should do this also the other way around for OK_DLS
        nextNextPtr.setValue(sh.addrOfTarget(dup,
                    /* XXX */ TS_REGION,
                    sh.valOffset(nextNextVal)));

    sh.segSetMinLength(seg,  len);
    sh.segSetMinLength(peer, len);

    LDP_PLOT(symabstract, sh);

    CL_BREAK_IF(!protoCheckConsistency(sh));
}
