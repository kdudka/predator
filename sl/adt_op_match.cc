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
typedef FixedPoint::TLocIdx                         TLocIdx;
typedef FixedPoint::THeapIdx                        THeapIdx;
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

    // compute total minimal length of the remaining template objects
    TMinLen lenTpl = 0;
    BOOST_FOREACH(const TObjId objTpl, objLists[C_TEMPLATE])
        lenTpl += objMinLength(shTpl, objTpl);

    // compute total minimal length of the remaining program objects
    TMinLen lenProg = 0;
    BOOST_FOREACH(const TObjId objProg, objLists[C_PROGRAM])
        lenProg += objMinLength(shProg, objProg);

    if (lenProg < lenTpl)
        // the program configuration does not guarantee sufficient list length
        return false;

    // map the remaining objects from both sets with each oder
    BOOST_FOREACH(const TObjId objTpl, objLists[C_TEMPLATE])
        BOOST_FOREACH(const TObjId objProg, objLists[C_PROGRAM])
            pMap->insert(objTpl, objProg);

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
    if (!matchAnchorHeapCore(pObjMap, shProg, shTpl, csProg, csTpl))
        return false;

    // successful match!
    pDst->props = csProg.props;
    pDst->tplProps = csTpl.props;
    pDst->matchedHeaps.push_back(shIdent.first);
    return true;
}

TObjId selectMappedObjByTs(
        const SymHeap              &sh,
        const BindingOff           &bOff,
        const TObjList             &objList,
        const ETargetSpecifier      ts)
{
    switch (ts) {
        case TS_FIRST:
        case TS_LAST:
            break;

        default:
            TM_DEBUG("selectMappedObjByTs() got invalid target specifier");
            return OBJ_INVALID;
    }

    // start with all objects as candidates for fst/lst
    TObjSet allObjs, cObjs;
    BOOST_FOREACH(const TObjId obj, objList) {
        allObjs.insert(obj);
        cObjs.insert(obj);
    }

    const TOffset offNext = (TS_FIRST == ts) ? bOff.next : bOff.prev;
    const TOffset offPrev = (TS_LAST  == ts) ? bOff.next : bOff.prev;

    SymHeap &shWritable = const_cast<SymHeap &>(sh);

    BOOST_FOREACH(const TObjId obj, objList) {
        const TObjId objNext = nextObj(shWritable, obj, offNext);
        cObjs.erase(objNext);

        const TObjId objPrev = nextObj(shWritable, obj, offPrev);
        if (hasKey(allObjs, objPrev))
            cObjs.erase(obj);
    }

    if (1U != cObjs.size()) {
        CL_BREAK_IF("unsupported ID mapping in selectMappedObjByTs()");
        return OBJ_INVALID;
    }

    return *cObjs.begin();
}

ETargetSpecifier tsByOffset(const TOffset off, const ShapeProps &props)
{
    const BindingOff &bf = props.bOff;
    if (off == bf.next)
        return TS_LAST;

    if (off == bf.prev)
        return TS_FIRST;

    CL_BREAK_IF("tsByOffset() failed to resolve target specifier");
    return TS_INVALID;
}

bool relocSingleObj(
        TObjId                     *pObj,
        const ETargetSpecifier      ts,
        const TObjectMapper        &objMap,
        const ShapeProps           &props,
        const SymHeap              &sh)
{
    // do not relocate ID of special objects
    TObjId obj = *pObj;
    switch (obj) {
        case OBJ_INVALID:
            CL_BREAK_IF("relocSingleObj() got OBJ_INVALID");
            return false;

        case OBJ_NULL:
            return true;

        default:
            break;
    }

    // query the object map to obtain the resulting object ID
    TObjList objList;
    objMap.query<D_LEFT_TO_RIGHT>(&objList, obj);
    obj = (1U == objList.size())
        ? /* unambiguous ID mapping */ objList.front()
        : selectMappedObjByTs(sh, props.bOff, objList, ts);

    if (OBJ_INVALID == obj)
        return false;

    *pObj = obj;
    return true;
}

bool relocObjsInMetaOps(
        TMetaOpSet                 *pMetaOps,
        const TObjectMapper        &objMap,
        const ShapeProps           &props,
        const SymHeap              &sh)
{
    const TMetaOpSet &src = *pMetaOps;
    TMetaOpSet dst;

    BOOST_FOREACH(MetaOperation mo, src) {
        ETargetSpecifier ts = TS_INVALID;
        if (MO_SET == mo.code) {
            ts = tsByOffset(mo.off, props);
            const ETargetSpecifier tgtTs = (TS_REGION == mo.tgtTs)
                ? ts
                : mo.tgtTs;
            if (!relocSingleObj(&mo.tgtObj, tgtTs, objMap, props, sh))
                return false;
        }

        if (!relocSingleObj(&mo.obj, ts, objMap, props, sh))
            return false;

        dst.insert(mo);
    }

    pMetaOps->swap(dst);
    return true;
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

typedef const FixedPoint::TraceEdge                *TEdgePtr;

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

    TObjSet freshObjs;
    BOOST_FOREACH(const MetaOperation &mo, metaOpsNow)
        if (MO_ALLOC == mo.code)
            freshObjs.insert(mo.obj);

    BOOST_FOREACH(const MetaOperation &mo, metaOpsNow) {
        const SymHeap &sh = (hasKey(freshObjs, mo.obj)) ? sh1 : sh0;
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
    }
};

void seekTemplateMatchInstances(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const FootprintMatch       &fmInit,
        const TMetaOpSet           &metaOps,
        const TShapeIdent          &shIdent)
{
    const ESearchDirection sd = tpl.searchDirection();
    SeekContext seekCtxInit(fmInit, metaOps, sd);

    // relocate object IDs
    const SymHeap &sh = *heapByIdent(ctx.progState, shIdent.first);
    if (!relocObjsInMetaOps(&seekCtxInit.metaOpsToLookFor,
                seekCtxInit.objMapFromTpl, fmInit.props, sh))
        return;

    // crawl the fixed-point
    // TODO: optimize to skip push/pack when traversing linear graphs
    std::stack<SeekContext> seekStack;
    seekStack.push(seekCtxInit);
    while (!seekStack.empty()) {
        const SeekContext seekCtxOrig = seekStack.top();
        seekStack.pop();

        const THeapIdent heapCurr = seekCtxOrig.heapCurrent;

        // collect the list of successor heaps (together with object mapping)
        THeapIdentList heapList;
        TObjectMapperList objMapList;
        collectNextHeaps(&heapList, &objMapList, heapCurr, ctx.progState, sd);
        const unsigned cnt = heapList.size();
        CL_BREAK_IF(cnt != objMapList.size());

        // iterate through successors
        for (unsigned idx = 0U; idx < cnt; ++idx) {
            SeekContext seekCtx = seekCtxOrig;
            TMetaOpSet &metaOpsToLookFor = seekCtx.metaOpsToLookFor;
            FootprintMatch &fm = seekCtx.fm;

            const THeapIdent heapNext = heapList[idx];
            if (!insertOnce(seekCtx.seen, heapNext))
                // loop detected
                continue;

            // resolve the pair of heaps in program configuration
            const THeapIdent heap0 = (SD_FORWARD  == sd) ? heapCurr : heapNext;
            const THeapIdent heap1 = (SD_BACKWARD == sd) ? heapCurr : heapNext;

            // resolve the current ID map
            const TObjectMapper &objMap = objMapList[idx];
            const SymHeap &sh = *heapByIdent(ctx.progState, heapNext);
            if (SD_BACKWARD == sd && !relocObjsInMetaOps(&metaOpsToLookFor,
                        objMap, fmInit.props, sh))
                continue;

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

            if (SD_FORWARD == sd && !relocObjsInMetaOps(&metaOpsToLookFor,
                        objMap, fmInit.props, sh))
                continue;

            // schedule the successor heap
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
            seekTemplateMatchInstances(ctx, tpl, fm, metaOps, shIdent);

            // now please continue with _another_ shape sequence!
            break;
        }
    }
}

void matchTemplates(
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
        const TFootprintIdx fpCnt = tpl.size();
        for (TFootprintIdx fpIdx = 0; fpIdx < fpCnt; ++fpIdx) {
            const OpFootprint &fp = tpl[fpIdx];
            const TFootprintIdent fpIdent(tplIdx, fpIdx);
            TM_DEBUG("[ADT] trying to match template: " << tpl.name()
                    << "[" << fpIdx << "]");
            matchSingleFootprint(ctx, tpl, fp, fpIdent);
        }
    }
}

/// if lastHeap is the last heap at location lastLoc, push lastLoc to *pDst
void pushBackIfLast(
        TInsnList                  *pDst,
        const TLocIdx               lastLoc,
        const THeapIdx              lastHeap,
        const TProgState           &progState)
{
    if (1U + lastHeap == progState[lastLoc].heapList.size())
        pDst->push_back(lastLoc);
}

typedef std::set<THeapIdent>                        THeapSet;

/// this assumes heapSet to be sorted by location idx, then by heap idx
void collectReplacedInsnsCore(
        TInsnList                  *pDst,
        const THeapSet             &heapSet,
        const TProgState           &progState)
{
    TLocIdx lastLoc = -1;
    THeapIdx lastHeap = -1;

    BOOST_FOREACH(const THeapIdent &heap, heapSet) {
        const TLocIdx currLoc = heap.first;
        if (-1 != lastLoc && lastLoc != currLoc) {
            // we have reached next location ==> was the last sequence correct?
            pushBackIfLast(pDst, lastLoc, lastHeap, progState);
            lastHeap = -1;
        }

        // check whether the current heap follows right after the previous one
        const THeapIdx currHeap = heap.second;
        lastLoc = (lastHeap + 1 == currHeap)
            ? currLoc
            : -1;

        // move to the next loc/heap pair
        lastHeap = currHeap;
    }

    if (-1 != lastLoc)
        // finally check the very last sequence
        pushBackIfLast(pDst, lastLoc, lastHeap, progState);
}

void collectReplacedInsns(
        TInsnListByTplIdx          *pDst,
        const TMatchList           &matchList,
        const TProgState           &progState)
{
    CL_BREAK_IF(!pDst->empty());

    // set of matched heaps per each template
    typedef std::vector<THeapSet>                   THeapSetByTpl;
    THeapSetByTpl heapSetByTpl;

    // go through all matched footprints and populate heapSetByTpl
    BOOST_FOREACH(const FootprintMatch &fm, matchList) {
        const THeapIdentSeq &heapList = fm.matchedHeaps;
        const TTemplateIdx tpl = fm.footprint./* tpl idx */first;
        if (heapSetByTpl.size() <= static_cast<unsigned>(tpl))
            heapSetByTpl.resize(1U + tpl);

        // skip the last element of the list while collecting the set
        THeapSet &heapSet = heapSetByTpl[tpl];
        std::copy(heapList.begin(), std::prev(heapList.end()),
                std::inserter(heapSet, heapSet.begin()));
    }

    // allocate space required for the result
    const TTemplateIdx tplCnt = heapSetByTpl.size();
    pDst->resize(tplCnt);

    // collect the set of replaced insns for each template
    TInsnListByTplIdx &dst = *pDst;
    for (TTemplateIdx tpl = 0; tpl < tplCnt; ++tpl)
        collectReplacedInsnsCore(&dst[tpl], heapSetByTpl[tpl], progState);
}

bool applyMatch(THeapSet *pHeapPool, const FootprintMatch &fm, const bool ro)
{
    // remove the last heap as it does not represent an insn to be replaced
    THeapIdentSeq matchedHeaps = fm.matchedHeaps;
    CL_BREAK_IF(matchedHeaps.size() < 2U);
    matchedHeaps.pop_back();

    if (ro) {
        // try it read-only
        BOOST_FOREACH(const THeapIdent &heap, matchedHeaps)
            if (!hasKey(pHeapPool, heap))
                // incomplete match!
                return false;
    }
    else {
        // remove all heaps used by this match
        BOOST_FOREACH(const THeapIdent &heap, matchedHeaps)
            pHeapPool->erase(heap);
    }

    // all OK!
    return true;
}

void selectApplicableMatches(
        TMatchList                 *pMatchList,
        const TProgState           &progState)
{
    // collect instructions to be replaced
    const TMatchList &mlOrig = *pMatchList;
    TInsnListByTplIdx insnsToBeReplaced;
    collectReplacedInsns(&insnsToBeReplaced, mlOrig, progState);

    // collect heaps to be replaced
    THeapSet toReplace;
    BOOST_FOREACH(const TInsnList &insns, insnsToBeReplaced) {
        BOOST_FOREACH(const TLocIdx locIdx, insns) {
            const THeapIdx cntHeaps = progState[locIdx].heapList.size();
            for (THeapIdx heapIdx = 0; heapIdx < cntHeaps; ++heapIdx) {
                const THeapIdent heap(locIdx, heapIdx);
                toReplace.insert(heap);
            }
        }
    }

    // sort templates by the number of instructions they would replace
    typedef std::vector<TTemplateIdx>               TTplIdxList;
    typedef std::map<int /* prio */, TTplIdxList>   TTplIdxByPrio;
    TTplIdxByPrio tplIdxByPrio;
    const TTemplateIdx tplCnt = insnsToBeReplaced.size();
    for (TTemplateIdx tpl = 0; tpl < tplCnt; ++tpl) {
        const unsigned replCnt = insnsToBeReplaced[tpl].size();
        if (!replCnt)
            continue;

        const int prio = -replCnt;
        tplIdxByPrio[prio].push_back(tpl);
    }

    const unsigned matchCnt = mlOrig.size();

    // go through matches ordered by prio and select a set of applicable ones
    TMatchList mlSelected;
    BOOST_FOREACH(const TTplIdxByPrio::const_reference item, tplIdxByPrio)
    BOOST_FOREACH(const TTemplateIdx tpl, item.second) {
        // first check with matches are applicable
        std::vector<unsigned /* match idx */> picked;
        for (unsigned matchIdx = 0U; matchIdx < matchCnt; ++matchIdx) {
            const FootprintMatch &fm = mlOrig[matchIdx];
            if (fm.footprint./* tpl idx */first != tpl)
                continue;

            if (applyMatch(&toReplace, fm, /* ro */ true))
                picked.push_back(matchIdx);
        }

        // then remove the replaced heaps from the set and append the result
        BOOST_FOREACH(const unsigned matchIdx, picked) {
            const FootprintMatch &fm = mlOrig[matchIdx];
            applyMatch(&toReplace, fm, /* ro */ false);
            mlSelected.push_back(fm);
        }
    }

    if (!toReplace.empty()) {
        CL_BREAK_IF("unhandled partial match in selectApplicableMatches()");
        mlSelected.clear();
    }

    pMatchList->swap(mlSelected);
    CL_DEBUG("[ADT] count of applicable template match instances: "
            << pMatchList->size());
}

} // namespace AdtOp
