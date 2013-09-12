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

    if (csProg.length != csTpl.length) {
        CL_BREAK_IF("only 1:1 mapping is supported by matchAnchorHeapCore()");
        return false;
    }

    // clear the destination object map (if not already)
    pMap->clear();

    // resolve list of objects belonging to containers shapes
    TObjList tplObjs, progObjs;
    objListByShape(&tplObjs,  shTpl,  csTpl);
    objListByShape(&progObjs, shProg, csProg);

    // count the objects
    const unsigned objsCnt = tplObjs.size();
    CL_BREAK_IF(objsCnt != progObjs.size());

    // iterate object-wise
    for (unsigned idx = 0U; idx < objsCnt; ++idx) {
        const TObjId objTpl  = tplObjs [idx];
        const TObjId objProg = progObjs[idx];

        const EObjKind kindTpl  = shTpl .objKind(objTpl);
        const EObjKind kindProg = shProg.objKind(objProg);
        if (kindTpl != kindProg) {
            CL_BREAK_IF("kind mismatch not supported by matchAnchorHeap()");
            return false;
        }

        const TMinLen lenTpl  = objMinLength(shTpl,  objTpl);
        const TMinLen lenProg = objMinLength(shProg, objProg);
        if (lenTpl != lenProg) {
            CL_BREAK_IF("minLen mismatch not supported by matchAnchorHeap()");
            return false;
        }

        // remember the mapping of objects
        pMap->insert(objTpl, objProg);
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

    // resolve tamplate state and shape list
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

void seekTemplateMatchInstances(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        FootprintMatch              fm,
        TMetaOpSet                  metaOpsToLookFor)
{
    EDirection mapDirection;
    EFootprintPort beg, end;
    const ESearchDirection sd = tpl.searchDirection();
    switch (sd) {
        case SD_FORWARD:
            beg = FP_SRC;
            end = FP_DST;
            mapDirection = D_LEFT_TO_RIGHT;
            break;

        case SD_BACKWARD:
            beg = FP_DST;
            end = FP_SRC;
            mapDirection = D_RIGHT_TO_LEFT;
            break;

        default:
            CL_BREAK_IF("seekTemplateMatchInstances() got invalid direction");
            return;
    }

    const TObjectMapper &objMap = fm.objMap[beg];
    relocObjsInMetaOps(&metaOpsToLookFor, objMap);

    // TODO
    (void) ctx;
    (void) metaOpsToLookFor;
    (void) mapDirection;
    (void) end;
    CL_BREAK_IF("please implement");
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
        if (SD_BACKWARD == tpl.searchDirection())
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
