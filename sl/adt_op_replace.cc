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
#include "symutil.hh"

#include <cl/storage.hh>
#include <cl/cldebug.hh>
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
using FixedPoint::THeapIdx;
using FixedPoint::TInsn;
using FixedPoint::TLocIdx;
using FixedPoint::TShapeIdx;
using FixedPoint::TextInsn;

typedef std::vector<cl_uid_t>                       TPtrVarList;
typedef std::vector<TShapeVarId>                    TShapeVarList;
typedef std::set<TShapeVarId>                       TShapeVarSet;

TBoolVarId acquireFreshBoolVar(void)
{
    static TBoolVarId last;
    return ++last;
}

void initBackTrace(SymBackTrace *bt, TStorRef stor)
{
    // XXX: assume main() as the only fnc
    using namespace CodeStorage;
    const NameDb::TNameMap &glNames = stor.fncNames.glNames;
    const NameDb::TNameMap::const_iterator iter = glNames.find("main");
    if (glNames.end() != iter)
        bt->pushCall(iter->second, /* loc */ 0);
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

        TStorRef stor = sh.stor();
        const TSizeOf psize = stor.types.dataPtrSizeof();

        SymBackTrace bt(stor);
        initBackTrace(&bt, stor);

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

                CL_WARN("[ADT] possible field clash detected in checkIndependency()");
                // TODO
#if 0
                return false;
#endif
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
        if (!sh.isValid(obj))
            // the object we are looking for does not exist (partial match?)
            continue;

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
        BOOST_FOREACH(const cl_uid_t uid, *pDst) {
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
    TStorRef stor = anySymHeap->stor();
    std::string str;

    BOOST_FOREACH(const cl_uid_t uid, ptrVarList) {
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

    // XXX: protect destructive container operations from being removed as dead
    // code in case their result consists of dead variables only
    (void) varsToString(cOut, &live);

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
typedef std::set<TLocIdx>                           TLocSet;

enum ECondBranch {
    CB_TRUE,                        ///< true branch
    CB_FALSE,                       ///< false branch
    CB_TOTAL
};

typedef THeapIdentSet                               THeapsByBranch[CB_TOTAL];

struct TCondData {
    TLocIdx                         dstLoc;
    THeapsByBranch                  heapsByBranch;
};

typedef std::map<TLocIdx /* dst */, TCondData>      TCondDataMap;
typedef std::stack<TLocIdx>                         TLocStack;

struct CondReplaceCtx {
    const TProgState               &progState;
    const TShapeVarByShape         &varMap;
    const TLocSet                  &locsInOps;
    TBoolVarId                      condVar;
    TCondDataMap                    data;
    TLocStack                       todo;
    RecordRewriter                  writer;

    CondReplaceCtx(
            const TProgState       &progState_,
            const TShapeVarByShape &varMap_,
            const TLocSet          &locsInOps_,
            const TBoolVarId        condVar_):
        progState(progState_),
        varMap(varMap_),
        locsInOps(locsInOps_),
        condVar(condVar_)
    {
    }
};

/// return true if the sets are disjoint and their union contains all heaps
bool crHeapsCovered(
        const TCondData            &locData,
        const TProgState           &progState,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    THeapIdentSet all;
    const TLocIdx locDst = locData.dstLoc;
    if (-1 == locDst) {
        // collect all heaps at location 'loc'
        const LocalState &locStateSrc = progState[loc];
        const THeapIdx cnt = locStateSrc.heapList.size();
        for (THeapIdx idx = 0; idx < cnt; ++idx) {
            const THeapIdent heap(loc, idx);
            all.insert(heap);
        }
    }
    else {
        // follow all trace edges going to from loc to locDst
        const LocalState &locStateDst = progState[locDst];
        BOOST_FOREACH(const TTraceEdgeList &trEdges, locStateDst.traceInEdges) {
            BOOST_FOREACH(const TraceEdge *te, trEdges) {
                const THeapIdent &src = te->src;
                if (src./* loc */first == loc)
                    all.insert(src);
                }
        }
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

bool inferNonEmpty(
        TShapeVarId                *pShapeVar,
        CondReplaceCtx             &ctx,
        const THeapIdentSet        &heapSet)
{
    if (heapSet.empty())
        // if the set of heaps is empty, we cannot determine the shape var
        return false;

    TShapeVarId var = -1;

    BOOST_FOREACH(const THeapIdent &heap, heapSet) {
        const TShapeIdent shape(heap, /* XXX */ 0);
        const TShapeVarByShape::const_iterator it = ctx.varMap.find(shape);
        if (it == ctx.varMap.end())
            return false;

        if (-1 == var)
            var = it->second;
        else if (var != it->second)
            return false;
    }

    *pShapeVar = var;
    return true;
}

bool inferEmpty(
        CondReplaceCtx             &ctx,
        const THeapIdentSet        &heapSet)
{
    BOOST_FOREACH(const THeapIdent &heap, heapSet) {
        const TShapeIdent shape(heap, /* XXX */ 0);
        if (hasKey(ctx.varMap, shape))
            return false;
    }

    return true;
}

bool insertEmpChk(
        CondReplaceCtx             &ctx,
        const THeapsByBranch       &heapsByBranch,
        const TLocIdx               srcLoc,
        const TLocIdx               dstLoc)
{
    using namespace FixedPoint;

    TShapeVarId var;
    bool neg;

    if (inferNonEmpty(&var, ctx, heapsByBranch[CB_TRUE]))
        neg = true;
    else if (inferNonEmpty(&var, ctx, heapsByBranch[CB_FALSE]))
        neg = false;
    else
        return false;

    const THeapIdentSet &emptySet = heapsByBranch[(neg) ? CB_FALSE : CB_TRUE];
    if (!inferEmpty(ctx, emptySet))
        return false;

    std::ostringstream str;
    str << "cond" << ctx.condVar << " := ";
    if (neg)
        str << "!";
    str << "empty(C" << var << ")";

    // insert assignment of true/false to the condition variable
    TGenericVarSet live, kill;
    kill.insert(GenericVar(VL_COND_VAR, ctx.condVar));
    GenericInsn *insn = new TextInsn(str.str(), live, kill);
    ctx.writer.insertInsn(srcLoc, dstLoc, insn);
    return true;
}

bool crResolveLoc(
        CondReplaceCtx             &ctx,
        TCondData                  &locData,
        const TLocIdx               srcLoc)
{
    const TLocIdx dstLoc = locData.dstLoc;
    if (-1 == dstLoc)
        return false;

    if (hasKey(ctx.locsInOps, srcLoc) && hasKey(ctx.locsInOps, dstLoc))
        // we are in the middle of an operation
        return false;

    if (insertEmpChk(ctx, ctx.data[dstLoc].heapsByBranch, srcLoc, dstLoc))
        return true;

    // TODO: insert other condition-replace hooks here

    return false;
}

bool crHandleLoc(
        CondReplaceCtx             &ctx,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    TCondData &locData = ctx.data[loc];
    if (!crHeapsCovered(locData, ctx.progState, loc))
        // we have already failed
        return false;

    const TLocIdx dst = locData.dstLoc;
    if (-1 == dst)
        goto skip_replace;

    if (crResolveLoc(ctx, locData, loc))
        // already resolved
        return true;

    for (unsigned br = CB_TRUE; br < CB_TOTAL; ++br) {
        if (!locData.heapsByBranch[br].empty())
            continue;

        // no heaps heading to the true/false branch ==> resolved trivially
        std::ostringstream str;
        str << "cond" << ctx.condVar << " := ";
        if (CB_TRUE == br)
            str << "false";
        else
            str << "true";

        // insert assignment of true/false to the condition variable
        TGenericVarSet live, kill;
        kill.insert(GenericVar(VL_COND_VAR, ctx.condVar));
        GenericInsn *insn = new TextInsn(str.str(), live, kill);
        ctx.writer.insertInsn(loc, dst, insn);
        return true;
    }

skip_replace:
    // expand predecessors
    const LocalState &locState = ctx.progState[loc];
    return crExpandLoc(ctx, locState, loc);
}

bool tryReplaceCond(
        TInsnWriter                *pInsnWriter,
        const TShapeVarByShape     &varMap,
        const TProgState           &progState,
        const TLocSet              &locsInOps,
        const LocalState           &locState,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    // initialize condition replacement context
    const TBoolVarId condVar = acquireFreshBoolVar();
    CondReplaceCtx ctx(progState, varMap, locsInOps, condVar);

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
    AnnotatedInsn *oldInsn = static_cast<AnnotatedInsn *>(locState.insn);
    GenericInsn *insn = new TextInsn(str.str(), live, oldInsn->killVars());
    pInsnWriter->replaceInsn(loc, insn);

    return /* success */ true;
}

bool shapeByIter(
        TShapeIdx                  *pShapeIdx,
        ShapeProps                 *pProps,
        const SymHeap              &shOrig,
        const struct cl_operand    &opSrc,
        const TShapeList           &shapeList)
{
    // resolve the target object
    SymHeap sh(shOrig);
    const int varSrc = varIdFromOperand(&opSrc);
    const CVar cvSrc(varSrc, /* assume main() */ 1);
    const TObjId var = sh.regionByVar(cvSrc, /* createIfNeeded */ false);
    const TValId val = valOfPtr(sh, var, /* off */ 0);
    const TObjId obj = sh.objByAddr(val);
    if (!sh.isValid(obj))
        // invalid target
        return false;

    const TShapeIdx shapeCnt = shapeList.size();
    for (TShapeIdx shapeIdx = 0; shapeIdx < shapeCnt; ++shapeIdx) {
        const Shape &shape = shapeList[shapeIdx];
        TObjSet shapeObjs;
        objSetByShape(&shapeObjs, sh, shape);
        if (!hasKey(shapeObjs, obj))
            continue;

        *pShapeIdx = shapeIdx;
        *pProps = shape.props;
        return true;
    }

    return /* not found */ false;
}

void replaceIter(
        TInsnWriter                *pInsnWriter,
        TStorRef                    stor,
        const TLocIdx               loc,
        const char                 *name,
        const TShapeVarId           var,
        const struct cl_operand    &opSrc,
        const struct cl_operand    &opDst)
{
    using namespace FixedPoint;

    const int varSrc = varIdFromOperand(&opSrc);
    const GenericVar varDst(VL_CODE_LISTENER, varIdFromOperand(&opDst));

    std::ostringstream str;
    str << opDst << " := " << name << "(C" << var << ", "
        << varToString(stor, varSrc) << ")";

    TGenericVarSet live, kill;
    live.insert(GenericVar(VL_CONTAINER_VAR, var));
    live.insert(GenericVar(VL_CODE_LISTENER, varSrc));
    if (opDst.accessor)
        // safe over-approximation (we ignore the way how varDst is used)
        live.insert(varDst);
    else
        // trivial access to a variable (safe to notify dead code killer)
        kill.insert(varDst);

    GenericInsn *insn = new TextInsn(str.str(), live, kill);
    pInsnWriter->replaceInsn(loc, insn);
}

bool tryReplaceIter(
        TInsnWriter                *pInsnWriter,
        const TShapeVarByShape     &varMap,
        const LocalState           &locState,
        const TLocIdx               loc)
{
    using namespace FixedPoint;

    const TInsn insn = locState.insn->clInsn();
    CL_BREAK_IF(insn->code != CL_INSN_UNOP);

    const struct cl_operand &opDst = insn->operands[0];
    if (CL_OPERAND_VAR != opDst.code || !isDataPtr(opDst.type))
        return false;

    const struct cl_operand &opSrc = insn->operands[1];
    if (CL_OPERAND_VAR != opSrc.code || !isDataPtr(opSrc.type))
        return false;

    const struct cl_accessor *acSrc = opSrc.accessor;
    if (!acSrc || CL_ACCESSOR_DEREF != acSrc->code)
        // unsupported use of accessors
        return false;

    acSrc = acSrc->next;
    if (!acSrc || CL_ACCESSOR_ITEM != acSrc->code || acSrc->next)
        // unsupported access to member
        return false;

    TOffset off = acSrc->type->items[acSrc->data.item.id].offset;
    TShapeVarId var = -1;
    ShapeProps props;

    const THeapIdx heapCnt = locState.heapList.size();
    if (!heapCnt)
        // no program configurations captured for this location
        return false;

    const CodeStorage::Storage *pStor = 0;

    for (THeapIdx heapIdx = 0; heapIdx < heapCnt; ++heapIdx) {
        const SymHeap &sh = locState.heapList[heapIdx];
        if (!pStor)
            // store a point to CodeStorage::Storage
            pStor = &sh.stor();

        const TShapeList &csList = locState.shapeListByHeapIdx[heapIdx];
        TShapeIdx shapeIdx;
        ShapeProps propsNow;
        if (!shapeByIter(&shapeIdx, &propsNow, sh, opSrc, csList))
            return false;

        const TShapeIdent shapeNow(THeapIdent(loc, heapIdx), shapeIdx);

        const TShapeVarByShape::const_iterator it = varMap.find(shapeNow);
        if (it == varMap.end())
            return false;

        const TShapeVarId varNow = it->second;
        if (!heapIdx) {
            var = varNow;
            props = propsNow;
        }
        else if (var != varNow || props != propsNow)
            return false;
    }

    // FIXME: we should first check that the HEAD is really an embedded struct
    off += props.bOff.head;

    const char *name;
    if (off == props.bOff.next)
        name = "next";
    else if (off == props.bOff.prev)
        name = "prev";
    else
        // offset does not match next/prev pointer
        return false;

    replaceIter(pInsnWriter, *pStor, loc, name, var, opSrc, opDst);
    return /* success */ true;
}

bool checkBound(
        TShapeIdx                  *pShapeIdx,
        ETargetSpecifier           *pTs,
        const SymHeap              &shOrig,
        const struct cl_operand    &opSrc,
        const TShapeList           &shapeList)
{
    // resolve the value of opSrc
    SymHeap sh(shOrig);
    SymBackTrace bt(sh.stor());
    initBackTrace(&bt, sh.stor());
    SymProc proc(sh, &bt);
    const TValId val = proc.valFromOperand(opSrc);
    if (val == VAL_NULL) {
        // possibly the empty variant where first()/last() is NULL
        *pShapeIdx = -1;
        *pTs = TS_REGION;
        return true;
    }

    const ETargetSpecifier ts = sh.targetSpec(val);
    const TObjId obj = sh.objByAddr(val);
    CL_BREAK_IF(!sh.isValid(obj));

    const TShapeIdx shapeCnt = shapeList.size();
    for (TShapeIdx shapeIdx = 0; shapeIdx < shapeCnt; ++shapeIdx) {
        const Shape &shape = shapeList[shapeIdx];
        TObjList shapeObjs;
        objListByShape(&shapeObjs, sh, shape);

        const unsigned objCnt = shapeObjs.size();
        switch (objCnt) {
            case 0:
                CL_BREAK_IF("no objects in container shape");
                return false;

            case 1:
                if (obj == shapeObjs.front() && TS_REGION == ts) {
                    *pTs = TS_REGION;
                    break;
                }
                // fall through!

            default:
                if (obj == shapeObjs.front() && canPointToFront(ts))
                    *pTs = TS_FIRST;
                else if (obj == shapeObjs.back() && canPointToBack(ts))
                    *pTs = TS_LAST;
                else
                    continue;
        }

        *pShapeIdx = shapeIdx;
        return true;
    }

    return /* not found */ false;
}

void replaceBound(
        TInsnWriter                *pInsnWriter,
        const TLocIdx               loc,
        const char                 *name,
        const TShapeVarId           var,
        const struct cl_operand    &opDst)
{
    using namespace FixedPoint;
    std::ostringstream str;
    str << opDst << " := " << name << "(C" << var << ")";

    TGenericVarSet live, kill;
    live.insert(GenericVar(VL_CONTAINER_VAR, var));
    const GenericVar varDst(VL_CODE_LISTENER, varIdFromOperand(&opDst));
    if (opDst.accessor)
        // safe over-approximation (we ignore the way how varDst is used)
        live.insert(varDst);
    else
        // trivial access to a variable (safe to notify dead code killer)
        kill.insert(varDst);

    GenericInsn *insn = new TextInsn(str.str(), live, kill);
    pInsnWriter->replaceInsn(loc, insn);
}

bool tryReplaceBound(
        TInsnWriter                *pInsnWriter,
        const TShapeVarByShape     &varMap,
        const LocalState           &locState,
        const TLocIdx               loc)
{
    const TInsn insn = locState.insn->clInsn();
    CL_BREAK_IF(insn->code != CL_INSN_UNOP || insn->subCode != CL_UNOP_ASSIGN);

    const struct cl_operand &opDst = insn->operands[0];
    const struct cl_operand &opSrc = insn->operands[1];
    if (!isDataPtr(opDst.type) || !isDataPtr(opSrc.type))
        // not a pointer assignment
        return false;

    TShapeVarId var = -1;
    ETargetSpecifier ts = TS_INVALID;
    const THeapIdx heapCnt = locState.heapList.size();
    for (THeapIdx heapIdx = 0; heapIdx < heapCnt; ++heapIdx) {
        const SymHeap &sh = locState.heapList[heapIdx];
        const TShapeList &csList = locState.shapeListByHeapIdx[heapIdx];
        TShapeIdx shapeIdx;
        ETargetSpecifier tsNow;
        if (!checkBound(&shapeIdx, &tsNow, sh, opSrc, csList))
            return false;

        if (TS_REGION == tsNow && -1 == shapeIdx && csList.empty())
            // the empty variant where first()/last() is NULL
            continue;

        const TShapeIdent shapeNow(THeapIdent(loc, heapIdx), shapeIdx);

        const TShapeVarByShape::const_iterator it = varMap.find(shapeNow);
        if (it == varMap.end())
            return false;

        const TShapeVarId varNow = it->second;
        if (-1 == var)
            var = varNow;
        else if (var != varNow)
            // container varibale mismatch
            return false;

        if (TS_REGION == tsNow)
            // so far it could be either begin() or end()
            continue;

        if (TS_INVALID == ts)
            ts = tsNow;
        else if (ts != tsNow)
            // begin() vs. end()
            return false;
    }

    const char *name;
    switch (ts) {
        case TS_FIRST:
            name = "first";
            break;

        case TS_LAST:
            name = "last";
            break;

        default:
            return false;
    }

    replaceBound(pInsnWriter, loc, name, var, opDst);
    return /* success */ true;
}

void collectLocsInOps(
        TLocSet                    *pDst,
        const TMatchList           &matchList,
        const TOpList              &opList)
{
    BOOST_FOREACH(const TMatchIdxList &idxList, opList) {
        BOOST_FOREACH(const TMatchIdx idx, idxList) {
            const FootprintMatch &fm = matchList[idx];
            const THeapIdentSeq &hList = fm.matchedHeaps;
            typedef THeapIdentSeq::const_reverse_iterator TIter;
            for (TIter it = ++hList.rbegin(); it != hList.rend(); ++it)
                pDst->insert(it->/* loc */first);

            BOOST_FOREACH(const THeapIdent &heap, fm.skippedHeaps)
                pDst->insert(heap./* loc */first);
        }
    }
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

    TLocSet locsInOps;
    collectLocsInOps(&locsInOps, matchList, opList);

    const TLocIdx locCnt = progState.size();
    for (TLocIdx locIdx = 0; locIdx < locCnt; ++locIdx) {
        const LocalState &locState = progState[locIdx];
        const unsigned outEdgesCnt = locState.cfgOutEdges.size();
        switch (outEdgesCnt) {
            case 1U:
                if (locState.insn) {
                    const TInsn insn = locState.insn->clInsn();
                    if (!insn || CL_INSN_UNOP != insn->code)
                        // not a native unary operation
                        continue;

                    if (CL_UNOP_ASSIGN != insn->subCode)
                        // not an assignment instruction
                        continue;

                    if (hasKey(locsInOps, locIdx))
                        // cannot iterate in the "middle" of an operation
                        continue;

                    if (!tryReplaceIter(pInsnWriter, varMap, locState, locIdx))
                        tryReplaceBound(pInsnWriter, varMap, locState, locIdx);
                }
                continue;

            case 2U:
                // assume CL_INSN_COND
                tryReplaceCond(pInsnWriter, varMap, progState, locsInOps,
                        locState, locIdx);
                continue;

            default:
                continue;
        }
    }

    return /* success */ true;
}

} // namespace AdtOp
