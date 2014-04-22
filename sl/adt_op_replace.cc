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

#include "adt_op_replace.hh"

#include "fixed_point_rewrite.hh"
#include "symproc.hh"
#include "symtrace.hh"

#include <cl/storage.hh>
#include <cl/clutil.hh>
#include <cl/cl_msg.hh>

#include <boost/foreach.hpp>

namespace AdtOp {

using FixedPoint::GenericInsn;
using FixedPoint::LocalState;
using FixedPoint::RecordRewriter;
using FixedPoint::TBoolVarId;
using FixedPoint::TGenericVarSet;
using FixedPoint::THeapIdent;
using FixedPoint::TInsn;
using FixedPoint::TLocIdx;
using FixedPoint::TextInsn;

typedef std::vector<int /* uid */>                  TPtrVarList;
typedef std::vector<TShapeVarId>                    TShapeVarList;
typedef std::set<TShapeVarId>                       TShapeVarSet;

TBoolVarId acquireFreshBoolVar(void)
{
    static TBoolVarId last;
    return ++last;
}

bool checkIndependency(
        const FootprintMatch       &fm,
        const TProgState           &progState)
{
    using namespace FixedPoint;

    const BindingOff &bOff = fm.props.bOff;
    std::vector<TOffset> offs;
    offs.push_back(bOff.next);
    offs.push_back(bOff.prev);

    BOOST_FOREACH(const THeapIdent heap, fm.skippedHeaps) {
        const LocalState &locState = progState[heap.first];
        const TInsn insn = locState.insn->clInsn();
        SymHeap sh = locState.heapList[heap.second];
        Trace::waiveCloneOperation(sh);

        const TStorRef stor = sh.stor();
        const TSizeOf psize = stor.types.dataPtrSizeof();

        // XXX: assume main() as the only fnc
        SymBackTrace bt(stor);
        using namespace CodeStorage;
        const NameDb::TNameMap &glNames = stor.fncNames.glNames;
        const NameDb::TNameMap::const_iterator iter = glNames.find("main");
        if (glNames.end() != iter)
            bt.pushCall(iter->second, /* loc */ 0);

        SymProc proc(sh, &bt);
        proc.setLocation(&insn->loc);

        BOOST_FOREACH(const struct cl_operand &op, insn->operands) {
            if (CL_OPERAND_VAR != op.code)
                continue;

            const FldHandle fld = proc.fldByOperand(op);
            if (!fld.isValidHandle())
                continue;

            const EStorageClass sc = sh.objStorClass(fld.obj());
            if (isProgramVar(sc) && !op.accessor)
                // access to program variable with no dereference
                continue;

            const TOffset winLo = fld.offset();
            const TOffset winHi = winLo + fld.type()->size;

            BOOST_FOREACH(const TOffset lo, offs) {
                const TOffset hi = lo + psize;
                if (winHi <= lo)
                    continue;

                if (hi <= winLo)
                    continue;

                CL_WARN("[ADT] field clash detected in checkIndependency()");
                return false;
            }
        }
    }

    return /* success */ true;
}

bool findLocToReplace(
        TLocIdx                    *pDst,
        const TMatchList           &matchList,
        const TMatchIdxList        &idxList)
{
    const int idxCnt = idxList.size();
    CL_BREAK_IF(!idxCnt);

    const TMatchIdx refMatchIdx = idxList[0];
    const THeapIdentSeq &refHeaps = matchList[refMatchIdx].matchedHeaps;
    const TLocIdx srcLoc = refHeaps.front().first;
    for (int idx = 1; idx < idxCnt; ++idx) {
        const TMatchIdx matchIdx = idxList[idx];
        const THeapIdentSeq &curHeaps = matchList[matchIdx].matchedHeaps;
        if (srcLoc != curHeaps.front().first)
            goto src_port_mismatch;
    }

    *pDst = srcLoc;
    return true;

src_port_mismatch:
    const TLocIdx dstLoc = (++refHeaps.rbegin())->first;
    for (int idx = 1; idx < idxCnt; ++idx) {
        const TMatchIdx matchIdx = idxList[idx];
        const THeapIdentSeq &curHeaps = matchList[matchIdx].matchedHeaps;
        if (dstLoc != (++curHeaps.rbegin())->first)
            return false;
    }

    *pDst = dstLoc;
    return true;
}

void findShapeVarsInUseCore(
        TShapeVarList              *pDst,
        TShapeVarSet               *pSet,
        const THeapIdent           &heap,
        const TShapeVarByShape     &varMap)
{
    const TShapeIdent shape(heap, /* TODO */ 0);
    const TShapeVarByShape::const_iterator it = varMap.find(shape);
    if (it == varMap.end())
        // no shape var found for this shape
        return;

    const TShapeVarId var = it->second;
    if (insertOnce(*pSet, var))
        pDst->push_back(var);
}


bool findShapeVarsInUse(
        TShapeVarList              *pIn,
        TShapeVarList              *pOut,
        const TMatchList           &matchList,
        const TMatchIdxList        &idxList,
        const TShapeVarByShape     &varMap)
{
    TShapeVarSet inSet, outSet;

    BOOST_FOREACH(const TMatchIdx idx, idxList) {
        const FootprintMatch &fm = matchList[idx];
        const THeapIdentSeq &heaps = fm.matchedHeaps;

        findShapeVarsInUseCore(pIn,  &inSet,  heaps.front(), varMap);
        findShapeVarsInUseCore(pOut, &outSet, heaps.back(),  varMap);
    }

    return /* success */ true;
}

std::string varsToString(const TShapeVarList &vars, TGenericVarSet *pSet)
{
    const unsigned cnt = vars.size();
    if (!cnt)
        return "";

    using namespace FixedPoint;
    std::ostringstream str;
    TShapeVarId var = vars.front();
    pSet->insert(GenericVar(VL_CONTAINER_VAR, var));
    str << "C" << var;

    for (unsigned i = 1U; i < cnt; ++i) {
        var = vars[i];
        pSet->insert(GenericVar(VL_CONTAINER_VAR, var));
        str << ", C" << var;
    }

    return str.str();
}

void collectArgObjs(
        TObjList                   *pObjList,
        const FootprintMatch       &fm,
        const OpTemplate           &tpl,
        const EFootprintPort        port)
{
    const TFootprintIdx fpIdx = fm.footprint.second;
    const OpFootprint &fp = tpl[fpIdx];
    const TObjList tplObjs = (FP_SRC == port)
        ? fp.inArgs
        : fp.outArgs;

    TObjSet progObjs;
    project<D_LEFT_TO_RIGHT>(fm.objMap[port], &progObjs, tplObjs);
    BOOST_FOREACH(const TObjId obj, progObjs)
        pObjList->push_back(obj);
}

void collectPtrVarsCore(
        TPtrVarList                *pDst,
        const FootprintMatch       &fm,
        const OpTemplate           &tpl,
        const TProgState           &progState,
        const EFootprintPort        port)
{
    TObjList objList;
    collectArgObjs(&objList, fm, tpl, port);
    if (1U != objList.size())
        // unsupported number of objects not in container shape
        return;

    const THeapIdent &heap = (FP_SRC == port)
        ? fm.matchedHeaps.front()
        : fm.matchedHeaps.back();
    const SymHeap *shOrig = heapByIdent(progState, heap);
    SymHeap sh(*shOrig);
    Trace::waiveCloneOperation(sh);

    BOOST_FOREACH(const TObjId obj, objList) {
        FldList refs;
        sh.pointedBy(refs, obj);
        BOOST_FOREACH(const FldHandle &fld, refs) {
            const TObjId refObj = fld.obj();
            const TSizeRange size = sh.objSize(refObj);
            if (size.hi != fld.type()->size)
                // not an atomic variable
                continue;

            const EStorageClass sc = sh.objStorClass(refObj);
            if (!isProgramVar(sc))
                // the referee is not a program variable
                continue;

            const CVar cv = sh.cVarByObject(refObj);
            pDst->push_back(cv.uid);
        }
    }
}

void collectPtrVars(
        TPtrVarList                *pDst,
        const TMatchList           &matchList,
        const TMatchIdxList        &idxList,
        const OpTemplate           &tpl,
        const TProgState           &progState,
        const EFootprintPort        port)
{
    CL_BREAK_IF(!pDst->empty());
    collectPtrVarsCore(pDst, matchList[idxList.front()], tpl, progState, port);

    const int idxCnt = idxList.size();
    for (int idx = 1; idx < idxCnt; ++idx) {
        if (pDst->empty())
            // empty intersection already
            return;

        const TMatchIdx matchIdx = idxList[idx];
        const FootprintMatch &fm = matchList[matchIdx];
        TPtrVarList varsNow, intersect;

        collectPtrVarsCore(&varsNow, fm, tpl, progState, port);
        BOOST_FOREACH(const int uid, *pDst) {
            if (varsNow.end() == std::find(varsNow.begin(), varsNow.end(), uid))
                // uid not in the intersection
                continue;

            intersect.push_back(uid);
        }

        pDst->swap(intersect);
    }

    if (1U < pDst->size())
        pDst->resize(1U);
}

std::string ptrVarsToString(
        const TMatchList           &matchList,
        const TMatchIdxList        &idxList,
        const OpTemplate           &tpl,
        const TProgState           &progState,
        const EFootprintPort        port,
        TGenericVarSet             *pSet)
{
    using namespace FixedPoint;

    TPtrVarList ptrVarList;
    collectPtrVars(&ptrVarList, matchList, idxList, tpl, progState, port);
    if (ptrVarList.empty())
        return "";

    const THeapIdentSeq &heapSeq = matchList[0].matchedHeaps;
    const THeapIdent &anyHeapIdent = (FP_SRC == port)
        ? heapSeq.front()
        : heapSeq.back();
    const SymHeap *anySymHeap = heapByIdent(progState, anyHeapIdent);
    const TStorRef stor = anySymHeap->stor();
    std::string str;

    BOOST_FOREACH(const int uid, ptrVarList) {
        str += ", ";
        str += varToString(stor, uid);
        pSet->insert(GenericVar(VL_CODE_LISTENER, uid));
    }

    return str;
}

bool replaceSingleOp(
        TInsnWriter                *pInsnWriter,
        const TMatchList           &matchList,
        const TMatchIdxList        &idxList,
        const TShapeVarByShape     &varMap,
        const OpTemplate           &tpl,
        const TProgState           &progState)
{
    BOOST_FOREACH(const TMatchIdx idx, idxList)
        if (!checkIndependency(matchList[idx], progState))
            return false;

    TLocIdx locToReplace;
    if (!findLocToReplace(&locToReplace, matchList, idxList))
        return false;

    TShapeVarList cIn, cOut;
    if (!findShapeVarsInUse(&cIn, &cOut, matchList, idxList, varMap))
        return false;

    std::ostringstream str;
    TGenericVarSet live, kill;
    str << "(" << varsToString(cOut, &kill)
        << ptrVarsToString(matchList, idxList, tpl, progState, FP_DST, &kill)
        << ") := " << tpl.name()
        << "(" << varsToString(cIn, &live)
        << ptrVarsToString(matchList, idxList, tpl, progState, FP_SRC, &live)
        << ")";

    GenericInsn *insn = new TextInsn(str.str(), live, kill);
    pInsnWriter->replaceInsn(locToReplace, insn);

    std::set<TLocIdx> removed;
    BOOST_FOREACH(const TMatchIdx idx, idxList) {
        const FootprintMatch &fm = matchList[idx];

        // skip the last heap that does not represent any insn
        const THeapIdentSeq &hList = fm.matchedHeaps;
        typedef THeapIdentSeq::const_reverse_iterator TIter;
        for (TIter it = ++hList.rbegin(); it != hList.rend(); ++it) {
            const TLocIdx loc = it->first;
            if (loc == locToReplace)
                // already replaced by the container operation
                continue;

            if (!insertOnce(removed, loc))
                // already removed
                continue;

            pInsnWriter->dropInsn(loc);
        }
    }

    return /* success */ true;
}

typedef std::set<THeapIdent>                        THeapIdentSet;

enum ECondBranch {
    CB_TRUE,                        ///< true branch
    CB_FALSE,                       ///< false branch
    CB_TOTAL
};

struct TCondData {
    TLocIdx                         dstLoc;
    THeapIdentSet                   heapsByBranch[CB_TOTAL];
};

typedef std::map<TLocIdx /* dst */, TCondData>      TCondDataMap;
typedef std::stack<TLocIdx>                         TLocStack;

struct CondReplaceCtx {
    const TProgState               &progState;
    TBoolVarId                      condVar;
    TCondDataMap                    data;
    TLocStack                       todo;
    RecordRewriter                  writer;

    CondReplaceCtx(const TProgState &progState_, const TBoolVarId condVar_):
        progState(progState_),
        condVar(condVar_)
    {
    }
};

/// return true if the sets are disjoint and their union contains all heaps
bool crHeapsCovered(
        const TCondData            &locData,
        const LocalState           &locState,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    // collect all heaps at location 'loc'
    THeapIdentSet all;
    const THeapIdx cnt = locState.heapList.size();
    for (THeapIdx idx = 0; idx < cnt; ++idx) {
        const THeapIdent heap(loc, idx);
        all.insert(heap);
    }

    // remove them one by one while checking for dupes
    for (unsigned br = CB_TRUE; br < CB_TOTAL; ++br) {
        BOOST_FOREACH(const THeapIdent &heap, locData.heapsByBranch[br])
            if (1U != all.erase(heap))
                return false;
    }

    // in case of success, there should be no uncovered heaps
    return all.empty();
}

bool crExpandLoc(
        CondReplaceCtx             &ctx,
        const LocalState           &locState,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    const TCfgEdgeList &cfgEdges = locState.cfgInEdges;
    if (cfgEdges.empty())
        // no predecessor, something went wrong
        return false;

    BOOST_FOREACH(const CfgEdge &ce, cfgEdges) {
        const TLocIdx srcLoc = ce.targetLoc;
        if (hasKey(ctx.data, srcLoc))
            // location already processed, something went wrong
            return false;

        // schedule the predecessor location for processing
        ctx.data[srcLoc].dstLoc = loc;
        ctx.todo.push(srcLoc);
    }

    const TCondData &dstData = ctx.data[loc];

    // go through incoming trace edges
    BOOST_FOREACH(const TTraceEdgeList &trEdges, locState.traceInEdges) {
        BOOST_FOREACH(const TraceEdge *te, trEdges) {
            const THeapIdent &src = te->src;
            const THeapIdent &dst = te->dst;

            TCondData &srcData = ctx.data[src.first];

            // propagate the sets of heaps heading to the true/false branch
            for (unsigned br = CB_TRUE; br < CB_TOTAL; ++br) {
                if (hasKey(dstData.heapsByBranch[br], dst))
                    srcData.heapsByBranch[br].insert(src);
            }
        }
    }

    return true;
}

bool crHandleLoc(
        CondReplaceCtx             &ctx,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    TCondData &locData = ctx.data[loc];
    const LocalState &locState = ctx.progState[loc];
    if (!crHeapsCovered(locData, locState, loc))
        // we have already failed
        return false;

    for (unsigned br = CB_TRUE; br < CB_TOTAL; ++br) {
        if (!locData.heapsByBranch[br].empty())
            continue;

        // no heaps heading to the true/false branch ==> resolved trivially
        std::ostringstream str;
        str << "cond" << ctx.condVar << " := ";
        if (CB_TRUE == br)
            str << "true";
        else
            str << "false";

        // insert assignment of true/false to the condition variable
        TGenericVarSet live, kill;
        kill.insert(GenericVar(VL_COND_VAR, ctx.condVar));
        GenericInsn *insn = new TextInsn(str.str(), live, kill);
        ctx.writer.insertInsn(loc, locData.dstLoc, insn);
        return true;
    }

    // TODO: insert other condition-replace hooks here

    // expand predecessors
    return crExpandLoc(ctx, locState, loc);
}

bool tryReplaceCond(
        TInsnWriter                *pInsnWriter,
        const TMatchList           &matchList,
        const TOpList              &opList,
        const TShapeVarByShape     &varMap,
        const TProgState           &progState,
        const LocalState           &locState,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    // initialize condition replacement context
    const TBoolVarId condVar = acquireFreshBoolVar();
    CondReplaceCtx ctx(progState, condVar);

    // start with the location of the condition itself
    TCondData &locData = ctx.data[loc];
    locData.dstLoc = /* XXX */ -1;

    // resolve locations of the true/false targets
    const TCfgEdgeList &cfgEdges = locState.cfgOutEdges;
    CL_BREAK_IF(2U != cfgEdges.size());
    const TLocIdx tloc = cfgEdges[/* true  */ 0].targetLoc;
    const TLocIdx floc = cfgEdges[/* false */ 1].targetLoc;

    // check which heaps are heading to true/false branches
    BOOST_FOREACH(const TTraceEdgeList &trEdges, locState.traceOutEdges) {
        BOOST_FOREACH(const TraceEdge *te, trEdges) {
            const THeapIdent &src = te->src;
            const TLocIdx dstLoc = te->dst.first;
            if (dstLoc == tloc)
                locData.heapsByBranch[CB_TRUE].insert(src);
            if (dstLoc == floc)
                locData.heapsByBranch[CB_FALSE].insert(src);
        }
    }

    // handle the location of the condition itself
    if (!crHandleLoc(ctx, loc))
        return false;

    CL_BREAK_IF(ctx.todo.empty());

    // traverse the CFG looking for a proper condition replacement
    while (!ctx.todo.empty()) {
        const TLocIdx locNow = ctx.todo.top();
        ctx.todo.pop();
        if (!crHandleLoc(ctx, locNow))
            return false;
    }

    // no problem encountered ==> insert the assignments now!
    CL_BREAK_IF(ctx.writer.empty());
    ctx.writer.flush(pInsnWriter);

    // replace the original condition by the newly introduced bool variable
    std::ostringstream str;
    str << "?cond" << ctx.condVar;
    TGenericVarSet live;
    live.insert(GenericVar(VL_COND_VAR, condVar));
    AnnotatedInsn *oldInsn = dynamic_cast<AnnotatedInsn *>(locState.insn);
    GenericInsn *insn = new TextInsn(str.str(), live, oldInsn->killVars());
    pInsnWriter->replaceInsn(loc, insn);

    return /* success */ true;
}

bool replaceAdtOps(
        TInsnWriter                *pInsnWriter,
        const TMatchList           &matchList,
        const TOpList              &opList,
        const OpCollection         &adtOps,
        const TShapeVarByShape     &varMap,
        const TProgState           &progState)
{
    BOOST_FOREACH(const TMatchIdxList &idxList, opList) {
        const FootprintMatch &fm0 = matchList[idxList.front()];
        const TFootprintIdent &fp = fm0.footprint;
        const OpTemplate &tpl = adtOps[fp.first];

        if (!replaceSingleOp(pInsnWriter, matchList, idxList, varMap, tpl,
                    progState))
            return false;
    }

    const TLocIdx locCnt = progState.size();
    for (TLocIdx locIdx = 0; locIdx < locCnt; ++locIdx) {
        const LocalState &locState = progState[locIdx];
        if (2U == locState.cfgOutEdges.size())
            // assume CL_INSN_COND
            tryReplaceCond(pInsnWriter, matchList, opList, varMap, progState,
                    locState, locIdx);
    }

    return /* success */ true;
}

} // namespace AdtOp
