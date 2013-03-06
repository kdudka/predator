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
#include "fixed_point.hh"

#include "cont_shape.hh"
#include "symtrace.hh"
#include "worklist.hh"

#include <cl/storage.hh>

#include <boost/foreach.hpp>

namespace FixedPoint {

typedef const CodeStorage::Block                   *TBlock;

const THeapIdent InvalidHeap(-1, -1);

const SymHeap *heapByIdent(const GlobalState &glState, const THeapIdent shIdent)
{
    const TLocIdx locIdx = shIdent.first;
    const LocalState &locState = glState[locIdx];

    const THeapIdx shIdx = shIdent.second;
    const SymHeap &sh = locState.heapList[shIdx];
    return &sh;
}

bool isTransparentInsn(const TInsn insn)
{
    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_COND:
        case CL_INSN_JMP:
            return true;

        default:
            return false;
    }
}

class TraceIndex {
    public:
        TraceIndex(const GlobalState &glState):
            glState_(glState)
        {
        }

        void indexTraceOf(const TLocIdx);

        /// return InvalidHeap if not found
        THeapIdent nearestPredecessorOf(const THeapIdent) const;

    private:
        typedef std::map<const Trace::Node *, THeapIdent> TLookup;
        const GlobalState          &glState_;
        TLookup                     lookup_;
};

void TraceIndex::indexTraceOf(const TLocIdx locIdx)
{
    const SymState &state = glState_[locIdx].heapList;
    const THeapIdx shCnt = state.size();
    for (THeapIdx shIdx = 0; shIdx < shCnt; ++shIdx) {
        const THeapIdent shIdent(locIdx, shIdx);
        const SymHeap &sh = state[shIdx];
        const Trace::Node *tr = sh.traceNode();

        // we should never change the target heap of an already indexed trace node
        CL_BREAK_IF(hasKey(lookup_, tr) && lookup_[tr] != shIdent);

        lookup_[tr] = shIdent;
    }
}

THeapIdent TraceIndex::nearestPredecessorOf(const THeapIdent shIdent) const
{
    const SymHeap *const sh = heapByIdent(glState_, shIdent);
    const Trace::Node *tr = sh->traceNode();

    while (0U < tr->parents().size()) {
        // TODO: handle trace nodes with more than one parent!
        tr = tr->parent();

        // check the current trace node
        const TLookup::const_iterator it = lookup_.find(tr);
        if (it == lookup_.end())
            continue;

        // found!
        const THeapIdent shPred = it->second;
        CL_BREAK_IF(heapByIdent(glState_, shPred)->traceNode() != tr);
        return shPred;
    }

    return /* not found */ InvalidHeap;
}

typedef StateByInsn::TStateMap                      TStateMap;

typedef CleanList<LocalState>                       TStateList;
typedef CleanList<TraceEdge>                        TTraceList;
typedef std::map<TInsn, TLocIdx>                    TInsnLookup;

struct StateBuilderCtx {
    const TFnc          fnc;
    TStateList         &stateList;
    TInsnLookup         insnLookup;

    StateBuilderCtx(const TFnc fnc_, TStateList &stateList_):
        fnc(fnc_),
        stateList(stateList_)
    {
    }
};

void loadHeaps(StateBuilderCtx &ctx, const TStateMap &stateMap)
{
    typedef WorkList<TBlock> TWorkList;

    // traverse the original (block-oriented) control-flow graph
    TBlock bb = ctx.fnc->cfg.entry();
    TWorkList wl(bb);
    while (wl.next(bb)) {
        LocalState *locState;

        // go through instructions of the current basic block
        const TLocIdx insnCnt = bb->size();
        for (TLocIdx insnIdx = 0; insnIdx < insnCnt; ++insnIdx) {
            const TInsn insn = bb->operator[](insnIdx);

            BOOST_FOREACH(const TBlock bbNext, insn->targets)
                // schedule successor blocks for processing
                wl.schedule(bbNext);

            if (isTransparentInsn(insn))
                // skip instruction we do not want in the result
                continue;

            const TLocIdx locIdx = ctx.stateList.size();
            if (insnIdx)
                // update successor location of the _previous_ instruction
                locState->cfgOutEdges.push_back(locIdx);

            // allocate a new location for the current instruction
            locState = new LocalState;
            locState->insn = insn;
            ctx.stateList.append(locState);

            // store the reverse mapping from instructions to locations
            ctx.insnLookup[insn] = locIdx;

            // load heaps if a non-empty fixed-point is available for this loc
            const TStateMap::const_iterator it = stateMap.find(insn);
            if (it != stateMap.end()) {
                locState->heapList = it->second;
                Trace::waiveCloneOperation(locState->heapList);
            }
        }
    }
}

void finalizeFlow(StateBuilderCtx &ctx)
{
    const TLocIdx locCnt = ctx.stateList.size();
    for (TLocIdx locIdx = 0; locIdx < locCnt; ++locIdx) {
        LocalState *locState = ctx.stateList[locIdx];
        const TInsn insn = locState->insn;

        if (!locState->cfgOutEdges.empty()) {
            // non-terminal instructions are already handled in loadHeaps()
            CL_BREAK_IF(cl_is_term_insn(insn->code));
            continue;
        }

        // jump to terminal instruction (in most cases insn == term here)
        const TInsn term = insn->bb->back();
        CL_BREAK_IF(!cl_is_term_insn(term->code));

        BOOST_FOREACH(TBlock bb, term->targets) {
            TInsn dst = bb->front();

            // skip trivial basic blocks containing only single goto instruction
            while (1U == dst->targets.size()) {
                bb = dst->targets.front();
                dst = bb->front();
            }

            // create a new control-flow edge (originally block-level edge)
            CL_BREAK_IF(!hasKey(ctx.insnLookup, dst));
            const TLocIdx dstIdx = ctx.insnLookup[dst];
            locState->cfgOutEdges.push_back(dstIdx);
        }

        // tag loop-closing edges using the info provided by Code Listener
        BOOST_FOREACH(const unsigned tgIdx, term->loopClosingTargets)
            locState->cfgOutEdges[tgIdx].closesLoop = true;
    }

    // initialize backward control-flow edges
    for (TLocIdx srcIdx = 0; srcIdx < locCnt; ++srcIdx) {
        const LocalState *srcState = ctx.stateList[srcIdx];
        BOOST_FOREACH(CfgEdge oe, srcState->cfgOutEdges) {
            const TLocIdx dstIdx = oe.targetLoc;
            LocalState *dstState = ctx.stateList[dstIdx];
            oe.targetLoc = srcIdx;
            dstState->cfgInEdges.push_back(oe);
        }
    }
}

void initIdMapping(const GlobalState &glState, TraceEdge *te)
{
    const SymHeap *const shSrc = heapByIdent(glState, te->src);
    const SymHeap *const shDst = heapByIdent(glState, te->dst);

    const Trace::Node *const trSrc = shSrc->traceNode();
    const Trace::Node *const trDst = shDst->traceNode();

    // start with identity, then go through the trace and construct composition
    te->objMap.setNotFoundAction(TObjectMapper::NFA_RETURN_IDENTITY);

    // TODO: handle trace nodes with more than one parent!
    for(const Trace::Node *tr = trDst; trSrc != tr; tr = tr->parent())
        te->objMap.composite<D_RIGHT_TO_LEFT>(tr->idMapper());
}

void createTraceEdges(GlobalState &glState, TTraceList &traceList)
{
    const TLocIdx locCnt = glState.size();
    for (TLocIdx dstLocIdx = 0; dstLocIdx < locCnt; ++dstLocIdx) {
        LocalState &dstState = glState[dstLocIdx];

        // build trace index
        TraceIndex trIndex(glState);
        BOOST_FOREACH(const CfgEdge &ie, dstState.cfgInEdges)
            trIndex.indexTraceOf(ie.targetLoc);

        // try to find a predecessor for each local heap
        const THeapIdx heapCnt = dstState.heapList.size();
        for (THeapIdx dstHeapIdx = 0; dstHeapIdx < heapCnt; ++ dstHeapIdx) {
            const THeapIdent dstHeap(dstLocIdx, dstHeapIdx);
            const THeapIdent srcHeap = trIndex.nearestPredecessorOf(dstHeap);

            if (srcHeap == InvalidHeap)
                // predecessor not found
                continue;

            // allocate a new trace edge
            TraceEdge *te = new TraceEdge(srcHeap, dstHeap);
            traceList.append(te);
            dstState.traceInEdges.push_back(te);

            // store backward reference
            const TLocIdx srcLocIdx = srcHeap.first;
            LocalState &srcState = glState[srcLocIdx];
            srcState.traceOutEdges.push_back(te);

            // initialize object IDs mapping
            initIdMapping(glState, te);
        }
    }
}

void detectShapeMapping(
        TraceEdge                  *te,
        const SymHeap              &shSrc,
        const SymHeap              &shDst,
        const TShapeList           &srcShapes,
        const TShapeList           &dstShapes)
{
    typedef std::map<TObjSet, TShapeIdx> TIndex;
    TIndex index;

    const TShapeIdx srcCnt = srcShapes.size();
    for (TShapeIdx srcIdx = 0; srcIdx < srcCnt; ++srcIdx) {
        TObjSet keySrc;
        objSetByShape(&keySrc, shSrc, srcShapes[srcIdx]);

        // translate the object IDs using the mapping stored in the edge
        TObjSet key;
        project<D_LEFT_TO_RIGHT>(te->objMap, &key, keySrc);

        // there should be no redefinitions
        CL_BREAK_IF(hasKey(index, key));

        index[key] = srcIdx;
    }

    const TShapeIdx dstCnt = dstShapes.size();
    for (TShapeIdx dstIdx = 0; dstIdx < dstCnt; ++dstIdx) {
        TObjSet key;
        objSetByShape(&key, shDst, dstShapes[dstIdx]);

        const TIndex::const_iterator it = index.find(key);
        if (it == index.end())
            // not found
            continue;

        const TShapeIdx srcIdx = it->second;
        te->csMap.insert(srcIdx, dstIdx);
    }
}

void detectContShapes(GlobalState &glState)
{
    const TLocIdx locCnt = glState.size();
    for (TLocIdx locIdx = 0; locIdx < locCnt; ++locIdx) {
        LocalState &locState = glState[locIdx];
        const SymState &state = locState.heapList;
        detectLocalContShapes(&locState.shapeListByHeapIdx, state);
    }

    for (TLocIdx dstLocIdx = 0; dstLocIdx < locCnt; ++dstLocIdx) {
        const LocalState &dstState = glState[dstLocIdx];
        BOOST_FOREACH(TraceEdge *te, dstState.traceInEdges) {
            const TLocIdx srcLocIdx = te->src.first;
            const LocalState &srcState = glState[srcLocIdx];

            const THeapIdx srcShIdx = te->src.second;
            const THeapIdx dstShIdx = te->dst.second;

            const SymHeap &shSrc = srcState.heapList[srcShIdx];
            const SymHeap &shDst = dstState.heapList[dstShIdx];

            const TShapeList &srcShapes = srcState.shapeListByHeapIdx[srcShIdx];
            const TShapeList &dstShapes = dstState.shapeListByHeapIdx[dstShIdx];

            detectShapeMapping(te, shSrc, shDst, srcShapes, dstShapes);
        }
    }
}

GlobalState* computeStateOf(const TFnc fnc, const TStateMap &stateByInsn)
{
    GlobalState *glState = new GlobalState;

    StateBuilderCtx ctx(fnc, glState->stateList_);

    loadHeaps(ctx, stateByInsn);

    finalizeFlow(ctx);

    createTraceEdges(*glState, glState->traceList_);

    detectContShapes(*glState);

    return glState;
}

} // namespace FixedPoint
