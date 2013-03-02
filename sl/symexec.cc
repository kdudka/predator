/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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
#include "symexec.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/memdebug.hh>
#include <cl/storage.hh>

#include "fixed_point_proxy.hh"
#include "glconf.hh"
#include "sigcatch.hh"
#include "symabstract.hh"
#include "symcall.hh"
#include "symdebug.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "util.hh"

#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>

#include <boost/foreach.hpp>

LOCAL_DEBUG_PLOTTER(nondetCond, DEBUG_SE_NONDET_COND)

bool installSignalHandlers(void)
{
    // will be processed in SymExecEngine::processPendingSignals() eventually
    return SignalCatcher::install(SIGINT)
        && SignalCatcher::install(SIGUSR1)
        && SignalCatcher::install(SIGTERM);
}

// /////////////////////////////////////////////////////////////////////////////
// ExecStack
class SymExecEngine;

struct ExecStackItem {
    SymCallCtx      *ctx;
    SymExecEngine   *eng;
    SymState        *dst;
};

typedef std::deque<ExecStackItem> TExecStack;

// /////////////////////////////////////////////////////////////////////////////
// SymExec
class SymExec: public IStatsProvider {
    public:
        SymExec(const CodeStorage::Storage &stor):
            stor_(stor),
            callCache_(stor)
        {
        }

        /// just to avoid memory leakage in case an exception falls through
        ~SymExec();

        const CodeStorage::Fnc* resolveCallInsn(
                SymState                    &results,
                SymHeap                     entry,
                const CodeStorage::Insn     &insn);

        void enterCall(SymCallCtx *ctx, SymState &results);

        void execFnc(
                SymState                    &results,
                const SymHeap               &entry,
                const CodeStorage::Insn     &insn,
                const CodeStorage::Fnc      &fnc);

        virtual void printStats() const;

    private:
        const CodeStorage::Storage              &stor_;
        SymCallCache                            callCache_;
        TExecStack                              execStack_;
};

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine
class SymExecEngine: public IStatsProvider {
    public:
        SymExecEngine(
                SymState                &results,
                const SymHeap           &entry,
                const IStatsProvider    &stats,
                SymBackTrace            &bt):
            stor_(entry.stor()),
            bt_(bt),
            dst_(results),
            stats_(stats),
            sched_(stateMap_),
            block_(0),
            insnIdx_(0),
            heapIdx_(0),
            waiting_(false),
            endReached_(false)
        {
            this->initEngine(entry);
        }

    public:
        bool /* complete */ run();

        virtual void printStats() const;

        // TODO: describe the interface briefly
        const SymHeap&                  callEntry() const;
        const CodeStorage::Insn&        callInsn() const;
        SymState&                       callResults();
        bool                            endReached() const;
        void                            forceEndReached();

    private:
        const CodeStorage::Storage      &stor_;
        SymBackTrace                    &bt_;
        SymState                        &dst_;
        const IStatsProvider            &stats_;
        std::string                     fncName_;
        TObjType                        fncReturnType_;

        SymStateMap                     stateMap_;
        BlockScheduler                  sched_;
        const CodeStorage::Block        *block_;
        unsigned                        insnIdx_;
        unsigned                        heapIdx_;
        bool                            waiting_;
        bool                            endReached_;

        SymHeapList                     localState_;
        SymHeapList                     nextLocalState_;
        SymHeapList                     callResults_;
        const struct cl_loc             *lw_;

    private:
        void initEngine(const SymHeap &init);

        void joinCallResults();

        void updateState(SymHeap &sh, const CodeStorage::Block *ofBlock);

        void updateStateInBranch(
                SymHeap                             sh,
                const bool                          branch,
                const CodeStorage::Insn             &insnCmp,
                const CodeStorage::Insn             &insnCnd,
                const TValId                        v1,
                const TValId                        v2);

        bool bypassNonPointers(
                SymProc                             &proc,
                const CodeStorage::Insn             &insnCmp,
                const CodeStorage::Insn             &insnCnd,
                const TValId                        v1,
                const TValId                        v2);

        void execJump();
        void execReturn();
        void execCondInsn();
        void execTermInsn();
        bool execNontermInsn();
        bool execInsn();
        bool execBlock();
        void processPendingSignals();
        void pruneOrigin();

        /// @param flags see DEBUG_SE_FIXED_POINT in config.h
        void dumpStateMap(int flags);

        void printStatsHelper(const BlockScheduler::TBlock bb) const;
};

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine implementation
void SymExecEngine::initEngine(const SymHeap &init)
{
    // look for fnc name
    const CodeStorage::Fnc &fnc = *bt_.topFnc();
    fncName_ = nameOf(fnc);
    lw_ = locationOf(fnc);
    CL_DEBUG_MSG(lw_, ">>> entering " << fncName_ << "()");

    const TObjType fncType = fnc.def.type;
    CL_BREAK_IF(CL_TYPE_FNC != fncType->code || fncType->item_cnt < 1);
    fncReturnType_ = fncType->items[/* ret */ 0].type;

    // look for the entry block
    const CodeStorage::Block *entry = fnc.cfg.entry();
    if (!entry) {
        CL_ERROR_MSG(lw_, fncName_ << ": " << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    stateMap_.insert(entry, init);

    // schedule the entry block for processing
    sched_.schedule(entry);
}

void SymExecEngine::execJump()
{
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TTargetList &tlist = insn->targets;

    // make a copy in case we needed to perform an abstraction
    const SymHeap &origin = localState_[heapIdx_];
    SymHeap sh(origin);
    Trace::waiveCloneOperation(sh);

    this->updateState(sh, tlist[/* target */ 0]);
}

void SymExecEngine::execReturn()
{
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &opList = insn->operands;
    CL_BREAK_IF(1 != opList.size());

    // create a local copy of the heap
    const SymHeap &origin = localState_[heapIdx_];
    SymHeap sh(origin);

    Trace::Node *trOrig = origin.traceNode();
    Trace::Node *trRet = new Trace::InsnNode(trOrig, insn, /* bin */ false);
    sh.traceUpdate(trRet);

    if (CL_TYPE_VOID != fncReturnType_->code) {
        SymProc proc(sh, &bt_);
        proc.setLocation(lw_);

        sh.objSetEstimatedType(OBJ_RETURN, fncReturnType_);

        const struct cl_operand &src = opList[0];
        if (CL_OPERAND_VOID == src.code) {
            // the function returns value, but we were given no value!
            const TSizeOf size = fncReturnType_->size;
            const CustomValue cv(IR::rngFromNum(size));
            const TValId valSize = sh.valWrapCustom(cv);
            const TValId retAddr = sh.addrOfTarget(OBJ_RETURN, TS_REGION);
            const TValId valUnknown = sh.valCreate(VT_UNKNOWN, VO_STACK);
            executeMemset(proc, retAddr, valUnknown, valSize);
        }
        else {
            const TValId val = proc.valFromOperand(src);
            const FldHandle ret(sh, OBJ_RETURN, src.type);
            CL_BREAK_IF(VAL_INVALID == val);
            proc.setValueOf(ret, val);
        }
    }

    // commit one of the function results
    dst_.insert(sh);
    endReached_ = true;
}

bool isLoopClosingEdge(
        const CodeStorage::Insn     *term,
        const CodeStorage::Block    *ofBlock)
{
    BOOST_FOREACH(const unsigned idxTarget, term->loopClosingTargets)
        if (term->targets[idxTarget] == ofBlock)
            return true;

    return false;
}

void SymExecEngine::updateState(SymHeap &sh, const CodeStorage::Block *ofBlock)
{
    const std::string &name = ofBlock->name();

    bool closingLoop = isLoopClosingEdge(/* term */ block_->back(), ofBlock);
    if (closingLoop)
        CL_DEBUG_MSG(lw_, "-L- traversing a loop-closing edge");

    // time to consider abstraction
#if SE_ABSTRACT_ON_LOOP_EDGES_ONLY
    if (closingLoop)
#endif
        abstractIfNeeded(sh);

#if !SE_JOIN_ON_LOOP_EDGES_ONLY
    closingLoop = true;
#endif

    // update _target_ state and check if anything has changed
    if (stateMap_.insert(ofBlock, sh, closingLoop)) {
        const SymStateMarked &target = stateMap_[ofBlock];

        // schedule for next wheel (if not already)
        sched_.schedule(ofBlock);

        CL_DEBUG_MSG(lw_, "+++ block " << name << " updated: "
                << target.cntPending() << " heaps pending, "
                << target.size() << " heaps total");
    }
    else {
        CL_DEBUG_MSG(lw_, "--- block " << name << " left intact");
    }
}

bool isAnyAbstractOf(const SymHeap &sh, const TValId v1, const TValId v2)
{
    const TObjId obj1 = sh.objByAddr(v1);
    const TObjId obj2 = sh.objByAddr(v2);

    return isAbstractObject(sh, obj1)
        || isAbstractObject(sh, obj2);
}

void SymExecEngine::updateStateInBranch(
        SymHeap                     shOrig,
        const bool                  branch,
        const CodeStorage::Insn    &insnCmp,
        const CodeStorage::Insn    &insnCnd,
        const TValId                v1,
        const TValId                v2)
{
    SymHeapList dst;
    SymProc procOrig(shOrig, &bt_);
    procOrig.setLocation(lw_);

    // prepare trace node for a non-deterministic condition
    Trace::waiveCloneOperation(shOrig);
    Trace::Node *trCond = new Trace::CondNode(shOrig.traceNode(),
                &insnCmp, &insnCnd, /* det */ false, branch);

#if DEBUG_SE_NONDET_COND < 2
    const bool hasAbstract = isAnyAbstractOf(shOrig, v1, v2);
#endif
    const enum cl_binop_e code = static_cast<enum cl_binop_e>(insnCmp.subCode);
    if (!reflectCmpResult(dst, procOrig, code, branch, v1, v2))
        CL_DEBUG_MSG(lw_, "XXX unable to reflect comparison result");

    CL_BREAK_IF(!dst.size());

    BOOST_FOREACH(SymHeap *sh, dst) {
        sh->traceUpdate(trCond);
#if DEBUG_SE_NONDET_COND < 2
        if (hasAbstract)
#endif
            LDP_PLOT(nondetCond, *sh);

        SymProc proc(*sh, &bt_);
        proc.setLocation(lw_);
        const unsigned targetIdx = !branch;
        proc.killInsn(insnCmp);
        proc.killPerTarget(insnCnd, targetIdx);

        const CodeStorage::Block *target = insnCnd.targets[targetIdx];
        this->updateState(*sh, target);
    }
}

bool isTrackableValue(const SymHeap &sh, const TValId val)
{
    if (isPossibleToDeref(sh, val))
        return true;

    const EValueTarget code = sh.valTarget(val);
    if (VT_UNKNOWN == code && VO_ASSIGNED == sh.valOrigin(val))
        // this allows to track results of undefined functions returning int
        return true;

    return false;
}

bool SymExecEngine::bypassNonPointers(
        SymProc                                     &proc,
        const CodeStorage::Insn                     &insnCmp,
        const CodeStorage::Insn                     &insnCnd,
        const TValId                                v1,
        const TValId                                v2)
{
    SymHeap &sh = proc.sh();
    const TObjType clt1 = insnCmp.operands[/* src1 */ 1].type;
    const TObjType clt2 = insnCmp.operands[/* src2 */ 2].type;
#if SE_TRACK_NON_POINTER_VALUES < 3
    if (isCodePtr(clt1) || isCodePtr(clt2))
        goto skip_tracking;
#endif

#if !SE_TRACK_NON_POINTER_VALUES
    if (isDataPtr(clt1) || isDataPtr(clt2))
#endif
        return false;

    // white-list some values that are worth tracking
    if (isTrackableValue(sh, v1) || isTrackableValue(sh, v2))
        return false;

skip_tracking:
    proc.killInsn(insnCmp);

    SymHeap sh1(sh);
    sh1.traceUpdate(new Trace::CondNode(sh.traceNode(),
                &insnCmp, &insnCnd, /* det */ false, /* branch */ true));

    CL_DEBUG_MSG(lw_, "-T- CL_INSN_COND updates TRUE branch");
    SymProc proc1(sh1, proc.bt());
    proc1.setLocation(proc.lw());
    proc1.killPerTarget(insnCnd, /* then label */ 0);
    this->updateState(sh1, insnCnd.targets[/* then label */ 0]);

    SymHeap sh2(sh);
    sh2.traceUpdate(new Trace::CondNode(sh.traceNode(),
                &insnCmp, &insnCnd, /* det */ false, /* branch */ false));

    CL_DEBUG_MSG(lw_, "-F- CL_INSN_COND updates FALSE branch");
    SymProc proc2(sh2, proc.bt());
    proc2.setLocation(proc.lw());
    proc2.killPerTarget(insnCnd, /* else label */ 1);
    this->updateState(sh2, insnCnd.targets[/* else label */ 1]);

    return true;
}

void SymExecEngine::execCondInsn()
{
    // we should get a CL_INSN_BINOP instruction and a CL_INSN_COND instruction
    const CodeStorage::Insn *insnCmp = block_->operator[](insnIdx_ - 1);
    const CodeStorage::Insn *insnCnd = block_->operator[](insnIdx_);
    CL_BREAK_IF(CL_INSN_BINOP != insnCmp->code);
    CL_BREAK_IF(CL_INSN_COND != insnCnd->code);

#ifndef NDEBUG
    // check that both instructions are connected accordingly
    const struct cl_operand &cmpDst = insnCmp->operands[/* dst */ 0];
    const struct cl_operand &cndSrc = insnCnd->operands[/* src */ 0];
    CL_BREAK_IF(CL_OPERAND_VAR != cmpDst.code || CL_OPERAND_VAR != cndSrc.code);
    CL_BREAK_IF(varIdFromOperand(&cmpDst) != varIdFromOperand(&cndSrc));
#endif

    // read operands
    const struct cl_operand &op1 = insnCmp->operands[/* src1 */ 1];
    const struct cl_operand &op2 = insnCmp->operands[/* src2 */ 2];
    CL_BREAK_IF(!areComparableTypes(op1.type, op2.type));

    // a working area in case of VAL_TRUE and VAL_FALSE
    SymHeap sh(localState_[heapIdx_]);
    Trace::waiveCloneOperation(sh);
    SymProc proc(sh, &bt_);
    proc.setLocation(lw_);

    // compute the result of CL_INSN_BINOP
    const enum cl_binop_e code = static_cast<enum cl_binop_e>(insnCmp->subCode);
    const TValId v1 = proc.valFromOperand(op1);
    const TValId v2 = proc.valFromOperand(op2);
    const TValId val = compareValues(sh, code, v1, v2);

    // check whether we know where to go
    switch (val) {
        case VAL_TRUE:
            sh.traceUpdate(new Trace::CondNode(sh.traceNode(),
                        insnCmp, insnCnd, /* det */ true, /* branch */ true));

            CL_DEBUG_MSG(lw_, ".T. CL_INSN_COND got VAL_TRUE");
            proc.killInsn(*insnCmp);
            proc.killPerTarget(*insnCnd, /* then label */ 0);
            this->updateState(sh, insnCnd->targets[/* then label */ 0]);
            return;

        case VAL_FALSE:
            sh.traceUpdate(new Trace::CondNode(sh.traceNode(),
                        insnCmp, insnCnd, /* det */ true, /* branch */ false));

            CL_DEBUG_MSG(lw_, ".F. CL_INSN_COND got VAL_FALSE");
            proc.killInsn(*insnCmp);
            proc.killPerTarget(*insnCnd, /* else label */ 1);
            this->updateState(sh, insnCnd->targets[/* else label */ 1]);
            return;

        default:
            // we need to update both targets
            break;
    }

    const EValueOrigin origin = sh.valOrigin(val);
    if (VO_DEREF_FAILED == origin) {
        // error should have been already emitted
        CL_DEBUG_MSG(lw_, "ignored VO_DEREF_FAILED");
        return;
    }

    if (this->bypassNonPointers(proc, *insnCmp, *insnCnd, v1, v2))
        // do not track relations over data we are not interested in
        return;

    if (isUninitialized(origin)) {
        CL_WARN_MSG(lw_, "conditional jump depends on uninitialized value");
        describeUnknownVal(proc, val, "use");
        proc.printBackTrace(ML_WARN);
    }

#if DEBUG_SE_NONDET_COND < 2
    if (isAnyAbstractOf(sh, v1, v2))
#endif
    {
        std::ostringstream str;
        str << "at-line-" << lw_->line;
        LDP_INIT(nondetCond, str.str());
        LDP_PLOT(nondetCond, sh);
    }

    CL_DEBUG_MSG(lw_, "?T? CL_INSN_COND updates TRUE branch");
    this->updateStateInBranch(sh, true,  *insnCmp, *insnCnd, v1, v2);

    CL_DEBUG_MSG(lw_, "?F? CL_INSN_COND updates FALSE branch");
    this->updateStateInBranch(sh, false, *insnCmp, *insnCnd, v1, v2);
}

void SymExecEngine::execTermInsn()
{
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_RET:
            this->execReturn();
            break;

        case CL_INSN_JMP:
            this->execJump();
            break;

        case CL_INSN_COND:
            this->execCondInsn();
            break;

        case CL_INSN_ABORT:
            CL_DEBUG_MSG(lw_, "CL_INSN_ABORT reached");
            endReached_ = true;
            break;

        default:
            CL_BREAK_IF("SymExecEngine::execTermInsn() got something special");
    }
}

bool /* handled */ SymExecEngine::execNontermInsn()
{
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // set some properties of the execution
    SymExecCoreParams ep;
    ep.trackUninit      = GlConf::data.trackUninit;
    ep.oomSimulation    = GlConf::data.oomSimulation;
    ep.skipPlot         = GlConf::data.skipUserPlots;
    ep.errLabel         = GlConf::data.errLabel;

    // working area for non-terminal instructions
    const SymHeap &origin = localState_[heapIdx_];
    SymHeap sh(origin);
    SymExecCore core(sh, &bt_, ep);
    core.setLocation(lw_);

    // drop the unnecessary Trace::CloneNode node in the trace graph
    Trace::waiveCloneOperation(sh);

    // execute the instruction
    if (!core.exec(nextLocalState_, *insn)) {
        CL_BREAK_IF(CL_INSN_CALL != insn->code);
        return false;
    }

    if (core.hasFatalError())
        // suppress the annoying warnings 'end of foo() not reached' since we
        // have already told user that there was something more serious going on
        endReached_ = true;

    return /* insn handled */ true;
}

bool /* complete */ SymExecEngine::execInsn()
{
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // true for terminal instruction
    const bool isTerm = cl_is_term_insn(insn->code);

    if (!heapIdx_) {
        CL_DEBUG_MSG(lw_, "!!! executing insn #" << insnIdx_
                << " ... " << (*insn));

        // let's begin with empty resulting heap union
        nextLocalState_.clear();
    }

    bool nextInsnIsCond = false;
    if (!isTerm) {
        CL_BREAK_IF(block_->size() <= insnIdx_);

        // look ahead for CL_INSN_COND
        const CodeStorage::Insn *nextInsn = block_->operator[](insnIdx_ + 1);
        if (CL_INSN_COND == nextInsn->code) {
            CL_BREAK_IF(CL_INSN_BINOP != insn->code);
            nextInsnIsCond = true;
        }
    }

    // used only if (0 == insnIdx_)
    SymStateMarked &origin = stateMap_[block_];

    // go through the remainder of symbolic heaps corresponding to localState_
    const unsigned hCnt = localState_.size();
    for (/* we allow resume */; heapIdx_ < hCnt; ++heapIdx_) {
        if (!insnIdx_) {
            if (origin.isDone(heapIdx_))
                // for this particular symbolic heap, we already know the result
                // and the result is already included in the resulting state,
                // skip it
                continue;

            // mark as processed now since it can be re-scheduled right away
            origin.setDone(heapIdx_);
        }

        // capture fixed-point for plotting if configured to do so
        if (GlConf::data.fixedPoint)
            GlConf::data.fixedPoint->insert(insn, localState_[heapIdx_]);

        if (nextInsnIsCond)
            // this is going to be handled in execCondInsn() right away
            continue;

        if (1 < hCnt) {
            CL_DEBUG_MSG(lw_, "*** processing block " << block_->name()
                         << ", heap #" << heapIdx_
                         << " (initial size of state was " << hCnt << ")");
        }

        // time to respond to a single pending signal
        this->processPendingSignals();

        if (isTerm) {
            // terminal insn
            this->execTermInsn();
            continue;
        }

        if (this->execNontermInsn())
            // regular heap instruction
            continue;

        // assuming CL_INSN_CALL; just suspend self and wait for results...
        ++heapIdx_;
        return false;
    }

    if (nextInsnIsCond)
        localState_.swap(nextLocalState_);

    // completed execution of the given insn
    heapIdx_ = 0;
    return true;
}

bool /* complete */ SymExecEngine::execBlock()
{
    const std::string &name = block_->name();

    if (insnIdx_ || heapIdx_) {
        // some debugging output of the resume process
        CL_DEBUG_MSG(lw_, "___ we are back in " << fncName_
                << "(), block " << name
                << ", insn #" << insnIdx_
                << ", heap #" << (heapIdx_ - 1)
                << ", " << sched_.cntWaiting()
                << " basic block(s) in the queue");
    }
    else {
        // fresh run, let's initialize the local state by the BB entry
        const SymState &origin = stateMap_[block_];
        localState_ = origin;

        // eliminate the unneeded Trace::CloneNode instances
        Trace::waiveCloneOperation(localState_);
    }

    // go through the remainder of BB insns
    for (; insnIdx_ < block_->size(); ++insnIdx_) {

        const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
        if (0 < insn->loc.line)
            // update location info
            lw_ = &insn->loc;

        // execute current instruction
        if (!this->execInsn()) {
            // function call reached, we should stand by
            callResults_.clear();
            return false;
        }

        if (!insnIdx_)
            this->pruneOrigin();

        if (!nextLocalState_.size())
            // we ended up with an empty state already, jump to the end of bb
            break;

        // swap states in order to be ready for next insn
        localState_.swap(nextLocalState_);
    }

    // the whole block is processed now
    CL_DEBUG_MSG(lw_, "___ completed batch for " << name << ", " << fncName_
                 << "(), " << sched_.cntWaiting()
                 << " basic block(s) in the queue");
    insnIdx_ = 0;
    return true;
}

void SymExecEngine::joinCallResults()
{
#if SE_ABSTRACT_ON_CALL_DONE
    SymStateWithJoin all;
#else
    SymHeapUnion all;
#endif
    all.swap(nextLocalState_);

    const unsigned cnt = callResults_.size();
    for (unsigned i = 0; i < cnt; ++i) {
        if (1 < cnt) {
            CL_DEBUG("*** SymExecEngine::joinCallResults() is processing heap #"
                     << i << " of " << cnt << " heaps total (size of target is "
                     << all.size() << ")");
        }

        // time to respond to a single pending signal
        this->processPendingSignals();

        all.insert(callResults_[i]);
    }

    all.swap(nextLocalState_);
}

bool /* complete */ SymExecEngine::run()
{
    const CodeStorage::Fnc fnc = *bt_.topFnc();

    if (waiting_) {
        // pick up results of the pending call
        this->joinCallResults();

        // we're on the way from a just completed function call...
        if (!this->execBlock())
            // ... and we've just hit another one
            return false;
    }
    else {
        // we have a fresh instance of SymExecEngine
        waiting_ = true;

        // check for possible protocol error
        CL_BREAK_IF(1 != sched_.cntWaiting());
    }

    // main loop of SymExecEngine
    while (sched_.getNext(&block_)) {
        // update location info and ptracer
        const CodeStorage::Insn *first = block_->front();
        lw_ = &first->loc;

        // enter the basic block
        const std::string &name = block_->name();
        CL_DEBUG_MSG(lw_, "___ entering " << name << ", " << fncName_
                     << "(), " << sched_.cntWaiting()
                     << " basic block(s) in the queue");
        insnIdx_ = 0;
        heapIdx_ = 0;

        // process the basic block till the first function call
        if (!this->execBlock())
            // function call reached, suspend the execution for now
            return false;
    }

    int debugFixedPoint = (DEBUG_SE_FIXED_POINT);

    const struct cl_loc *loc = locationOf(fnc);
    if (!endReached_) {
        CL_WARN_MSG(loc, "end of function "
                << nameOf(fnc) << "() has not been reached");
#if DEBUG_SE_END_NOT_REACHED
        sched_.printStats();
        debugFixedPoint |= /* plot heaps */ 0x2;
#endif
        bt_.printBackTrace();
    }

    // we are done with this function
    CL_DEBUG_MSG(loc, "<<< leaving " << nameOf(fnc) << "()");
    waiting_ = false;

    this->dumpStateMap(debugFixedPoint);

    return true;
}

void SymExecEngine::printStatsHelper(const BlockScheduler::TBlock bb) const
{
    const std::string &name = bb->name();

    // query total count of heaps
    SymExecEngine *self = const_cast<SymExecEngine *>(this);
    const SymStateMarked &state = self->stateMap_[bb];
    const unsigned total = state.size();

    // read count of the heaps pending for execution
    const unsigned waiting = state.cntPending();

    const char *status = (bb == block_)
        ? " in progress"
        : " scheduled";

    const CodeStorage::Insn *first = bb->front();
    CL_NOTE_MSG(&first->loc,
            "___ block " << name << status <<
            ", " << total << " heap(s) total"
            ", " << waiting << " heap(s) pending");
}

void SymExecEngine::printStats() const
{
    // per function statistics
    const BlockScheduler::TBlockSet &bset = sched_.todo();
    CL_NOTE_MSG(lw_,
            "... while executing " << fncName_ << "()"
            ", " << dst_.size() << " result(s) already computed"
            ", " << bset.size() << " basic block(s) in the queue"
            ", " << localState_.size() << " src heap(s)"
            ", " << nextLocalState_.size() << " dst heap(s)"
            ", insn #" << insnIdx_ <<
            ", heap #" << heapIdx_);

    if (block_)
        // print statistics for the basic block just being computed
        this->printStatsHelper(block_);

    // go through scheduled basic blocks
    BOOST_FOREACH(const BlockScheduler::TBlock bb, bset) {
        if (bb == block_)
            // already handled
            continue;

        this->printStatsHelper(bb);
    }

    // TODO: a separate compile-time option for this?
#if DEBUG_SE_END_NOT_REACHED
    // finally print the statistics provided by BlockScheduler
    sched_.printStats();
#endif
}

void SymExecEngine::dumpStateMap(int flags)
{
    if (!flags)
        return;

    if (SE_STATE_PRUNING_MODE) {
        CL_WARN("fixed-point dump poisoned by SE_STATE_PRUNING_MODE = "
                << (SE_STATE_PRUNING_MODE));
    }

    // obtain the list of visisted blocks
    const BlockScheduler::TBlockList &bbs = sched_.done();

    BOOST_FOREACH(const BlockScheduler::TBlock block, bbs) {
        const std::string name = block->name();

        SymStateWithJoin state(stateMap_[block]);

        for (unsigned idx = 0; idx < block->size(); ++idx) {
            const CodeStorage::Insn *insn = block->operator[](idx);
            const struct cl_loc *loc = &insn->loc;

            if (flags & /* print size */ 0x1) {
                CL_NOTE_MSG(loc, "block " << name << ", insn #" << idx
                        << ": count of heaps is " << state.size());
            }

            if (flags & /* plot heaps */ 0x2) {
                std::ostringstream str;
                str << "block-" << name << "-insn-" << idx << "-heap";
                BOOST_FOREACH(const SymHeap *sh, state)
                    plotHeap(*sh, str.str(), loc);
            }

            if (!(flags & /* per insn */ 0x4) || cl_is_term_insn(insn->code))
                break;

            SymStateWithJoin result;
            BOOST_FOREACH(const SymHeap *pHeap, state) {
                SymHeap sh(*pHeap);
                SymExecCore core(sh, &bt_ /* TODO: propagate params_ */);
                if (!core.exec(result, *insn))
                    CL_BREAK_IF("replay of CL_INSN_CALL not supported for now");
            }

            state.swap(result);
        }
    }
}

const SymHeap& SymExecEngine::callEntry() const
{
    CL_BREAK_IF(heapIdx_ < 1);
    return localState_[heapIdx_ - /* already incremented for next wheel */ 1];
}

const CodeStorage::Insn& SymExecEngine::callInsn() const
{
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // check for possible protocol error
    CL_BREAK_IF(CL_INSN_CALL != insn->code);

    return *insn;
}

SymState& SymExecEngine::callResults()
{
    return callResults_;
}

bool SymExecEngine::endReached() const
{
    return endReached_;
}

void SymExecEngine::forceEndReached()
{
    endReached_ = true;
}

void SymExecEngine::processPendingSignals()
{
    int signum;
    if (!SignalCatcher::caught(&signum))
        return;

    CL_WARN_MSG(lw_, "caught signal " << signum);
    stats_.printStats();
    printMemUsage("SymExec::printStats");

    switch (signum) {
        case SIGUSR1:
            break;

        default:
            // time to finish...
            throw std::runtime_error("signalled to die");
    }
}

void SymExecEngine::pruneOrigin()
{
#if SE_STATE_PRUNING_MODE
    if (block_->isLoopEntry())
#endif
        // never prune loop entry, it would break the fixed-point computation
        return;

    SymStateMarked &origin = stateMap_[block_];
    const unsigned size = origin.size();

#if SE_STATE_PRUNING_MISS_THR
    if (!stateMap_.anyReuseHappened(block_)
            && (SE_STATE_PRUNING_MISS_THR) <= size)
        goto thr_reached;
#endif

#if SE_STATE_PRUNING_TOTAL_THR
    if ((SE_STATE_PRUNING_TOTAL_THR) <= size)
        goto thr_reached;
#endif

#if SE_STATE_PRUNING_MODE < 2
    if (!cl_is_term_insn(block_->front()->code)
            && (CL_INSN_COND != block_->back()->code || 2 < block_->size()))
        return;
#endif

#if SE_STATE_PRUNING_MODE < 3
    if (1 < block_->inbound().size())
        // more than one incoming edges, keep this one
        return;
#endif

#if SE_STATE_PRUNING_MISS_THR || SE_STATE_PRUNING_TOTAL_THR
thr_reached:
#endif
    if (0x100 < size)
        printMemUsage("SymExecEngine::execInsn");

    for (unsigned i = 0; i < size; ++i) {
        if (origin.isDone(i))
            continue;

        CL_DEBUG_MSG(lw_, "SymExecEngine::pruneOrigin() failed to pack "
                << block_->name()
                << ", sh #" << i << " has not been processed yet");

        CL_BREAK_IF("SymExecEngine::pruneOrigin() has failed");
        return;
    }

    origin.clear();

    CL_DEBUG_MSG(lw_, "SymExecEngine::pruneOrigin() cleared " << block_->name()
            << " (initial size of state was " << size << ")");

    if (0x100 < size)
        printMemUsage("SymExecEngine::pruneOrigin");
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
SymExec::~SymExec()
{
    // NOTE this is actually the right direction (from top of the backtrace)
    BOOST_FOREACH(const ExecStackItem &item, execStack_) {

        // invalidate call ctx
        item.ctx->invalidate();

        // delete engine
        delete item.eng;
        printMemUsage("SymExecEngine::~SymExecEngine");
    }
}

const CodeStorage::Fnc* SymExec::resolveCallInsn(
        SymState                    &results,
        SymHeap                     entry,
        const CodeStorage::Insn     &insn)
{
    const CodeStorage::Fnc *fnc;
    const CodeStorage::TOperandList &opList = insn.operands;
    CL_BREAK_IF(CL_INSN_CALL != insn.code || opList.size() < 2);

    // look for Fnc ought to be called
    SymBackTrace &bt = callCache_.bt();
    SymProc proc(entry, &bt);
    const struct cl_loc *lw = &insn.loc;
    proc.setLocation(lw);

    const struct cl_operand &opFnc = opList[/* fnc */ 1];

    EMsgLevel ml = ML_ERROR;

    int uid;
    if (!proc.fncFromOperand(&uid, opFnc)) {
        CL_BREAK_IF(CL_OPERAND_CST == opFnc.code);
        CL_ERROR_MSG(lw, "failed to resolve indirect function call");
        goto fail;
    }

    if (SE_MAX_CALL_DEPTH < bt.size()) {
        CL_ERROR_MSG(lw, "call depth exceeds SE_MAX_CALL_DEPTH"
                << " (" << SE_MAX_CALL_DEPTH << ")");
        goto fail;
    }

    fnc = stor_.fncs[uid];
    if (!isDefined(*fnc)) {
        const char *name = nameOf(*fnc);
        CL_WARN_MSG(lw, "ignoring call of undefined function: "
                << name << "()");

        // do not treat this as a real error
        ml = ML_WARN;
        goto fail;
    }

    // all OK
    return fnc;

fail:
    // failed to resolve fnc call, so that we have exactly one resulting heap
    Trace::waiveCloneOperation(entry);

    const struct cl_operand dst = opList[/* dst */ 0];
    if (CL_OPERAND_VOID != dst.code) {
        // set return value to unknown
        const EValueOrigin origin = (CL_TYPE_INT == dst.type->code)
            ? VO_ASSIGNED
            : VO_UNKNOWN;

        const TValId val = entry.valCreate(VT_UNKNOWN, origin);
        const FldHandle fld = proc.fldByOperand(dst);
        if (fld.isValidHandle())
            proc.setValueOf(fld, val);
    }

    // something wrong happened, print the backtrace
    proc.printBackTrace(ml);

    if (!proc.hasFatalError())
        results.insert(entry);

    return 0;
}

void SymExec::enterCall(SymCallCtx *ctx, SymState &results)
{
    // create engine
    SymExecEngine *eng = new SymExecEngine(
            ctx->rawResults(),
            ctx->entry(),
            /* IStatsProvider */ *this,
            callCache_.bt());

    // initialize a stack item
    ExecStackItem item;
    item.ctx = ctx;
    item.eng = eng;
    item.dst = &results;

    // push the item to the exec-stack
    execStack_.push_front(item);
    printMemUsage("SymExec::enterCall");
}

void SymExec::execFnc(
        SymState                        &results,
        const SymHeap                   &entry,
        const CodeStorage::Insn         &insn,
        const CodeStorage::Fnc          &fnc)
{
    // get call context for the root function
    SymCallCtx *ctx = callCache_.getCallCtx(entry, fnc, insn);
    CL_BREAK_IF(!ctx || !ctx->needExec());

    // root call
    this->enterCall(ctx, results);

    // main loop
    while (!execStack_.empty()) {
        const ExecStackItem &item = execStack_.front();
        SymExecEngine *engine = item.eng;

        // do as much as we can at the current call level
        if (engine->run()) {
            printMemUsage("SymExecEngine::run");

            // call done at this level
            item.ctx->flushCallResults(*item.dst);
            item.ctx->invalidate();

            // noisy warnings elimination
            const bool forceEndReached = !item.dst->size()
                                      && engine->endReached();

            // remove top of the stack
            delete engine;
            printMemUsage("SymExecEngine::~SymExecEngine");
            execStack_.pop_front();

            if (!execStack_.empty() && forceEndReached)
                // well, we got no results, but the callee suggests to be silent
                execStack_.front().eng->forceEndReached();

            // we are done with this call, now wake up the caller!
            continue;
        }

        // function call requested
        // --> we need to nest unless the computed result is already available
        SymState &results = engine->callResults();
        const SymHeap &entry = engine->callEntry();
        const CodeStorage::Insn &insn = engine->callInsn();
        const CodeStorage::Fnc *fnc = this->resolveCallInsn(results, entry, insn);

        SymCallCtx *ctx = 0;
        if (fnc)
            // call cache lookup
            ctx = callCache_.getCallCtx(entry, *fnc, insn);

        if (!ctx)
            // the error message should have been already emitted, but there
            // are probably some unknown values in the result; now wake up
            // the caller
            continue;

        if (!ctx->needExec()) {
            // call cache hit
            const struct cl_loc *loc = &insn.loc;
            const std::string name = nameOf(*fnc);
            CL_DEBUG_MSG(loc,
                    "(x) call of function optimized out: " << name << "()");

            // use the cached result
            ctx->flushCallResults(engine->callResults());

            // wake up the caller
            continue;
        }

        // create a new engine and push it to the exec stack
        this->enterCall(ctx, engine->callResults());
    }
}

void SymExec::printStats() const
{
    // TODO: print SymCallCache stats here as soon as we have implemented some

    BOOST_FOREACH(const ExecStackItem &item, execStack_) {
        const IStatsProvider *provider = item.eng;
        provider->printStats();
    }
}

void execTopCall(
        SymState                        &results,
        const SymHeap                   &entry,
        const CodeStorage::Insn         &insn,
        const CodeStorage::Fnc          &fnc)
{
    // do not include the memory allocated by Code Listener into our statistics
    initMemDrift();

    try {
        SymExec se(entry.stor());
        se.execFnc(results, entry, insn, fnc);
        // SymExec::~SymExec() is going to be executed as leaving this block
    }
    catch (const std::runtime_error &e) {
        const struct cl_loc *loc = locationOf(fnc);
        CL_WARN_MSG(loc, "symbolic execution terminates prematurely");
        CL_NOTE_MSG(loc, e.what());
        throw;
    }
}

void execute(
        SymState                        &results,
        const SymHeap                   &entry,
        const CodeStorage::Fnc          &fnc)
{
    if (!installSignalHandlers())
        CL_WARN("unable to install signal handlers");

    // XXX: synthesize CL_INSN_CALL
    static CodeStorage::Insn insn;
    insn.stor = fnc.stor;
    insn.bb   = const_cast<CodeStorage::Block *>(fnc.cfg.entry());
    insn.code = CL_INSN_CALL;
    insn.loc  = *locationOf(fnc);
    insn.operands.resize(2);
    insn.operands[1] = fnc.def;

    // run the symbolic execution
    execTopCall(results, entry, insn, fnc);
    printMemUsage("SymExec::~SymExec");

    // uninstall signal handlers
    if (!SignalCatcher::cleanup())
        CL_WARN("unable to restore previous signal handlers");
}
