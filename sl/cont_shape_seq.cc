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

#include <algorithm>                // for std::reverse
#include <queue>

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

class CollectQueueItem {
    private:
        TShapeSeq                   seq_;
        std::set<TShapeIdent>       seen_;

    public:
        CollectQueueItem(const TShapeIdent &si) {
            seq_.push_front(si);
            seen_.insert(si);
        }

        const TShapeSeq& seq() const {
            return seq_;
        }

        const TShapeIdent& front() const {
            return seq_.front();
        }

        bool pushFrontOnce(const TShapeIdent &si) {
            if (!insertOnce(seen_, si))
                return false;

            seq_.push_front(si);
            return true;
        }
};

void collectShapeSequencesCore(
        TShapeSeqList              *pDst,
        const GlobalState          &glState,
        const TShapeIdent          &dstCsIdent)
{
    std::queue<CollectQueueItem> cQueue;
    const CollectQueueItem entry(dstCsIdent);
    cQueue.push(entry);

    do {
        CollectQueueItem &now = cQueue.front();

        // find predecessor container shapes
        TShapeIdentList srcIdents;
        findPredecessors(&srcIdents, now.front(), glState);

        const unsigned cnt = srcIdents.size();
        switch (cnt) {
            case 0:
                // no predecessor found --> append a new sequence
                pDst->push_back(now.seq());
                cQueue.pop();
                continue;

            case 1:
                // optimized variant for the most common case
                if (now.pushFrontOnce(srcIdents.front()))
                    continue;

            default:
                break;
        }

        // move the top of the queue out of the structure
        const CollectQueueItem snap(now);
        cQueue.pop();

        // schedule all predecessor (not closing a loop) for processing
        BOOST_FOREACH(const TShapeIdent &si, srcIdents) {
            CollectQueueItem next(snap);
            if (next.pushFrontOnce(si))
                cQueue.push(next);
        }
    }
    while (!cQueue.empty());
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
                const TShapeIdent dstCsIdent(dstShIdent, dstCsIdx);

                // find all shape sequences ending with dstCsIdent
                collectShapeSequencesCore(pDst, glState, dstCsIdent);
            }
        }
    }
}

} // namespace FixedPoint
