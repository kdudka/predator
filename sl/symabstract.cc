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
    redirectRefs(sh, ownerDst, proto, TS_INVALID, clone, TS_INVALID);
    redirectRefs(sh, clone, ownerSrc, TS_INVALID, ownerDst, (uplink)
            ? TS_REGION
            : TS_INVALID);
}

TObjId protoClone(SymHeap &sh, const TObjId proto)
{
    const TObjId clone = sh.objClone(proto);
    objDecrementProtoLevel(sh, clone);

    // recover pointers to self
    redirectRefs(sh,
            /* pointingFrom  */ clone,
            /* pointingTo    */ proto,
            /* pointingWith  */ TS_INVALID,
            /* redirectTo    */ clone,
            /* redirectWith  */ TS_INVALID);

    if (OK_REGION == sh.objKind(proto))
        // clone all unknown values in order to keep prover working
        duplicateUnknownValues(sh, clone);

    return clone;
}

void clonePrototypes(
        SymHeap                &sh,
        const TObjId            objDst,
        const TObjId            objSrc,
        const TObjSet          &protos)
{
    // allocate lists for object IDs
    const unsigned cnt = protos.size();
    TObjList protoList(cnt);
    TObjList cloneList(cnt);

    // export the 'protos' set to a vector
    unsigned i = 0;
    BOOST_FOREACH(const TObjId obj, protos)
        protoList[i++] = obj;

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

// clone a segment including its prototypes and decrement their nesting level
TObjId regFromSegDeep(SymHeap &sh, TObjId seg)
{
    // collect the list of prototypes
    TObjSet protoList;
    collectPrototypesOf(protoList, sh, seg);

    // clone the object itself
    const TObjId reg = sh.objClone(seg);
    sh.objSetConcrete(reg);

    // recover pointers to self
    redirectRefs(sh,
            /* pointingFrom  */ reg,
            /* pointingTo    */ seg,
            /* pointingWith  */ TS_INVALID,
            /* redirectTo    */ reg,
            /* redirectWith  */ TS_REGION);

    // clone all unknown values in order to keep prover working
    duplicateUnknownValues(sh, reg);

    // clone all prototypes originally owned by seg
    clonePrototypes(sh, reg, seg, protoList);

    return reg;
}

TObjId /* objDst */ slSegAbstractionStep(
        SymHeap                    &sh,
        const TObjId                obj1,
        const TObjId                obj2,
        const BindingOff           &off)
{
    // join data
    TObjId seg;
    TObjSet protos[2];
    if (!joinData(sh, off, obj1, obj2, &seg, &protos)) {
        CL_BREAK_IF("call of joinData() failed in slSegAbstractionStep()");
        return OBJ_INVALID;
    }

    // compute the list of objects that can point up to 'obj1'
    TObjSet &privSet = protos[0];
    privSet.insert(obj1);

    // redirect pointers going to 'obj1' from left to 'seg'
    redirectRefsNotFrom(sh, privSet, obj1, seg, TS_FIRST);

    // preserve valNext
    const TValId valNext = valOfPtr(sh, obj2, off.next);
    const PtrHandle nextPtr(sh, seg, off.next);
    nextPtr.setValue(valNext);

    // destroy 'obj1' and 'obj2', including all prototypes
    REQUIRE_GC_ACTIVITY(sh, obj2, slSegAbstractionStep);
    REQUIRE_GC_ACTIVITY(sh, obj1, slSegAbstractionStep);

    return seg;
}

void dlSegCreate(SymHeap &sh, TObjId obj1, TObjId obj2, BindingOff off)
{
    // compute resulting segment's length
    const TMinLen len = objMinLength(sh, obj1) + objMinLength(sh, obj2);

    // merge data
    joinData(sh, off, obj1, obj2);

    sh.objSetAbstract(obj1, OK_DLS, off);

    TObjSet privSet;
    collectPrototypesOf(privSet, sh, obj1);

    // convert the TS_REGION addresses to TS_FIRST/TS_LAST
    privSet.insert(obj1);
    redirectRefsNotFrom(sh, privSet, obj1, obj1, TS_FIRST);
    privSet.insert(obj2);
    redirectRefsNotFrom(sh, privSet, obj2, obj1, TS_LAST);

    const TValId valNext = valOfPtr(sh, obj2, off.next);
    const PtrHandle prevPtr(sh, obj1, off.next);
    prevPtr.setValue(valNext);

    REQUIRE_GC_ACTIVITY(sh, obj2, dlSegCreate);

    // just created DLS is said to be 2+ as long as no OK_SEE_THROUGH are involved
    sh.segSetMinLength(obj1, len);
}

void dlSegGobble(SymHeap &sh, TObjId dls, TObjId reg, bool backward)
{
    CL_BREAK_IF(OK_DLS != sh.objKind(dls));

    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(dls) + objMinLength(sh, reg);

    // merge data
    const BindingOff &off = sh.segBinding(dls);
    joinData(sh, off, dls, reg);

    TOffset offNext = off.next;;
    TOffset offPrev = off.prev;
    if (backward)
        swapValues(offNext, offPrev);

    const TValId headOld = sh.addrOfTarget(reg, TS_REGION, off.head);
    const TValId headNew = valOfPtr(sh, reg, offPrev);
    sh.valReplace(headOld, headNew);

    const PtrHandle nextPtr(sh, dls, offNext);
    const TValId valNext = valOfPtr(sh, reg, offNext);
    nextPtr.setValue(valNext);

    REQUIRE_GC_ACTIVITY(sh, reg, dlSegGobble);

    // handle DLS Neq predicates
    sh.segSetMinLength(dls, len);
}

void dlSegMerge(SymHeap &sh, TObjId seg1, TObjId seg2)
{
    // compute the resulting minimal length
    const TMinLen len = sh.segMinLength(seg1) + sh.segMinLength(seg2);

    // merge data
    const BindingOff &bf2 = sh.segBinding(seg2);
    joinData(sh, bf2, seg1, seg2);

    // preserve valNext
    const TValId valNext = nextValFromSeg(sh, seg2);
    const PtrHandle ptrNext = nextPtrFromSeg(sh, seg1);
    ptrNext.setValue(valNext);

    redirectRefs(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ seg2,
            /* pointingWith */ TS_LAST,
            /* redirectTo   */ seg1,
            /* redirectWith */ TS_LAST);

    // destroy seg2 including all prototypes
    REQUIRE_GC_ACTIVITY(sh, seg2, dlSegMerge);

    // assign the resulting minimal length
    sh.segSetMinLength(seg1, len);
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
        if (OK_DLS == kind) {
            // DLS + DLS
            dlSegMerge(sh, obj, next);
            return /* jump next */ false;
        }

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

    // jump to the next object (as we know such an object exists)
    const TObjId next = nextObj(sh, obj, off.next);

    // check wheter he upcoming abstraction step is still doable
    SymHeap sandBox(sh);
    if (!joinData(sandBox, off, obj, next))
        return false;

    if (isDlsBinding(off)) {
        // DLS
        const bool jumpNext = dlSegAbstractionStep(sh, obj, next, off);
        CL_BREAK_IF(!segCheckConsistency(sh));
        if (jumpNext)
            *pCursor = next;
    }
    else {
        // SLS
        *pCursor = slSegAbstractionStep(sh, obj, next, off);
    }

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
    std::string pName;
    LDP_INIT(symabstract, "dlSegReplaceByConcrete");
    LDP_PLOTN(symabstract, sh, &pName);
    CL_BREAK_IF(!segCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));

    Trace::Node *trOrig = sh.traceNode();
    sh.traceUpdate(new Trace::ConcretizationNode(trOrig, OK_DLS, pName));

    CL_BREAK_IF(seg != peer);
    CL_BREAK_IF(OK_DLS != sh.objKind(seg));
    (void) peer;

    // redirect all references originally pointing to seg
    redirectRefs(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ seg,
            /* pointingWith */ TS_INVALID,
            /* redirectTo   */ seg,
            /* redirectWith */ TS_REGION);

    // convert OK_DLS to OK_REGION
    sh.objSetConcrete(seg);

    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(!segCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));
}

void spliceOutListSegment(
        SymHeap                &sh,
        const TObjId            seg,
        TObjSet                *leakObjs)
{
    LDP_INIT(symabstract, "spliceOutListSegment");
    LDP_PLOT(symabstract, sh);
    CL_BREAK_IF(objMinLength(sh, seg));

    const TValId valNext = nextValFromSeg(sh, seg);
    const TOffset offHead = headOffset(sh, seg);

    const EObjKind kind = sh.objKind(seg);
    if (OK_DLS == kind) {
        const PtrHandle prevPtr = prevPtrFromSeg(sh, seg);
        const TValId valPrev = prevPtr.value();
        redirectRefs(sh,
                /* pointingFrom */ OBJ_INVALID,
                /* pointingTo   */ seg,
                /* pointingWith */ TS_LAST,
                /* redirectTo   */ sh.objByAddr(valPrev),
                /* redirectWith */ sh.targetSpec(valPrev),
                /* offHead      */ sh.valOffset(valPrev) - offHead);
    }

    ETargetSpecifier ts = TS_INVALID;
    switch (kind) {
        case OK_REGION:
            CL_BREAK_IF("invalid call of spliceOutListSegment()");

        case OK_OBJ_OR_NULL:
        case OK_SEE_THROUGH:
        case OK_SEE_THROUGH_2N:
            ts = TS_REGION;
            break;

        case OK_SLS:
        case OK_DLS:
            ts = TS_FIRST;
            break;
    }

    // unlink self
    redirectRefs(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ seg,
            /* pointingWith */ ts,
            /* redirectTo   */ sh.objByAddr(valNext),
            /* redirectWith */ sh.targetSpec(valNext),
            /* offHead      */ sh.valOffset(valNext) - offHead);

    collectSharedJunk(sh, seg, leakObjs);

    // destroy self, including all nested prototypes
    if (collectJunk(sh, seg))
        CL_DEBUG("spliceOutListSegment() drops a sub-heap (seg)");

    LDP_PLOT(symabstract, sh);
}

void spliceOutSegmentIfNeeded(
        TMinLen                *pLen,
        SymHeap                &sh,
        const TObjId            seg,
        TSymHeapList           &todo,
        TObjSet                *leakObjs)
{
    if (!*pLen) {
        // possibly empty LS
        SymHeap sh0(sh);
        spliceOutListSegment(sh0, seg, leakObjs);

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

void concretizeObj(
        SymHeap                     &sh,
        TSymHeapList                &todo,
        const TObjId                 seg,
        const ETargetSpecifier       ts,
        TObjSet                     *leakObjs)
{
    CL_BREAK_IF(!protoCheckConsistency(sh));
    CL_BREAK_IF(TS_ALL == ts);

    // handle the possibly empty variant (if exists)
    TMinLen len = sh.segMinLength(seg);
    spliceOutSegmentIfNeeded(&len, sh, seg, todo, leakObjs);

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
    const TObjId reg = regFromSegDeep(sh, seg);

    // redirect all TS_FIRST/TS_LAST addresses to the region
    redirectRefs(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ seg,
            /* pointingWith */ ts,
            /* redirectTo   */ reg,
            /* redirectWith */ TS_REGION);

    // resolve the relative placement of the 'next' pointer
    const BindingOff off = sh.segBinding(seg);
    const TOffset offNext = (TS_FIRST == ts)
        ? off.next
        : off.prev;

    const TValId segHead = segHeadAt(sh, seg, ts);

    // update 'next' pointer
    const PtrHandle nextPtr(sh, reg, offNext);
    nextPtr.setValue(segHead);

    if (OK_DLS == kind) {
        // redirect 'prev' pointer from seg to the cloned (concrete) object
        const PtrHandle prev = (TS_FIRST == ts)
            ? prevPtrFromSeg(sh, seg)
            : nextPtrFromSeg(sh, seg);

        const TValId headAddr = sh.addrOfTarget(reg, TS_REGION, off.head);
        prev.setValue(headAddr);
        CL_BREAK_IF(!segCheckConsistency(sh));
    }

    // if there was a self loop from 'next' to the segment itself, recover it
    const PtrHandle nextNextPtr = nextPtrFromSeg(sh, seg);
    const TValId nextNextVal = nextNextPtr.value();
    const TObjId nextNextObj = sh.objByAddr(nextNextVal);
    if (nextNextObj == seg) {
        // FIXME: we should do this also the other way around for OK_DLS
        const TOffset off = sh.valOffset(nextNextVal);
        const TValId addr = sh.addrOfTarget(reg, TS_REGION, off);
        nextNextPtr.setValue(addr);
    }

    sh.segSetMinLength(seg,  len);

    LDP_PLOT(symabstract, sh);

    CL_BREAK_IF(!protoCheckConsistency(sh));
}
