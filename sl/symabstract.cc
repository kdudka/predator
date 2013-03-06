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

void reconnectProto(
        SymHeap                &sh,
        const TObjId            proto,
        const TObjId            clone,
        const TObjId            ownerSrc,
        const TObjId            ownerDst,
        const ETargetSpecifier  tsTarget)
{
    redirectRefs(sh,
            /* pointingFrom  */ ownerDst,
            /* pointingTo    */ proto,
            /* pointingWith  */ TS_INVALID,
            /* redirectTo    */ clone,
            /* redirectWith  */ TS_INVALID);

    redirectRefs(sh,
            /* pointingFrom  */ clone,
            /* pointingTo    */ ownerSrc,
            /* pointingWith  */ TS_INVALID,
            /* redirectTo    */ ownerDst,
            /* redirectWith  */ tsTarget);
}

void clonePrototypes(
        SymHeap                &sh,
        const TObjId            objDst,
        const TObjId            objSrc,
        const TObjSet          &protos,
        Trace::TIdMapper       &idMapper)
{
    // allocate lists for object IDs
    const unsigned cnt = protos.size();
    TObjList protoList(cnt);
    TObjList cloneList(cnt);

    // export the 'protos' set to a vector
    unsigned i = 0;
    BOOST_FOREACH(const TObjId obj, protos)
        protoList[i++] = obj;

    // clone the prototypes and reconnect them to the new owner and prototypes
    for (unsigned i = 0; i < cnt; ++i) {
        const TObjId protoI = protoList[i];
        const TObjId cloneI = protoClone(sh, protoList[i]);
        cloneList[i] = cloneI;

        // update object IDs mapping
        idMapper.insert(protoI, protoI);
        idMapper.insert(protoI, cloneI);

        reconnectProto(sh, protoI, cloneI, objSrc, objDst, TS_REGION);

        for (unsigned j = 0; j < i; ++j) {
            const TObjId protoJ = protoList[j];
            const TObjId cloneJ = cloneList[j];
            reconnectProto(sh, protoI, cloneI, protoJ, cloneJ, TS_INVALID);
        }
    }
}

// clone a segment including its prototypes and decrement their nesting level
TObjId regFromSegDeep(SymHeap &sh, TObjId seg, Trace::TIdMapper &idMapper)
{
    // collect the list of prototypes
    TObjSet protoList;
    collectPrototypesOf(protoList, sh, seg);

    // clone the object itself
    const TObjId reg = sh.objClone(seg);
    sh.objSetConcrete(reg);

    // update object IDs mapping
    idMapper.insert(seg, seg);
    idMapper.insert(seg, reg);

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
    clonePrototypes(sh, reg, seg, protoList, idMapper);

    return reg;
}

bool segAbstractionStep(
        SymHeap                     &sh,
        const ShapeProps            &props,
        TObjId                      *pCursor,
        Trace::TIdMapper            &idMapper)
{
    const BindingOff &off = props.bOff;

    // resolve the pair of objects to be merged
    const TObjId obj0 = *pCursor;
    const TObjId obj1 = nextObj(sh, obj0, off.next);

    // use a sand box since we do not know whether the join is still doable
    SymHeap sandBox(sh);
    Trace::waiveCloneOperation(sandBox);

    // join data
    TObjId seg;
    TObjSet protos[2];
    if (!joinData(sandBox, props, obj0, obj1, &seg, &protos, 0, &idMapper)) {
        CL_DEBUG("segAbstractionStep() forces segment re-discovery");
        return false;
    }

    // data joined successfully, pick the resulting heap
    sandBox.swap(sh);

    // store ID mapping of the roots and allow identity for all but prototypes
    idMapper.insert(obj0, seg);
    idMapper.insert(obj1, seg);
    idMapper.setNotFoundAction(Trace::TIdMapper::NFA_RETURN_IDENTITY);

    // add obj0/obj1 to the set of allowed bottom-up referrers
    protos[0].insert(obj0);
    protos[1].insert(obj1);

    // redirect pointers going to 'obj0' from left to 'seg'
    redirectRefsNotFrom(sh, protos[0], obj0, seg, TS_FIRST, canPointToFront);

    // preserve valNext
    const TValId valNext = valOfPtr(sh, obj1, off.next);
    const PtrHandle nextPtr(sh, seg, off.next);
    nextPtr.setValue(valNext);

    if (OK_DLS == props.kind) {
        // redirect pointers going to 'obj1' from right to 'seg'
        redirectRefsNotFrom(sh, protos[1], obj1, seg, TS_LAST, canPointToBack);

        // preserve valPrev
        const TValId valPrev = valOfPtr(sh, obj0, off.prev);
        const PtrHandle prevPtr(sh, seg, off.prev);
        prevPtr.setValue(valPrev);
    }

    // destroy 'obj0' and 'obj1', including all prototypes
    REQUIRE_GC_ACTIVITY(sh, obj1, segAbstractionStep);
    if (collectJunk(sh, obj0))
        CL_DEBUG("segAbstractionStep() drops a sub-heap (obj0)");

    // move to the resulting segment
    *pCursor = seg;
    return true;
}

bool applyAbstraction(
        SymHeap                    &sh,
        const Shape                &shape)
{
    const char *name = "[unhandled kind of abstract object]";

    const EObjKind kind = shape.props.kind;
    switch (kind) {
        case OK_SLS:
            name = "SLS";
            break;

        case OK_DLS:
            name = "DLS";
            break;

        default:
            CL_BREAK_IF("applyAbstraction() got something special");
    }

    const unsigned len = shape.length;
    CL_DEBUG("    AAA initiating " << name << " abstraction of length " << len);

    // cursor
    TObjId cursor = shape.entry;

    LDP_INIT(symabstract, name);
    LDP_PLOT(symabstract, sh);

    for (unsigned i = 0; i < len; ++i) {
        CL_BREAK_IF(!protoCheckConsistency(sh));

        // create the trace node in advance, so that we can capture ID mapping
        using namespace Trace;
        AbstractionNode *trAbs = new AbstractionNode(sh.traceNode(), kind);
        NodeHandle trAbsHandle(trAbs);

        if (!segAbstractionStep(sh, shape.props, &cursor, trAbs->idMapper())) {
            CL_DEBUG("<-- validity of next " << (len - i - 1)
                    << " abstraction step(s) broken, forcing re-discovery...");

            if (i)
                return true;

            CL_BREAK_IF("segAbstractionStep() failed, nothing has been done");
            return false;
        }

        // dump a shape graph if debugging
        std::string pName;
        LDP_PLOTN(symabstract, sh, &pName);

        // abstraction step has succeeded, update the trace graph!
        trAbs->setPlotName(pName);
        sh.traceUpdate(trAbs);

        CL_BREAK_IF(!protoCheckConsistency(sh));
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void dlSegReplaceByConcrete(SymHeap &sh, const TObjId obj)
{
    CL_BREAK_IF(OK_DLS != sh.objKind(obj));

    std::string pName;
    LDP_INIT(symabstract, "dlSegReplaceByConcrete");
    LDP_PLOTN(symabstract, sh, &pName);
    CL_BREAK_IF(!segCheckConsistency(sh));
    CL_BREAK_IF(!protoCheckConsistency(sh));

    // append a trace node for this operation (object IDs preserved 1:1)
    Trace::Node *trOrig = sh.traceNode();
    Trace::Node *trNode = new Trace::ConcretizationNode(trOrig, OK_DLS, pName);
    trNode->idMapper().setNotFoundAction(Trace::TIdMapper::NFA_RETURN_IDENTITY);
    sh.traceUpdate(trNode);

    // redirect all references originally pointing to obj
    redirectRefs(sh,
            /* pointingFrom */ OBJ_INVALID,
            /* pointingTo   */ obj,
            /* pointingWith */ TS_INVALID,
            /* redirectTo   */ obj,
            /* redirectWith */ TS_REGION);

    // convert OK_DLS to OK_REGION
    sh.objSetConcrete(obj);

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

    // append a trace node for this operation
    sh.traceUpdate(new Trace::SpliceOutNode(sh.traceNode()));

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

TMinLen spliceOutSegmentIfNeeded(
        SymHeap                &sh,
        const TObjId            seg,
        TSymHeapList           &todo,
        TObjSet                *leakObjs)
{
    const TMinLen len = sh.segMinLength(seg);
    if (0 < len)
        return len - 1;

    // possibly empty LS
    SymHeap sh0(sh);
    Trace::waiveCloneOperation(sh0);
    spliceOutListSegment(sh0, seg, leakObjs);
    todo.push_back(sh0);
    Trace::waiveCloneOperation(todo.back());
    return 0;
}

void abstractIfNeeded(SymHeap &sh)
{
#if SE_DISABLE_SLS && SE_DISABLE_DLS
    return;
#endif
    Shape shape;
    while (discoverBestAbstraction(&shape, sh)) {
        if (!applyAbstraction(sh, shape))
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
    const TMinLen len = spliceOutSegmentIfNeeded(sh, seg, todo, leakObjs);

    std::string pName;
    LDP_INIT(symabstract, "concretizeObj");
    LDP_PLOTN(symabstract, sh, &pName);

    const EObjKind kind = sh.objKind(seg);

    // append a trace node for this operation
    Trace::Node *trOrig = sh.traceNode();
    Trace::Node *trNode = new Trace::ConcretizationNode(trOrig, kind, pName);
    sh.traceUpdate(trNode);

    // set default ID mapping to identity
    Trace::TIdMapper &idMapper = trNode->idMapper();
    idMapper.setNotFoundAction(Trace::TIdMapper::NFA_RETURN_IDENTITY);

    if (isMayExistObj(kind)) {
        // these kinds are much easier than regular list segments
        sh.objSetConcrete(seg);
        decrementProtoLevel(sh, seg);
        LDP_PLOT(symabstract, sh);
        CL_BREAK_IF(!protoCheckConsistency(sh));
        return;
    }

    // duplicate seg (including all prototypes) and convert to OK_REGION
    const TObjId reg = regFromSegDeep(sh, seg, idMapper);

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
