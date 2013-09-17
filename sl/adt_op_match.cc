/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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
#include "adt_op_match.hh"

#include "adt_op_meta.hh"
#include "cont_shape_seq.hh"
#include "symseg.hh"                // for objMinLength()

#include <cl/cl_msg.hh>

#include <algorithm>                // for std::reverse

#include <boost/foreach.hpp>

namespace AdtOp {

typedef FixedPoint::TObjectMapper                   TObjectMapper;
typedef FixedPoint::THeapIdent                      THeapIdent;
typedef FixedPoint::TShapeIdent                     TShapeIdent;

struct MatchCtx {
    TMatchList                     &matchList;
    const OpCollection             &opCollection;
    const TProgState               &progState;
    FixedPoint::TShapeSeqList       shapeSeqs;

    MatchCtx(
            TMatchList             &matchList_,
            const OpCollection     &opCollection_,
            const TProgState       &progState_):
        matchList(matchList_),
        opCollection(opCollection_),
        progState(progState_)
    {
        FixedPoint::collectShapeSequences(&shapeSeqs, progState);
    }
};

unsigned countObjects(const SymHeap &sh)
{
    TObjList objs;
    sh.gatherObjects(objs);
    return objs.size();
}

enum EContext {
    C_TEMPLATE,
    C_PROGRAM,
    C_TOTAL
};

void processRegSuffix(
        TObjectMapper              *pMap,
        TObjList                    pObjLists[1][C_TOTAL],
        const SymHeap              &shTpl,
        const SymHeap              &shProg)
{
    TObjList &list0 = (*pObjLists)[0];
    TObjList &list1 = (*pObjLists)[1];

    while (!list0.empty() && !list1.empty()) {
        const TObjId obj0 = list0.back();
        const TObjId obj1 = list1.back();

        // check that both objects are regions
        if (OK_REGION !=  shTpl.objKind(obj0))
            break;
        if (OK_REGION != shProg.objKind(obj1))
            break;

        // remember the mapping and remove the pair of regions
        pMap->insert(obj0, obj1);
        list0.pop_back();
        list1.pop_back();
    }
}

void swapObjLists(TObjList pObjLists[1][C_TOTAL])
{
    for (unsigned u = 0U; u < C_TOTAL; ++u) {
        TObjList &objList = (*pObjLists)[u];
        std::reverse(objList.begin(), objList.end());
    }
}

bool matchAnchorHeapCore(
        TObjectMapper              *pMap,
        const SymHeap              &shProg,
        const SymHeap              &shTpl,
        const Shape                &csProg,
        const Shape                &csTpl)
{
    if (2U < csTpl.length || csTpl.length != countObjects(shTpl)) {
        CL_BREAK_IF("unsupported anchor heap in a template");
        return false;
    }

    // clear the destination object map (if not already)
    pMap->clear();

    // resolve list of objects belonging to containers shapes
    TObjList objLists[C_TOTAL];
    objListByShape(&objLists[C_TEMPLATE], shTpl, csTpl);
    objListByShape(&objLists[C_PROGRAM], shProg, csProg);
    CL_BREAK_IF(objLists[C_TEMPLATE].empty());

    // handle matching regions at both end-points
    for (unsigned u = 0; u < 2; ++u) {
        swapObjLists(&objLists);
        processRegSuffix(pMap, &objLists, shTpl, shProg);
    }

    if (objLists[C_TEMPLATE].empty() && objLists[C_PROGRAM].empty())
        // all regions were mapped and nothing remains
        return true;

    if (objLists[C_TEMPLATE].empty() || objLists[C_PROGRAM].empty())
        // we are no longer able to map the remaining objects
        return false;

    if (1U < objLists[C_TEMPLATE].size()) {
        CL_BREAK_IF("unsupported match of the anchor heap");
        return false;
    }

    // check the kind of the (only) remaining object in the template
    const TObjId tplObj = objLists[C_TEMPLATE].front();
    const EObjKind tplObjKind = shTpl.objKind(tplObj);
    switch (tplObjKind) {
        case OK_REGION:
            // unmatched region in the template
            return false;

        case OK_DLS:
            break;

        default:
            CL_BREAK_IF("unsupported object kind in matchAnchorHeapCore()");
            return false;
    }

    // map the remaining DLS in the template with the remaining program objects
    BOOST_FOREACH(const TObjId progObj, objLists[C_PROGRAM])
        pMap->insert(tplObj, progObj);

    // successfully matched!
    return true;
}

bool matchAnchorHeap(
        FootprintMatch             *pDst,
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const OpFootprint          &fp,
        const TFootprintIdent      &fpIdent,
        const TShapeIdent          &shIdent)
{
    // resolve program state and shape
    using namespace FixedPoint;
    const SymHeap &shProg = *heapByIdent(ctx.progState, shIdent.first);
    const Shape &csProg = *shapeByIdent(ctx.progState, shIdent);

    // check search direction
    bool reverse = false;
    const ESearchDirection sd = tpl.searchDirection();
    switch (sd) {
        case SD_FORWARD:
            break;

        case SD_BACKWARD:
            reverse = true;
            break;

        default:
            CL_BREAK_IF("matchAnchorHeap() got invalid search direction");
    }

    // resolve template state and shape list
    const SymHeap &shTpl = (reverse)
        ? fp.output
        : fp.input;
    const TShapeListByHeapIdx &csTplListByIdx = (reverse)
        ? tpl.outShapes()
        : tpl.inShapes();

    // check the count of container shapes in the template
    const TShapeList &csTplList = csTplListByIdx[fpIdent./* footprint */second];
    if (1U != csTplList.size()) {
        CL_BREAK_IF("unsupported count of shapes in matchAnchorHeap()");
        return false;
    }

    // resolve objMap by search direction
    const EFootprintPort port = (reverse)
        ? FP_DST
        : FP_SRC;

    // perform an object-wise match
    const Shape &csTpl = csTplList.front();
    if (!matchAnchorHeapCore(&pDst->objMap[port], shProg, shTpl, csProg, csTpl))
        return false;

    // successful match!
    pDst->props = csProg.props;
    pDst->tplProps = csTpl.props;
    pDst->heap[port] = shIdent.first;
    return true;
}

TObjId relocSingleObj(const TObjId obj, const TObjectMapper &objMap)
{
    // do not relocate ID of special objects
    switch (obj) {
        case OBJ_INVALID:
            CL_BREAK_IF("relocSingleObj() got OBJ_INVALID");
            // fall through!
        case OBJ_NULL:
            return obj;

        default:
            break;
    }

    // query the object map to obtain the resulting object ID
    TObjList objList;
    objMap.query<D_LEFT_TO_RIGHT>(&objList, obj);
    if (1U != objList.size()) {
        CL_BREAK_IF("unsupported ID mapping in relocSingleObj()");
        return OBJ_INVALID;
    }

    // an unambiguous ID has been found!
    return objList.front();
}

void relocObjsInMetaOps(TMetaOpSet *pMetaOps, const TObjectMapper &objMap)
{
    const TMetaOpSet &src = *pMetaOps;
    TMetaOpSet dst;

    BOOST_FOREACH(MetaOperation mo, src) {
        mo.obj = relocSingleObj(mo.obj, objMap);
        if (MO_SET == mo.code)
            mo.tgtObj = relocSingleObj(mo.tgtObj, objMap);

        dst.insert(mo);
    }

    pMetaOps->swap(dst);
}

void relocFieldOffset(MetaOperation *pMetaOp, const FootprintMatch &fm)
{
    const BindingOff &bfTpl = fm.tplProps.bOff;
    const BindingOff &bfProg = fm.props.bOff;
    TOffset &off = pMetaOp->off;

    CL_BREAK_IF(bfTpl.next == bfTpl.prev);

    if (off == bfTpl.next)
        off = bfProg.next;
    else if (off == bfTpl.prev)
        off = bfProg.prev;
}

void relocOffsetsInMetaOps(TMetaOpSet *pMetaOps, const FootprintMatch &fm)
{
    const TMetaOpSet &src = *pMetaOps;
    TMetaOpSet dst;

    BOOST_FOREACH(MetaOperation mo, src) {
        switch (mo.code) {
            case MO_SET:
                // TODO: relocate target offset according to head offset
                // fall through!
            case MO_UNSET:
                relocFieldOffset(&mo, fm);
                // fall through!
            default:
                break;
        }

        dst.insert(mo);
    }

    pMetaOps->swap(dst);
}

bool jumpToNextHeap(
        THeapIdent                 *pNext,
        TObjectMapper              *pObjmap,
        const THeapIdent            heapCurrent,
        const TProgState           &progState,
        const ESearchDirection      sd)
{
    using namespace FixedPoint;
    const LocalState &locState = progState[heapCurrent./* loc */first];

    // check search direction
    bool reverse = false;
    switch (sd) {
        case SD_FORWARD:
            break;

        case SD_BACKWARD:
            reverse = true;
            break;

        default:
            CL_BREAK_IF("jumpToNextHeap() got invalid search direction");
    }

    const TEdgeListByHeapIdx &eListByHeapIdx = (reverse)
        ? locState.traceInEdges
        : locState.traceOutEdges;

    const TTraceEdgeList &edgeList = eListByHeapIdx[heapCurrent./* sh */second];
    if (edgeList.empty())
        // no successor/predecessor
        return false;

    if (1U != edgeList.size()) {
        CL_BREAK_IF("jumpToNextHeap() would have ambiguous result");
        return false;
    }

    const TraceEdge *te = edgeList.front();
    *pNext = (reverse)
        ? te->src
        : te->dst;

    *pObjmap = te->objMap;
    if (reverse)
        pObjmap->flip();

    return true;
}

void seekTemplateMatchInstances(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        FootprintMatch              fm,
        TMetaOpSet                  metaOpsToLookFor)
{
    EFootprintPort beg, end;
    const ESearchDirection sd = tpl.searchDirection();
    switch (sd) {
        case SD_FORWARD:
            beg = FP_SRC;
            end = FP_DST;
            break;

        case SD_BACKWARD:
            beg = FP_DST;
            end = FP_SRC;
            break;

        default:
            CL_BREAK_IF("seekTemplateMatchInstances() got invalid direction");
            return;
    }

    TObjectMapper objMap = fm.objMap[beg];
    relocObjsInMetaOps(&metaOpsToLookFor, objMap);
    CL_BREAK_IF(metaOpsToLookFor.empty());

    // crawl the fixed-point
    THeapIdent heapCurrent = fm.heap[beg];
    std::set<THeapIdent> seen;
    seen.insert(heapCurrent);

    THeapIdent heapNext;
    while (!metaOpsToLookFor.empty()) {
        if (!jumpToNextHeap(&heapNext, &objMap, heapCurrent, ctx.progState, sd))
            return;

        if (!insertOnce(seen, heapNext)) {
            CL_BREAK_IF("loop detected in seekTemplateMatchInstances()");
            return;
        }

        // resolve the pair of heaps in program configuration
        const SymHeap *shCurrnet = heapByIdent(ctx.progState, heapCurrent);
        const SymHeap *shNext = heapByIdent(ctx.progState, heapNext);
        const SymHeap &sh0 = (SD_FORWARD  == sd) ? *shCurrnet : *shNext;
        const SymHeap &sh1 = (SD_BACKWARD == sd) ? *shCurrnet : *shNext;

        if (SD_BACKWARD == sd)
            relocObjsInMetaOps(&metaOpsToLookFor, objMap);

        // compute the difference of the pair of heaps
        TMetaOpSet metaOpsNow;
        if (!diffHeaps(&metaOpsNow, sh0, sh1)) {
            CL_BREAK_IF("diffHeaps() has failed");
            return;
        }

        BOOST_FOREACH(MetaOperation mo, metaOpsNow) {
            const EStorageClass code = sh0.objStorClass(mo.obj);
            if (isProgramVar(code))
                // we are not interested in changing non-heap variables
                continue;

            if (1U == metaOpsToLookFor.erase(mo))
                continue;

            if (MO_SET == mo.code && OK_REGION ==  sh0.objKind(mo.tgtObj)) {
                // translate TS_REGION to TS_{FIRST,LAST} if appropriate
                mo.tgtTs = TS_FIRST;
                if (1U == metaOpsToLookFor.erase(mo))
                    continue;
                mo.tgtTs = TS_LAST;
                if (1U == metaOpsToLookFor.erase(mo))
                    continue;
                mo.tgtTs = TS_REGION;
            }

            CL_BREAK_IF("please implement independency checking");
            return;
        }

        // move to the next one
        heapCurrent = heapNext;
        if (SD_FORWARD == sd)
            relocObjsInMetaOps(&metaOpsToLookFor, objMap);
    }

    fm.heap[end] = heapNext;
    // TODO: initialize fm.objMap[end]
    CL_DEBUG("[ADT] template instance matched: tpl = " << tpl.name()
            << ", beg = " << fm.heap[beg].first << "/" << fm.heap[beg].second
            << ", end = " << fm.heap[end].first << "/" << fm.heap[end].second);
    ctx.matchList.push_back(fm);
}

void matchSingleFootprint(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const OpFootprint          &fp,
        const TFootprintIdent      &fpIdent)
{
    TMetaOpSet metaOps;
    bool diffComputed = false;

    BOOST_FOREACH(const FixedPoint::ShapeSeq seq, ctx.shapeSeqs) {
        // resolve shape sequence to search through
        FixedPoint::TShapeIdentList shapes;
        expandShapeSequence(&shapes, seq, ctx.progState);
        if (SD_FORWARD == tpl.searchDirection())
            // reverse the sequence if searching _forward_
            std::reverse(shapes.begin(), shapes.end());

        // allocate a structure for the match result
        FootprintMatch fm(fpIdent);

        // search anchor heap
        bool found = false;
        BOOST_FOREACH(const TShapeIdent &shIdent, shapes) {
            if (matchAnchorHeap(&fm, ctx, tpl, fp, fpIdent, shIdent)) {
                found = true;
                break;
            }
        }
        if (!found)
            // no anchor heap found
            continue;

        if (!diffComputed) {
            // time to diff the template
            if (!diffHeaps(&metaOps, fp.input, fp.output)) {
                CL_BREAK_IF("AdtOp::diffHeaps() has failed");
                return;
            }

            relocOffsetsInMetaOps(&metaOps, fm);
            diffComputed = true;
        }

        seekTemplateMatchInstances(ctx, tpl, fm, metaOps);
    }
}

void matchTemplate(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const TTemplateIdx          tplIdx)
{
    const TFootprintIdx fpCnt = tpl.size();
    for (TFootprintIdx fpIdx = 0; fpIdx < fpCnt; ++fpIdx) {
        const OpFootprint &fp = tpl[fpIdx];
        const TFootprintIdent fpIdent(tplIdx, fpIdx);
        matchSingleFootprint(ctx, tpl, fp, fpIdent);
    }
}

void matchFootprints(
        TMatchList                 *pDst,
        const OpCollection         &opCollection,
        const TProgState           &progState)
{
    MatchCtx ctx(*pDst, opCollection, progState);
    CL_DEBUG("[ADT] found " << ctx.shapeSeqs.size()
            << " container shape sequences");

    const TTemplateIdx tplCnt = opCollection.size();
    for (TTemplateIdx tplIdx = 0; tplIdx < tplCnt; ++tplIdx) {
        const OpTemplate &tpl = ctx.opCollection[tplIdx];
        CL_DEBUG("[ADT] trying to match template: " << tpl.name());
        matchTemplate(ctx, tpl, tplIdx);
    }
}

} // namespace AdtOp
