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
#include <cl/storage.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>

#include "memdebug.hh"
#include "sigcatch.hh"
#include "symabstract.hh"
#include "symcall.hh"
#include "symdebug.hh"
#include "sympath.hh"
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

bool installSignalHandlers(void) {
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
        SymExec(const CodeStorage::Storage &stor, const SymExecParams &ep):
            stor_(stor),
            params_(ep),
            callCache_(stor, ep.ptrace)
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
        SymExecParams                           params_;
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
                const SymExecParams     &ep,
                SymBackTrace            &bt):
            stor_(entry.stor()),
            params_(ep),
            bt_(bt),
            dst_(results),
            stats_(stats),
            ptracer_(stateMap_),
            block_(0),
            insnIdx_(0),
            heapIdx_(0),
            waiting_(false),
            endReached_(false)
        {
            this->initEngine(entry);

            // register path printer
            bt_.pushPathTracer(&ptracer_);
        }

        ~SymExecEngine() {
            // unregister path printer
            bt_.popPathTracer(&ptracer_);
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
        SymExecParams                   params_;
        SymBackTrace                    &bt_;
        SymState                        &dst_;
        const IStatsProvider            &stats_;
        std::string                     fncName_;

        SymStateMap                     stateMap_;
        PathTracer                      ptracer_;
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

        void dumpStateMap();

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

    // look for the entry block
    const CodeStorage::Block *entry = fnc.cfg.entry();
    if (!entry) {
        CL_ERROR_MSG(lw_, fncName_ << ": " << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    stateMap_.insert(entry, /* no inbound edge here */ 0, init);

    // schedule the entry block for processing
    sched_.schedule(entry);
}

void SymExecEngine::execJump() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TTargetList &tlist = insn->targets;

    // make a copy in case we needed to perform an abstraction
    const SymHeap &origin = localState_[heapIdx_];
    SymHeap sh(origin);
    Trace::waiveCloneOperation(sh);

    this->updateState(sh, tlist[/* target */ 0]);
}

void SymExecEngine::execReturn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &opList = insn->operands;
    CL_BREAK_IF(1 != opList.size());

    // create a local copy of the heap
    const SymHeap &origin = localState_[heapIdx_];
    SymHeap sh(origin);

    Trace::Node *trOrig = origin.traceNode();
    Trace::Node *trRet = new Trace::InsnNode(trOrig, insn, /* bin */ false);
    sh.traceUpdate(trRet);

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymProc proc(sh, &bt_);
        proc.setLocation(lw_);

        const TValId val = proc.valFromOperand(src);
        CL_BREAK_IF(VAL_INVALID == val);

        sh.valSetLastKnownTypeOfTarget(VAL_ADDR_OF_RET, src.type);
        const ObjHandle ret(sh, VAL_ADDR_OF_RET, src.type);
        proc.objSetValue(ret, val);
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
    if (!stateMap_.insert(ofBlock, block_, sh, closingLoop)) {
        CL_DEBUG_MSG(lw_, "--- block " << name
                     << " left intact (size of target is "
                     << stateMap_[ofBlock].size() << ")");
    }
    else {
        // schedule for next wheel (if not already)
        const bool already = !sched_.schedule(ofBlock);
        CL_DEBUG_MSG(lw_, ((already) ? "-+-" : "+++")
                << " block " << name
                << ((already)
                    ? " changed, but already scheduled"
                    : " scheduled for next wheel")
                << " (size of target is " << stateMap_[ofBlock].size() << ")");
    }
}

bool isAnyAbstractOf(const SymHeapCore &sh, const TValId v1, const TValId v2) {
    return isAbstract(sh.valTarget(v1))
        || isAbstract(sh.valTarget(v2));
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

    const enum cl_binop_e code = static_cast<enum cl_binop_e>(insnCmp.subCode);
    if (!reflectCmpResult(dst, procOrig, code, branch, v1, v2))
        CL_DEBUG_MSG(lw_, "XXX unable to reflect comparison result");

    // as of yet, we always get exactly one heap, although the API is generic
    CL_BREAK_IF(1 != dst.size());

    BOOST_FOREACH(SymHeap *sh, dst) {
        sh->traceUpdate(trCond);
#if DEBUG_SE_END_NOT_REACHED < 2
        if (isAnyAbstractOf(shOrig, v1, v2))
            LDP_PLOT(nondetCond, *sh);
#endif
        SymProc proc(*sh, &bt_);
        proc.setLocation(lw_);
        const unsigned targetIdx = !branch;
        proc.killInsn(insnCmp);
        proc.killPerTarget(insnCnd, targetIdx);

        const CodeStorage::Block *target = insnCnd.targets[targetIdx];
        this->updateState(*sh, target);
    }
}

bool isTrackableValue(const SymHeap &sh, const TValId val) {
    const EValueTarget code = sh.valTarget(val);
    if (isPossibleToDeref(code))
        return true;

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
#if !SE_TRACK_NON_POINTER_VALUES
    const TObjType clt1 = insnCmp.operands[/* src1 */ 1].type;
    const TObjType clt2 = insnCmp.operands[/* src2 */ 2].type;
    if (isDataPtr(clt1) || isDataPtr(clt2))
#endif
        return false;

    // white-list some values that are worth tracking
    // cppcheck-suppress unreachableCode
    SymHeap &sh = proc.sh();
    if (isTrackableValue(sh, v1) || isTrackableValue(sh, v2))
        return false;

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

void SymExecEngine::execCondInsn() {
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

#if DEBUG_SE_END_NOT_REACHED < 2
    if (isAnyAbstractOf(sh, v1, v2))
#endif
    {
        std::ostringstream str;
        str << "at-line-" << lw_->line;
        LDP_INIT(nondetCond, str.str().c_str());
        LDP_PLOT(nondetCond, sh);
    }

    CL_DEBUG_MSG(lw_, "?T? CL_INSN_COND updates TRUE branch");
    this->updateStateInBranch(sh, true,  *insnCmp, *insnCnd, v1, v2);

    CL_DEBUG_MSG(lw_, "?F? CL_INSN_COND updates FALSE branch");
    this->updateStateInBranch(sh, false, *insnCmp, *insnCnd, v1, v2);
}

void SymExecEngine::execTermInsn() {
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

bool /* handled */ SymExecEngine::execNontermInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // set some properties of the execution
    SymExecCoreParams ep;
    ep.trackUninit      = params_.trackUninit;
    ep.oomSimulation    = params_.oomSimulation;
    ep.skipPlot         = params_.skipPlot;
    ep.errLabel         = params_.errLabel;

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

bool /* complete */ SymExecEngine::execInsn() {
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

        if (nextInsnIsCond)
            // this is going to be handled in execCondInsn() right away
            continue;

        if (1 < hCnt) {
            CL_DEBUG_MSG(lw_, "*** processing heap #" << heapIdx_
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

bool /* complete */ SymExecEngine::execBlock() {
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
        BOOST_FOREACH(SymHeap *sh, localState_)
            Trace::waiveCloneOperation(*sh);
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

void SymExecEngine::joinCallResults() {
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

bool /* complete */ SymExecEngine::run() {
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
        ptracer_.setBlock(block_);

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

    const struct cl_loc *loc = locationOf(fnc);
    if (!endReached_) {
        CL_WARN_MSG(loc, "end of function "
                << nameOf(fnc) << "() has not been reached");
#if DEBUG_SE_END_NOT_REACHED
        sched_.printStats();
        this->dumpStateMap();
#endif
        bt_.printBackTrace();
    }

    // we are done with this function
    CL_DEBUG_MSG(loc, "<<< leaving " << nameOf(fnc) << "()");
    waiting_ = false;
    return true;
}

void SymExecEngine::printStatsHelper(const BlockScheduler::TBlock bb) const {
    const std::string &name = bb->name();

    // query total count of heaps
    SymExecEngine *self = const_cast<SymExecEngine *>(this);
    const SymStateMarked &state = self->stateMap_[bb];
    const unsigned total = state.size();

    // compute heaps pending for execution
    unsigned waiting = 0;
    for (unsigned i = 0; i < total; ++i)
        if (!state.isDone(i))
            ++waiting;

    const char *status = (bb == block_)
        ? " in progress"
        : " scheduled";

    const CodeStorage::Insn *first = bb->front();
    CL_NOTE_MSG(&first->loc,
            "___ block " << name << status <<
            ", " << total << " heap(s) total"
            ", " << waiting << " heap(s) pending");
}

void SymExecEngine::printStats() const {
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

void SymExecEngine::dumpStateMap() {
    const BlockScheduler::TBlockList bbs(sched_.done());
    BOOST_FOREACH(const BlockScheduler::TBlock block, bbs) {
        const std::string name = block->name();

        const SymState &state = stateMap_[block];
        BOOST_FOREACH(const SymHeap *sh, state)
            plotHeap(*sh, name);
    }
}

const SymHeap& SymExecEngine::callEntry() const {
    CL_BREAK_IF(heapIdx_ < 1);
    return localState_[heapIdx_ - /* already incremented for next wheel */ 1];
}

const CodeStorage::Insn& SymExecEngine::callInsn() const {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // check for possible protocol error
    CL_BREAK_IF(CL_INSN_CALL != insn->code);

    return *insn;
}

SymState& SymExecEngine::callResults() {
    return callResults_;
}

bool SymExecEngine::endReached() const {
    return endReached_;
}

void SymExecEngine::forceEndReached() {
    endReached_ = true;
}

void SymExecEngine::processPendingSignals() {
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

void SymExecEngine::pruneOrigin() {
#if SE_STATE_PRUNING_MODE
    if (block_->isLoopEntry())
#endif
        // never prune loop entry, it would break the fixed-point computation
        return;

#if SE_STATE_PRUNING_MODE < 2
    if (1 < block_->inbound().size())
        // more than one incoming edges, keep this one
        return;
#endif

    SymStateMarked &origin = stateMap_[block_];
    SymHeapList tmp;

    bool hit = false;

    for (unsigned i = 0; i < origin.size(); ++i) {
        if (origin.isDone(i))
            hit = true;
        else
            tmp.insert(origin[i]);
    }

    if (!hit) {
        CL_DEBUG_MSG(lw_, "SymExecEngine::pruneOrigin() failed to pack "
                << block_->name());

        CL_BREAK_IF(tmp.size() != origin.size());
    }
    else {
        CL_DEBUG_MSG(lw_, "SymExecEngine::pruneOrigin() packed "
                << block_->name() << ": "
                << origin.size() << " -> "
                <<  tmp.size());

        CL_BREAK_IF(origin.size() <= tmp.size());
        origin.swap(tmp);
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
SymExec::~SymExec() {
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
        const ObjHandle obj = proc.objByOperand(dst);
        proc.objSetValue(obj, val);
    }

    // something wrong happened, print the backtrace
    proc.printBackTrace(ml);

    if (!proc.hasFatalError())
        results.insert(entry);

    return 0;
}

void SymExec::enterCall(SymCallCtx *ctx, SymState &results) {
    // create engine
    SymExecEngine *eng = new SymExecEngine(
            ctx->rawResults(),
            ctx->entry(),
            /* IStatsProvider */ *this,
            params_,
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
        this->enterCall(ctx, item.eng->callResults());
    }
}

void SymExec::printStats() const {
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
        const CodeStorage::Fnc          &fnc,
        const SymExecParams             &ep)
{
    // do not include the memory allocated by Code Listener into our statistics
    initMemDrift();

    try {
        SymExec se(entry.stor(), ep);
        se.execFnc(results, entry, insn, fnc);
        // SymExec::~SymExec() is going to be executed as leaving this block
    }
    catch (const std::runtime_error &e) {
        const struct cl_loc *loc = locationOf(fnc);
        CL_WARN_MSG(loc, "symbolic execution terminates prematurely");
        CL_NOTE_MSG(loc, e.what());
    }
}

void execute(
        SymState                        &results,
        const SymHeap                   &entry,
        const CodeStorage::Fnc          &fnc,
        const SymExecParams             &ep)
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
    execTopCall(results, entry, insn, fnc, ep);
    printMemUsage("SymExec::~SymExec");

    // uninstall signal handlers
    if (!SignalCatcher::cleanup())
        CL_WARN("unable to restore previous signal handlers");
}
