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

#include "cont_shape_seq.hh"

#include <cl/cl_msg.hh>

namespace AdtOp {

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

void matchSingleFootprint(
        MatchCtx                   &ctx,
        const OpTemplate           &tpl,
        const OpFootprint          &fp,
        const TFootprintIdent      &fpIdent)
{
    // TODO
    (void) ctx;
    (void) tpl;
    (void) fp;
    (void) fpIdent;
    CL_BREAK_IF("please implement");
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
