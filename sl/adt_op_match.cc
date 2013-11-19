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
#include <cl/storage.hh>            // for CodeStorage::TypeDb::dataPtrSizeof()

#include <algorithm>                // for std::reverse

#include <boost/foreach.hpp>

namespace AdtOp {

bool debuggingTplMatch;

#define TM_DEBUG(msg) do {              \
    if (!AdtOp::debuggingTplMatch)      \
        break;                          \
    CL_DEBUG(msg);                      \
} while (0)

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
        TMapOrder                  *pObjOrder,
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
        TM_DEBUG("matchAnchorHeapCore() might be insufficiently implemented");
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

    bool ambiguousMapping = (1U < objLists[C_PROGRAM].size());

    // map the remaining DLS in the template with the remaining program objects
    BOOST_FOREACH(const TObjId progObj, objLists[C_PROGRAM]) {
        pMap->insert(tplObj, progObj);

        if (ambiguousMapping)
            (*pObjOrder)[tplObj].push_back(progObj);
    }

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
    TObjectMapper *pObjMap = &pDst->objMap[port];
    TMapOrder *pObjOrder = &pDst->objMapOrder;
    if (!matchAnchorHeapCore(pObjMap, pObjOrder, shProg, shTpl, csProg, csTpl))
        return false;

    // successful match!
    pDst->props = csProg.props;
    pDst->tplProps = csTpl.props;
    pDst->matchedHeaps.push_back(shIdent.first);
    return true;
}

TObjId relocAmbiguousObj(
        const TObjId                tplObj,
        const ETargetSpecifier      tplTs,
        const TObjList             &objList,
        const TMapOrder            *pObjOrder)
{
    if (!pObjOrder) {
        CL_BREAK_IF("relocAmbiguousObj() got pObjOrder == NULL");
        return OBJ_INVALID;
    }

    const TMapOrder::const_iterator it = pObjOrder->find(tplObj);
    if (it == pObjOrder->end()) {
        CL_BREAK_IF("relocAmbiguousObj() failed to lookup template object");
        return OBJ_INVALID;
    }

    const TObjList objOrder = it->second;
    BOOST_FOREACH(const TObjId obj, objList) {
        if (objOrder.end() != std::find(objOrder.begin(), objOrder.end(), obj))
            continue;

        CL_BREAK_IF("unsupported ID mapping in relocAmbiguousObj()");
        return OBJ_INVALID;
    }

    switch (tplTs) {
        case TS_FIRST:
            return objOrder.front();

        case TS_LAST:
            return objOrder.back();

        default:
            CL_BREAK_IF("invalid call of relocAmbiguousObj()");
            return OBJ_INVALID;
    }
}

TObjId relocSingleObj(
        const TObjId                obj,
        const ETargetSpecifier      ts,
        const TObjectMapper        &objMap,
        const TMapOrder            *pObjOrder)
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
    if (1U == objList.size())
        // an unambiguous ID has been found!
        return objList.front();

    return relocAmbiguousObj(obj, ts, objList, pObjOrder);
}

ETargetSpecifier tsByOffset(const TOffset off, const ShapeProps *pProps)
{
    if (!pProps)
        // no data to proceed
        return TS_INVALID;

    const BindingOff &bf = pProps->bOff;
    if (off == bf.next)
        return TS_LAST;

    if (off == bf.prev)
        return TS_FIRST;

    CL_BREAK_IF("tsByOffset() failed to resolve target specifier");
    return TS_INVALID;
}

void relocObjsInMetaOps(
        TMetaOpSet                 *pMetaOps,
        const TObjectMapper        &objMap,
        const ShapeProps           *pProps      = 0,
        const TMapOrder            *pObjOrder   = 0)
{
    const TMetaOpSet &src = *pMetaOps;
    TMetaOpSet dst;

    BOOST_FOREACH(MetaOperation mo, src) {
        ETargetSpecifier ts = TS_INVALID;
        if (MO_SET == mo.code) {
            mo.tgtObj = relocSingleObj(mo.tgtObj, mo.tgtTs, objMap, pObjOrder);
            ts = tsByOffset(mo.off, pProps);
        }

        mo.obj = relocSingleObj(mo.obj, ts, objMap, pObjOrder);
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

typedef std::vector<THeapIdent>                     THeapIdentList;
typedef std::vector<TObjectMapper>                  TObjectMapperList;

void collectNextHeaps(
        THeapIdentList             *pHeapList,
        TObjectMapperList          *pObjMapList,
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
    typedef const TraceEdge *TEdgePtr;
    BOOST_FOREACH(const TEdgePtr te, edgeList) {
        const THeapIdent heapNext = (reverse)
            ? te->src
            : te->dst;

        TObjectMapper objMap = te->objMap;
        if (reverse)
            objMap.flip();

        pHeapList->push_back(heapNext);
        pObjMapList->push_back(objMap);
    }
}

bool removeOpFrom(TMetaOpSet *pLookup, const SymHeap &sh0, MetaOperation mo)
{
    if (1U == pLookup->erase(mo))
        return true;

    if (MO_SET != mo.code)
        return false;
    
    const EObjKind kind = sh0.objKind(mo.tgtObj);
    if (OK_REGION != kind)
        return false;

    // translate TS_REGION to TS_{FIRST,LAST} if appropriate
    mo.tgtTs = TS_FIRST;
    if (1U == pLookup->erase(mo))
        return true;
    mo.tgtTs = TS_LAST;
    if (1U == pLookup->erase(mo))
        return true;

    return false;
}

bool isIndependentOp(
        FootprintMatch             *pMatch,
        const SymHeap              &sh,
        const MetaOperation        &mo)
{
    if (MO_SET != mo.code)
        return false;

    // FIXME: we assume that all fields in MetaOperation have sizeof(void *)
    const TStorRef stor = sh.stor();
    const TSizeOf psize = stor.types.dataPtrSizeof();
    const TOffset winLo = mo.off;
    const TOffset winHi = winLo + psize;

    const BindingOff &bOff = pMatch->props.bOff;
    std::set<TOffset> offs;
    offs.insert(bOff.next);
    offs.insert(bOff.prev);
    BOOST_FOREACH(const TOffset lo, offs) {
        const TOffset hi = lo + psize;
        if (winHi <= lo)
            continue;

        if (hi <= winLo)
            continue;

        // field clash detected!
        return false;
    }

    // proven independent
    return true;
}
        
bool processDiffOf(
        FootprintMatch             *pMatch,
        TMetaOpSet                 *pLookup,
        const TProgState           &progState,
        const THeapIdent            heap0,
        const THeapIdent            heap1,
        const ESearchDirection      sd)
{
    const SymHeap &sh0 = *heapByIdent(progState, heap0);
    const SymHeap &sh1 = *heapByIdent(progState, heap1);

    // compute the difference of the pair of heaps
    TMetaOpSet metaOpsNow;
    if (!diffHeaps(&metaOpsNow, sh0, sh1)) {
        TM_DEBUG("diffHeaps() has failed");
        return false;
    }

    BOOST_FOREACH(const MetaOperation &mo, metaOpsNow) {
        const SymHeap &sh = (MO_FREE == mo.code) ? sh0 : sh1;
        const EStorageClass code = sh.objStorClass(mo.obj);
        if (isProgramVar(code))
            // we are not interested in changing non-heap variables
            continue;

        if (removeOpFrom(pLookup, sh0, mo)) {
            THeapIdentSeq *pDst = &pMatch->matchedHeaps;

            if (SD_FORWARD == sd) {
                if (!pDst->empty())
                    pDst->pop_back();

                pDst->push_back(heap0);
                pDst->push_back(heap1);
            }
            else /* SD_BACKWARD */ {
                if (pDst->empty())
                    pDst->push_back(heap1);

                pDst->push_front(heap0);
            }

            continue;
        }

        if (isIndependentOp(pMatch, sh0, mo))
            continue;

        if (MO_FREE == mo.code) {
            TM_DEBUG("unexpected MO_FREE meta-operation in processDiffOf()");
            return false;
        }

        TM_DEBUG("failed to prove independency of a meta-operation");
        return false;
    }

    return true;
}

struct SeekContext {
    FootprintMatch                  fm;
    TMetaOpSet                      metaOpsToLookFor;
    TObjectMapper                   objMapFromTpl;
    THeapIdent                      heapCurrent;
    std::set<THeapIdent>            seen;

    SeekContext(
            const FootprintMatch   &fm_,
            const TMetaOpSet       &metaOpsToLookFor_,
            const ESearchDirection  sd):
        fm(fm_),
        metaOpsToLookFor(metaOpsToLookFor_),
        objMapFromTpl(fm_.objMap[(SD_FORWARD == sd) ? FP_SRC : FP_DST]),
        heapCurrent(fm_.matchedHeaps.front())
    {
        // pick the starting position and clear the list of matched heaps
        CL_BREAK_IF(1U != fm.matchedHeaps.size());
        seen.insert(heapCurrent);
        fm.matchedHeaps.clear();

        // relocate object IDs
        relocObjsInMetaOps(&metaOpsToLookFor, objMapFromTpl,
                /* to resolve ambiguous mapping */ &fm.props, &fm.objMapOrder);
    }
};

void seekTemplateMatchInstances(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const FootprintMatch       &fmInit,
        const TMetaOpSet           &metaOps)
{
    const ESearchDirection sd = tpl.searchDirection();
    const SeekContext seekCtxInit(fmInit, metaOps, sd);
    CL_BREAK_IF(seekCtxInit.metaOpsToLookFor.empty());

    // crawl the fixed-point
    // TODO: optimize to skip push/pack when traversing linear graphs
    std::stack<SeekContext> seekStack;
    seekStack.push(seekCtxInit);
    while (!seekStack.empty()) {
        SeekContext seekCtx = seekStack.top();
        seekStack.pop();

        FootprintMatch &fm = seekCtx.fm;
        const THeapIdent heapCurr = seekCtx.heapCurrent;
        TMetaOpSet &metaOpsToLookFor = seekCtx.metaOpsToLookFor;

        // collect the list of successor heaps (together with object mapping)
        THeapIdentList heapList;
        TObjectMapperList objMapList;
        collectNextHeaps(&heapList, &objMapList, heapCurr, ctx.progState, sd);
        const unsigned cnt = heapList.size();
        CL_BREAK_IF(cnt != objMapList.size());

        // iterate through successors
        for (unsigned idx = 0U; idx < cnt; ++idx) {
            const THeapIdent heapNext = heapList[idx];
            if (!insertOnce(seekCtx.seen, heapNext))
                // loop detected
                continue;

            // resolve the pair of heaps in program configuration
            const THeapIdent heap0 = (SD_FORWARD  == sd) ? heapCurr : heapNext;
            const THeapIdent heap1 = (SD_BACKWARD == sd) ? heapCurr : heapNext;

            // resolve the current ID map
            const TObjectMapper &objMap = objMapList[idx];
            if (SD_BACKWARD == sd)
                relocObjsInMetaOps(&metaOpsToLookFor, objMap);

            if (!processDiffOf(&fm, &metaOpsToLookFor, ctx.progState,
                        heap0, heap1, sd))
                // failed to process the difference of the neighbouring heaps
                continue;

            // reflect the current traversal step in over-all mapping of objects
            seekCtx.objMapFromTpl.composite<D_LEFT_TO_RIGHT>(objMap);

            if (metaOpsToLookFor.empty()) {
                // matched!
                const EFootprintPort end = (SD_BACKWARD == sd)
                    ? FP_SRC
                    : FP_DST;
                fm.objMap[end] = seekCtx.objMapFromTpl;
                ctx.matchList.push_back(fm);

                const THeapIdent src = fm.matchedHeaps.front();
                const THeapIdent dst = fm.matchedHeaps.back();
                CL_DEBUG("[ADT] template instance matched: tpl = " << tpl.name()
                        << "[" << fm.footprint.second << "]"
                        << ", src = " << src.first << "/" << src.second
                        << ", dst = " << dst.first << "/" << dst.second);
                continue;
            }

            // schedule the successor heap
            if (SD_FORWARD == sd)
                relocObjsInMetaOps(&metaOpsToLookFor, objMap);
            seekCtx.heapCurrent = heapNext;
            seekStack.push(seekCtx);
        }
    }
}

bool diffHeapsIfNeeded(
        TMetaOpSet                 *pMetaOps,
        const OpFootprint          &fp,
        const FootprintMatch       &fm)
{
    if (!pMetaOps->empty())
        // diff already computed
        return true;

    // time to diff the template
    if (!diffHeaps(pMetaOps, fp.input, fp.output)) {
        CL_BREAK_IF("AdtOp::diffHeaps() has failed");
        return false;
    }

    relocOffsetsInMetaOps(pMetaOps, fm);
    if (pMetaOps->empty()) {
        CL_BREAK_IF("AdtOp::diffHeaps() returned an empty set");
        return false;
    }

    // diff successfully computed
    return true;
}

void matchSingleFootprint(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const OpFootprint          &fp,
        const TFootprintIdent      &fpIdent)
{
    TMetaOpSet metaOps;
    std::set<TShapeIdent> checkedShapes;

    BOOST_FOREACH(FixedPoint::TShapeSeq seq, ctx.shapeSeqs) {
        // resolve shape sequence to search through
        if (SD_FORWARD == tpl.searchDirection())
            // reverse the sequence if searching _forward_
            std::reverse(seq.begin(), seq.end());

        // search anchor heap
        BOOST_FOREACH(const TShapeIdent &shIdent, seq) {
            // allocate a structure for the match result
            FootprintMatch fm(fpIdent);

            if (!matchAnchorHeap(&fm, ctx, tpl, fp, fpIdent, shIdent))
                // failed to match anchor heap
                continue;

            if (!insertOnce(checkedShapes, shIdent))
                // already checked as part of different shape sequence
                break;

            if (!diffHeapsIfNeeded(&metaOps, fp, fm))
                // non-recoverable error while computing diff of the footprint
                return;

            // find all template match instances using this anchor heap
            seekTemplateMatchInstances(ctx, tpl, fm, metaOps);

            // now please continue with _another_ shape sequence!
            break;
        }
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
        TM_DEBUG("[ADT] trying to match template: " << tpl.name());
        matchTemplate(ctx, tpl, tplIdx);
    }
}

} // namespace AdtOp
