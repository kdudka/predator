/*
 * Copyright (C) 2014 Kamil Dudka <kdudka@redhat.com>
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

#include "cont_shape_var.hh"

#include "cont_shape_seq.hh"

#include <stack>

namespace AdtOp {

const TShapeVarId InvalidShapeVar = -1;

typedef FixedPoint::TLocIdx                         TLocIdx;
typedef FixedPoint::THeapIdent                      THeapIdent;
typedef FixedPoint::TShapeIdent                     TShapeIdent;

TShapeVarId acquireFreshShapeVar(void)
{
    static TShapeVarId last;
    return ++last;
}

bool assignShapeVarOnce(
        TShapeVarByShape           *pMap,
        const TShapeIdent          &shape,
        const TShapeVarId           varId)
{
    const TShapeVarByShape::const_iterator it = pMap->find(shape);
    if (pMap->end() != it)
        return (varId == it->second);

    (*pMap)[shape] = varId;
    return true;
}

bool singleShapeByHeapIdent(
        TShapeIdent                *pShape,
        const THeapIdent           &heap,
        const TProgState           &progState)
{
    using namespace FixedPoint;

    const LocalState &locState = progState[heap.first];
    const TShapeList &shList = locState.shapeListByHeapIdx[heap.second];
    if (1U != shList.size()) {
        CL_BREAK_IF("invalid call of singleShapeByHeapIdent()");
        return false;
    }

    *pShape = TShapeIdent(heap, 0);
    return true;
}

bool assignSingleOpPort(
        TShapeVarByShape           *pMap,
        const TProgState           &progState,
        const THeapIdent           &heap,
        const TShapeVarId           var)
{
    TShapeIdent shape;
    return singleShapeByHeapIdent(&shape, heap, progState)
        && assignShapeVarOnce(pMap, shape, var);
}

bool assignOpPorts(
        TShapeVarByShape           *pDst,
        const TMatchList           &matchList,
        const OpCollection         &coll,
        const TProgState           &progState,
        const TOpList              &opList)
{
    // iterate through container operations
    BOOST_FOREACH(const TMatchIdxList &idxList, opList) {
        // we always allocate a pair of vars (even if one would suffice)
        TShapeVarId inVar = acquireFreshShapeVar();
        TShapeVarId outVar = acquireFreshShapeVar();

        // iterate through template matches for this operation
        BOOST_FOREACH(const TMatchIdx idx, idxList) {
            const FootprintMatch &fm = matchList[idx];
            const TTemplateIdx tplIdx = fm.footprint.first;
            const TFootprintIdx fpIdx = fm.footprint.second;

            // count in/out shapes according to the matched template
            const OpTemplate &tpl = coll[tplIdx];
            const unsigned cntIn = tpl.inShapes()[fpIdx].size();
            const unsigned cntOut = tpl.outShapes()[fpIdx].size();
            CL_BREAK_IF(!cntIn && !cntOut);

            // TODO
            CL_BREAK_IF(1U < cntIn || 1U < cntOut);

            if (!!cntIn) {
                // assign variable for the input container shape
                const THeapIdent inHeap = fm.matchedHeaps.front();
                if (!assignSingleOpPort(pDst, progState, inHeap, inVar))
                    return false;
            }

            if (!!cntOut) {
                // assign variable for the output container shape
                const THeapIdent outHeap = fm.matchedHeaps.back();
                if (!assignSingleOpPort(pDst, progState, outHeap, outVar))
                    return false;
            }
        }
    }

    return /* success */ true;
}

typedef std::map<TLocIdx, TShapeVarId>              TShapeVarByLoc;

void propagateVarsLookAhead(
        TShapeVarByLoc             *pDst,
        const TShapeVarByShape     &varMap,
        const TShapeVarId           varNow,
        const TLocIdx               locNow,
        const TProgState           &progState)
{
    using namespace FixedPoint;

    TShapeIdentList siblingShapeList;
    BOOST_FOREACH(TShapeVarByShape::const_reference item, varMap) {
        const TShapeIdent &shape = item.first;
        const TShapeVarId var = item.second;
        if (var != varNow)
            continue;

        const TLocIdx loc = shape./* heap */first./* loc */first;
        if (loc != locNow)
            continue;

        siblingShapeList.push_back(shape);
    }

    BOOST_FOREACH(const TShapeIdent siblingShape, siblingShapeList) {
        TShapeIdentList prevShapes;
        findPredecessors(&prevShapes, siblingShape, progState);
        BOOST_FOREACH(const TShapeIdent &prev, prevShapes) {
            const TShapeVarByShape::const_iterator it = varMap.find(prev);
            if (varMap.end() == it)
                // previous shape has no shape var assigned yet
                continue;

            const TShapeVarId prevVar = it->second;
            const THeapIdent prevHeap = prev.first;
            const TLocIdx prevLoc = prevHeap.first;
            if (hasKey(pDst, prevLoc))
                CL_BREAK_IF((*pDst)[prevLoc] != prevVar);

            (*pDst)[prevLoc] = prevVar;
        }
    }
}

bool propagateVars(
        TShapeVarByShape           *pMap,
        const TMatchList           &matchList,
        const OpCollection         &coll,
        const TProgState           &progState)
{
    using namespace FixedPoint;

    std::stack<TShapeIdent> todo;

    // iterate through template matches
    BOOST_FOREACH(const FootprintMatch &fm, matchList) {
        const TTemplateIdx tplIdx = fm.footprint.first;
        const TFootprintIdx fpIdx = fm.footprint.second;

        // count in/out shapes according to the matched template
        const OpTemplate &tpl = coll[tplIdx];
        const unsigned cntIn = tpl.inShapes()[fpIdx].size();
        if (!cntIn)
            continue;

        // assign variable for the input container shape
        const THeapIdent inHeap = fm.matchedHeaps.front();
        TShapeIdent inShape;
        if (!singleShapeByHeapIdent(&inShape, inHeap, progState))
            return false;

        todo.push(inShape);
    }

    // traverse container shape edges
    while (!todo.empty()) {
        const TShapeIdent now = todo.top();
        todo.pop();

        // find predecessor shapes
        TShapeIdentList prevShapes;
        findPredecessors(&prevShapes, now, progState);
        const unsigned prevShapesCnt = prevShapes.size();
        if (!prevShapesCnt)
            // no predecessor shapes!
            return false;

        const THeapIdent &heap = now.first;
        const TLocIdx locNow = heap.first;
        const LocalState &locState = progState[locNow];
        const TTraceEdgeList &inEdgeList = locState.traceInEdges[heap.second];
        if (prevShapesCnt != inEdgeList.size())
            // incomplete shape mapping along trace edges!
            return false;

        CL_BREAK_IF(!hasKey(pMap, now));
        const TShapeVarId varNow = (*pMap)[now];

        // look ahead
        TShapeVarByLoc varByLoc;
        propagateVarsLookAhead(&varByLoc, *pMap, varNow, locNow, progState);

        // propagate current var backwards
        BOOST_FOREACH(const TShapeIdent &prev, prevShapes) {
            if (hasKey(pMap, prev))
                // var already assigned to prev
                continue;

            const THeapIdent prevHeap = prev.first;
            const TLocIdx prevLoc = prevHeap.first;
            const TShapeVarByLoc::const_iterator it = varByLoc.find(prevLoc);
            const TShapeVarId var = (varByLoc.end() == it)
                ? varNow
                : it->second;

            // propagate and schedule for processing
            (*pMap)[prev] = var;
            todo.push(prev);
        }
    }

    return /* success */ true;
}

bool assignShapeVariables(
        TShapeVarByShape           *pDst,
        const TMatchList           &matchList,
        const OpCollection         &coll,
        const TProgState           &progState)
{
    TOpList opList;
    if (!collectOpList(&opList, matchList))
        return false;

    if (!assignOpPorts(pDst, matchList, coll, progState, opList))
        return false;

    if (!propagateVars(pDst, matchList, coll, progState))
        return false;

    // TODO
    return false;
}

} // namespace AdtOp
