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

#include "cont_shape_seq.hh"

#include "worklist.hh"

#include <algorithm>                // for std::reverse

namespace FixedPoint {

typedef std::vector<TShapeIdent>                    TShapeIdentList;

bool hasSuccessorShape(const TTraceEdgeList &outEdges, const TShapeIdx csIdx)
{
    BOOST_FOREACH(const TraceEdge *e, outEdges) {
        TShapeMapper::TVector succs;
        e->csMap.query<D_LEFT_TO_RIGHT>(&succs, csIdx);
        if (!succs.empty())
            return true;
    }

    // no preceding shape found
    return false;
}

void findPredecessors(
        TShapeIdentList            *pDst,
        const TShapeIdent           csIdent,
        const GlobalState           &glState)
{
    CL_BREAK_IF(!pDst->empty());

    const THeapIdent dstShIdent = csIdent.first;
    const TShapeIdx dstCsIdx = csIdent.second;
    const TLocIdx dstLocIdx = dstShIdent.first;
    const THeapIdx dstShIdx = dstShIdent.second;

    const LocalState &dstState = glState[dstLocIdx];
    const TTraceEdgeList &inEdges = dstState.traceInEdges[dstShIdx];

    BOOST_FOREACH(const TraceEdge *e, inEdges) {
        TShapeMapper::TVector inbound;
        e->csMap.query<D_RIGHT_TO_LEFT>(&inbound, dstCsIdx);
        if (1U < inbound.size())
            CL_BREAK_IF("join nodes are not supported for now");

        if (inbound.empty())
            continue;

        const TShapeIdent srcCsIdent(e->src, inbound.front());
        pDst->push_back(srcCsIdent);
    }
}

void collectShapeSequences(TShapeSeqList *pDst, const GlobalState &glState)
{
    // for each location
    const TLocIdx locCnt = glState.size();
    for (TLocIdx dstLocIdx = 0; dstLocIdx < locCnt; ++dstLocIdx) {
        const LocalState &dstState = glState[dstLocIdx];

        // for each heap
        const THeapIdx shCnt = dstState.heapList.size();
        for (THeapIdx dstShIdx = 0; dstShIdx < shCnt; ++dstShIdx) {
            const TShapeList &shapes = dstState.shapeListByHeapIdx[dstShIdx];
            const TTraceEdgeList &outEdges = dstState.traceOutEdges[dstShIdx];

            // for each container shape
            const TShapeIdx csCnt = shapes.size();
            for (TShapeIdx dstCsIdx = 0; dstCsIdx < csCnt; ++dstCsIdx) {
                if (hasSuccessorShape(outEdges, dstCsIdx))
                    continue;

                // resolve end of the sequence
                const THeapIdent dstShIdent(dstLocIdx, dstShIdx);
                TShapeIdent dstCsIdent(dstShIdent, dstCsIdx);

                // resolve begin(s) of the sequence
                TShapeSeq seq;
                seq.push_front(dstCsIdent);

                typedef std::queue<TShapeSeq> TQueue;
                WorkList<TShapeSeq, TQueue> wl(seq);
                while (wl.next(seq)) {
                    TShapeIdentList srcIdents;
                    findPredecessors(&srcIdents, seq.front(), glState);

                    const unsigned cnt = srcIdents.size();
                    if (!cnt) {
                        // no predecessor found --> append a new sequence
                        pDst->push_back(seq);
                        continue;
                    }

                    for (unsigned i = 0U; i < cnt; ++i) {
                        seq.push_front(srcIdents[i]);
                        wl.schedule(seq);
                        if (i + 1U < cnt)
                            seq.pop_front();
                    }
                }
            }
        }
    }
}

} // namespace FixedPoint
