/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

#include "sigcatch.hh"
#include "symabstract.hh"
#include "symbt.hh"
#include "symcall.hh"
#include "symdebug.hh"
#include "sympath.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "util.hh"

#include <set>
#include <sstream>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

LOCAL_DEBUG_PLOTTER(nondetCond, DEBUG_SE_NONDET_COND)

#if DEBUG_MEM_USAGE
#   include <malloc.h>
void printMemUsage(const char *fnc) {
    static bool overflowDetected;
    if (overflowDetected)
        // instead of printing misleading numbers, we rather print nothing
        return;

    struct mallinfo info = mallinfo();
    const unsigned cnt = info.uordblks >> /* MiB */ 20;
    if (2048U <= cnt) {
        // mallinfo() is broken by design <https://bugzilla.redhat.com/173813>
        overflowDetected = true;
        return;
    }

    CL_DEBUG("current memory usage: " << cnt << " MB"
             << " (just completed " << fnc << "())");
}
#else
void printMemUsage(const char *) { }
#endif

// utilities
namespace {

bool installSignalHandlers(void) {
    // will be processed in SymExecEngine::processPendingSignals() eventually
    return SignalCatcher::install(SIGINT)
        && SignalCatcher::install(SIGUSR1)
        && SignalCatcher::install(SIGTERM);
}

void createGlVars(SymHeap &sh, const CodeStorage::Storage &stor) {
    using namespace CodeStorage;

    // now is the time to safely perform the initialization
    BOOST_FOREACH(const Var &var, stor.vars) {
        if (VAR_GL != var.code)
            continue;

        CL_DEBUG_MSG(&var.loc, "(g) initializing global variable: #" << var.uid
                << " (" << var.name << ")");

        // initialize a global/static variable
        initVariable(sh, var, /* gl variable */ 0);
    }
}

} // namespace

/// we want BBs to be taken from the scheduler somehow deterministically
struct BlockPtr {
    const CodeStorage::Block *bb;

    BlockPtr(const CodeStorage::Block *bb_):
        bb(bb_)
    {
    }
};
bool operator<(const BlockPtr &a, const BlockPtr &b) {
    if (a.bb == b.bb)
        // identical blocks, we're done
        return false;

    // first try to compare the names of blocks, as they're said to be more
    // consistent among runs than the addresses of CodeStorage::Block objects
    // NOTE: They're however _not_ guaranteed to be consistent among two
    //       different gcc builds anyway, and I am not yet talking about
    //       gcc versions...
    const std::string &aName = a.bb->name();
    const std::string &bName = b.bb->name();
    const int cmp = aName.compare(bName);
    if (cmp)
        return (cmp < 0);

    else
        // names are equal, just compare the pointers to keep std::set working
        return (a.bb < b.bb);
}

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine
class SymExecEngine: public IStatsProvider {
    public:
        SymExecEngine(const SymExec &se, SymBackTrace &bt,
                      const SymHeap &src, SymState &dst):
            stor_(se.stor()),
            params_(se.params()),
            bt_(bt),
            dst_(dst),
            stats_(se),
            ptracer_(stateMap_),
            block_(0),
            insnIdx_(0),
            heapIdx_(0),
            waiting_(false),
            endReached_(false)
        {
            this->initEngine(src);
            if (params_.ptrace)
                // register path printer
                bt_.pushPathTracer(&ptracer_);
        }

        ~SymExecEngine() {
            if (params_.ptrace)
                // unregister path printer
                bt_.popPathTracer(&ptracer_);
        }

    public:
        bool /* complete */ run();

        virtual void printStats() const;

        // TODO: describe the interface briefly
        const SymHeap*              callEntry() const;
        const CodeStorage::Insn*    callInsn() const;
        SymState*                   callResults();

    private:
        typedef std::set<BlockPtr>      TBlockSet;

        const CodeStorage::Storage      &stor_;
        SymExecParams                   params_;
        SymBackTrace                    &bt_;
        SymState                        &dst_;
        const IStatsProvider            &stats_;
        std::string                     fncName_;

        SymStateMap                     stateMap_;
        PathTracer                      ptracer_;
        TBlockSet                       todo_;
        const CodeStorage::Block        *block_;
        unsigned                        insnIdx_;
        unsigned                        heapIdx_;
        bool                            waiting_;
        bool                            endReached_;

        SymHeapList                     localState_;
        SymHeapList                     nextLocalState_;
        SymStateWithJoin                callResults_;
        const struct cl_loc             *lw_;

    private:
        void initEngine(const SymHeap &init);
        void execReturn();
        void updateState(SymHeap &sh, const CodeStorage::Block *ofBlock);
        void updateStateInBranch(
                SymHeap                     sh,
                const CodeStorage::Block    *ofBlock,
                const bool                  branch,
                const cl_binop_e            code,
                const TValId                v1,
                const TValId                v2);

        void execCondInsn();
        void execTermInsn();
        bool execNontermInsn();
        bool execInsn();
        bool execBlock();
        void processPendingSignals();
};

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine implementation
void SymExecEngine::initEngine(const SymHeap &init)
{
    // look for fnc name
    const CodeStorage::Fnc *fnc = bt_.topFnc();
    fncName_ = nameOf(*fnc);
    lw_ = &fnc->def.loc;
    CL_DEBUG_MSG(lw_, ">>> entering " << fncName_ << "()");

    // look for the entry block
    const CodeStorage::Block *entry = fnc->cfg.entry();
    if (!entry) {
        CL_ERROR_MSG(lw_, fncName_ << ": " << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    stateMap_.insert(entry, /* no inbound edge here */ 0, init);

    // schedule the entry block for processing
    todo_.insert(entry);
}

void SymExecEngine::execReturn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &opList = insn->operands;
    CL_BREAK_IF(1 != opList.size());

    SymHeap heap(localState_[heapIdx_]);

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymProc proc(heap, &bt_);
        proc.setLocation(lw_);

        const TValId val = proc.valFromOperand(src);
        CL_BREAK_IF(VAL_INVALID == val);

        heap.objDefineType(OBJ_RETURN, src.type);
        proc.objSetValue(OBJ_RETURN, val);
    }

    // commit one of the function results
    dst_.insertFast(heap);
    endReached_ = true;
}

void SymExecEngine::updateState(SymHeap &sh, const CodeStorage::Block *ofBlock) {
    const std::string &name = ofBlock->name();

    // time to consider abstraction
    abstractIfNeeded(sh);

    // update _target_ state and check if anything has changed
    if (!stateMap_.insertFast(ofBlock, block_, sh)) {
        CL_DEBUG_MSG(lw_, "--- block " << name
                     << " left intact (size of target is "
                     << stateMap_[ofBlock].size() << ")");
    }
    else {
        const size_t last = todo_.size();

        // schedule for next wheel (if not already)
        todo_.insert(ofBlock);

        const bool already = (todo_.size() == last);
        CL_DEBUG_MSG(lw_, ((already) ? "-+-" : "+++")
                << " block " << name
                << ((already)
                    ? " changed, but already scheduled"
                    : " scheduled for next wheel")
                << " (size of target is " << stateMap_[ofBlock].size() << ")");
    }
}

void SymExecEngine::updateStateInBranch(
        SymHeap                     sh,
        const CodeStorage::Block    *ofBlock,
        const bool                  branch,
        const cl_binop_e            code,
        const TValId                v1,
        const TValId                v2)
{
    // resolve binary operator
    bool neg, preserveEq, preserveNeq;
    if (describeCmpOp(code, &neg, &preserveEq, &preserveNeq)) {
        if (branch == neg) {
            if (!preserveNeq)
                goto fallback;

            // introduce a Neq predicate over v1 and v2
            sh.neqOp(SymHeap::NEQ_ADD, v1, v2);
        }
        else {
            if (!preserveEq)
                goto fallback;

            // we have deduced that v1 and v2 is actually the same value
            sh.valMerge(v1, v2);
        }
    }

    LDP_PLOT(nondetCond, sh);

fallback:
    this->updateState(sh, ofBlock);
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
    const struct cl_type *cltSrc = op1.type;
    CL_BREAK_IF(!cltSrc || !op2.type || cltSrc->code != op2.type->code);

    // a working area for the CL_INSN_BINOP instruction
    SymHeap sh(localState_[heapIdx_]);
    SymProc proc(sh, &bt_);
    proc.setLocation(lw_);

    // compute the result of CL_INSN_BINOP
    const enum cl_binop_e code = static_cast<enum cl_binop_e>(insnCmp->subCode);
    const TValId v1 = proc.valFromOperand(op1);
    const TValId v2 = proc.valFromOperand(op2);
    const TValId val = compareValues(sh, code, cltSrc, v1, v2);

    // read targets
    const CodeStorage::TTargetList &tlist = insnCnd->targets;
    const CodeStorage::Block *targetThen = tlist[/* then label */ 0];
    const CodeStorage::Block *targetElse = tlist[/* else label */ 1];

    // inconsistency check
    switch (val) {
        case VAL_DEREF_FAILED:
            // error should have been already emitted
            CL_DEBUG_MSG(lw_, "ignored VAL_DEREF_FAILED");
            return;

        case VAL_TRUE:
            CL_DEBUG_MSG(lw_, ".T. CL_INSN_COND got VAL_TRUE");
            this->updateState(sh, targetThen);
            return;

        case VAL_FALSE:
            CL_DEBUG_MSG(lw_, ".F. CL_INSN_COND got VAL_FALSE");
            this->updateState(sh, targetElse);
            return;

        default:
            // we need to update both targets
            break;
    }

    const EValueOrigin vo = sh.valOrigin(val);
    if (isUninitialized(vo)) {
        // TODO: make the warning messages more precise
        CL_WARN_MSG(lw_, "conditional jump depends on uninitialized value");
        bt_.printBackTrace();
    }

    std::ostringstream str;
    str << "at-line-" << lw_->line;
    LDP_INIT(nondetCond, str.str().c_str());
    LDP_PLOT(nondetCond, sh);

    CL_DEBUG_MSG(lw_, "?T? CL_INSN_COND updates TRUE branch");
    this->updateStateInBranch(sh, targetThen, true, code, v1, v2);

    CL_DEBUG_MSG(lw_, "?F? CL_INSN_COND updates FALSE branch");
    this->updateStateInBranch(sh, targetElse, false, code, v1, v2);
}

void SymExecEngine::execTermInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TTargetList &tlist = insn->targets;

    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_RET:
            this->execReturn();
            break;

        case CL_INSN_COND:
            this->execCondInsn();
            break;

        case CL_INSN_ABORT:
            CL_DEBUG_MSG(lw_, "CL_INSN_ABORT reached");
            endReached_ = true;
            break;

        case CL_INSN_JMP:
            if (1 == tlist.size()) {
                SymHeap sh(localState_[heapIdx_]);
                this->updateState(sh, tlist[/* target */ 0]);
                break;
            }
            // go through!

        default:
            CL_TRAP;
    }
}

bool /* handled */ SymExecEngine::execNontermInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // set some properties of the execution
    SymExecCoreParams ep;
    ep.fastMode         = params_.fastMode;
    ep.invCompatMode    = params_.invCompatMode;
    ep.skipPlot         = params_.skipPlot;

    // working area for non-terminal instructions
    SymHeap workingHeap(localState_[heapIdx_]);
    SymExecCore core(workingHeap, &bt_, ep);
    core.setLocation(lw_);

    // execute the instruction
    return core.exec(nextLocalState_, *insn);
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

    if (!isTerm) {
        CL_BREAK_IF(block_->size() <= insnIdx_);

        // look ahead for CL_INSN_COND
        const CodeStorage::Insn *nextInsn = block_->operator[](insnIdx_ + 1);
        if (CL_INSN_COND == nextInsn->code) {
            // this is going to be handled in execCondInsn() right away
            CL_BREAK_IF(CL_INSN_BINOP != insn->code);
            localState_.swap(nextLocalState_);
            return true;
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

        if (1 < hCnt) {
            CL_DEBUG_MSG(lw_, "*** processing heap #" << heapIdx_
                         << " (initial size of state was " << hCnt << ")");
        }

        // terrify the user by our current schedule if he is asking for that :-)
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

    // completed execution of the given insn
    heapIdx_ = 0;
    return true;
}

bool /* complete */ SymExecEngine::execBlock() {
    const std::string &name = block_->name();

    if (insnIdx_ || heapIdx_) {
        // some debugging output of the resume process
        CL_DEBUG_MSG(lw_, "___ we are back in " << name
                << ", insn #" << insnIdx_
                << ", heap #" << (heapIdx_ - 1)
                << ", " << todo_.size() << " basic block(s) in the queue");
    }

    if (!insnIdx_)
        // fresh run, let's initialize the local state by the BB entry
        localState_ = stateMap_[block_];

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

        // swap states in order to be ready for next insn
        localState_.swap(nextLocalState_);
    }

    // the whole block is processed now
    CL_DEBUG_MSG(lw_, "___ completed batch for " << name
                 << ", " << todo_.size() << " basic block(s) in the queue");
    insnIdx_ = 0;
    return true;
}

void joinNewResults(
        SymHeapList             &dst,
        const SymState          &src)
{
    SymStateWithJoin all;
    all.swap(dst);
    all.SymState::insert(src);
    all.swap(dst);
}

bool /* complete */ SymExecEngine::run() {
    const CodeStorage::Fnc *fnc = bt_.topFnc();

    if (waiting_) {
        // pick up results of the pending call
        joinNewResults(nextLocalState_, callResults_);

        // we're on the way from a just completed function call...
        if (!this->execBlock())
            // ... and we've just hit another one
            return false;
    }
    else {
        // we have a fresh instance of SymExecEngine
        waiting_ = true;

        // check for possible protocol error
        CL_BREAK_IF(1 != todo_.size());
    }

    // main loop of SymExecEngine
    while (!todo_.empty()) {
        // FIXME: take BBs in some reasonable order instead
        TBlockSet::iterator i = todo_.begin();
        block_ = i->bb;
        todo_.erase(i);

        // update location info and ptracer
        const CodeStorage::Insn *first = block_->front();
        lw_ = &first->loc;
        ptracer_.setBlock(block_);

        // enter the basic block
        const std::string &name = block_->name();
        CL_DEBUG_MSG(lw_, "___ entering " << name
                     << ", " << todo_.size() << " basic block(s) in the queue");
        insnIdx_ = 0;
        heapIdx_ = 0;

        // process the basic block till the first function call
        if (!this->execBlock())
            // function call reached, suspend the execution for now
            return false;
    }

    const struct cl_loc *loc = &fnc->def.loc;
    if (!endReached_) {
        CL_WARN_MSG(loc, "end of function "
                << nameOf(*fnc) << "() has not been reached");
        bt_.printBackTrace();
    }

    // we are done with this function
    CL_DEBUG_MSG(loc, "<<< leaving " << nameOf(*fnc) << "()");
    waiting_ = false;
    return true;
}

void SymExecEngine::printStats() const {
    TBlockSet bset(todo_);
    if (block_)
        // include the basic block just being computed into the statistics
        bset.insert(block_);

    // per function statistics
    CL_NOTE_MSG(lw_,
            "... while executing " << fncName_ << "()"
            ", " << dst_.size() << " result(s) already computed"
            ", " << bset.size() << " basic block(s) in the queue"
            ", " << localState_.size() << " src heap(s)"
            ", " << nextLocalState_.size() << " dst heap(s)"
            ", insn #" << insnIdx_ <<
            ", heap #" << heapIdx_);

    // go through scheduled basic blocks
    BOOST_FOREACH(const BlockPtr &ptr, bset) {
        const CodeStorage::Block *bb = ptr.bb;
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
}

const SymHeap* SymExecEngine::callEntry() const {
    CL_BREAK_IF(heapIdx_ < 1);
    return &localState_[heapIdx_ - /* already incremented for next wheel */ 1];
}

const CodeStorage::Insn* SymExecEngine::callInsn() const {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // check for possible protocol error
    CL_BREAK_IF(CL_INSN_CALL != insn->code);

    return insn;
}

SymState* SymExecEngine::callResults() {
    return &callResults_;
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
            // remove all handlers and forward the signal to the original
            // handler, which will most likely kill the process by SIGINT
            // or SIGTERM
            SignalCatcher::cleanup();
            CL_BREAK_IF(SIGINT != signum && SIGTERM != signum);
            raise(signum);
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct StackItem;
struct SymExec::Private {
    SymExec                                 &se;
    const CodeStorage::Storage              &stor;
    SymExecParams                           params;
    SymStateWithJoin                        stateZero;
    SymBackTrace                            bt;
    SymCallCache                            callCache;
    typedef std::stack<IStatsProvider *>    TStatsStack;
    TStatsStack                             statsStack;

    Private(SymExec &se_, const CodeStorage::Storage &stor_):
        se(se_),
        stor(stor_),
        bt(stor_),
        callCache(&bt)
    {
    }

    const CodeStorage::Fnc* resolveCallInsn(SymHeap                     entry,
                                            const CodeStorage::Insn     &insn,
                                            SymState                    &dst);

    SymExecEngine* createEngine(SymCallCtx &ctx);

    void execLoop(const StackItem &item);
};

SymExec::SymExec(const CodeStorage::Storage &stor, const SymExecParams &params):
    d(new Private(*this, stor))
{
    d->params = params;

    // create the initial state, consisting of global/static variables
    SymHeap init(stor);
    createGlVars(init, stor);
    d->stateZero.insertFast(init);
}

SymExec::~SymExec() {
    delete d;
    printMemUsage("SymExec::~SymExec");
}

const CodeStorage::Storage& SymExec::stor() const {
    return d->stor;
}

const SymExecParams& SymExec::params() const {
    return d->params;
}

const CodeStorage::Fnc* SymExec::Private::resolveCallInsn(
        SymHeap                     heap,
        const CodeStorage::Insn     &insn,
        SymState                    &results)
{
    const CodeStorage::Fnc *fnc;
    const CodeStorage::TOperandList &opList = insn.operands;
    CL_BREAK_IF(CL_INSN_CALL != insn.code || opList.size() < 2);

    // look for Fnc ought to be called
    SymProc proc(heap, &this->bt);
    const struct cl_loc *lw = &insn.loc;
    proc.setLocation(lw);

    const struct cl_operand &opFnc = opList[/* fnc */ 1];
    const int uid = proc.fncFromOperand(opFnc);
    if (-1 == uid) {
        CL_BREAK_IF(CL_OPERAND_CST == opFnc.code);
        CL_ERROR_MSG(lw, "failed to resolve indirect function call");
        goto fail;
    }

    if (SE_MAX_CALL_DEPTH < this->bt.size()) {
        CL_ERROR_MSG(lw, "call depth exceeds SE_MAX_CALL_DEPTH"
                << " (" << SE_MAX_CALL_DEPTH << ")");
        goto fail;
    }

    fnc = this->stor.fncs[uid];
    if (!isDefined(*fnc)) {
        const struct cl_cst &cst = opFnc.data.cst;
        const char *name = cst.data.cst_fnc.name;
        CL_BREAK_IF(CL_TYPE_FNC != cst.code || !name);

        CL_WARN_MSG(lw, "ignoring call of undefined function: "
                << name << "()");
        goto fail;
    }

    // enter backtrace
    this->bt.pushCall(uid, lw);
    return fnc;

fail:
    // something wrong happened, print the backtrace
    this->bt.printBackTrace();

    const struct cl_operand dst = opList[/* dst */ 0];
    if (CL_OPERAND_VOID != dst.code) {
        // set return value to unknown
        const TValId val = heap.valCreateUnknown(VT_UNKNOWN, VO_UNKNOWN);
        const TObjId obj = proc.objByOperand(dst);
        proc.objSetValue(obj, val);
    }

    // call failed, so that we have exactly one resulting heap
    results.insertFast(heap);
    return 0;
}

SymExecEngine* SymExec::Private::createEngine(SymCallCtx &ctx) {
    return new SymExecEngine(
            this->se,
            this->bt,
            ctx.entry(),
            ctx.rawResults());
}

struct StackItem {
    SymCallCtx      *ctx;
    SymExecEngine   *eng;
    SymState        *dst;
};

void SymExec::Private::execLoop(const StackItem &item) {
    using CodeStorage::Fnc;

    // register statistics provider
    CL_BREAK_IF(!this->statsStack.empty());
    this->statsStack.push(item.eng);

    // run-time stack
    typedef std::stack<StackItem> TStack;
    TStack rtStack;
    rtStack.push(item);

    // main loop
    while (!rtStack.empty()) {
        const StackItem &item = rtStack.top();
        SymExecEngine *engine = item.eng;

        // do as much as we can at the current call level
        if (engine->run()) {
            printMemUsage("SymExecEngine::run");

            // call done at this level
            item.ctx->flushCallResults(*item.dst);
            item.ctx->invalidate();
            this->bt.popCall();
            printMemUsage("SymCallCtx::flushCallResults");

            // unregister statistics provider
            CL_BREAK_IF(this->statsStack.empty());
            this->statsStack.pop();

            // remove top of the stack
            delete engine;
            rtStack.pop();

            // wake up the caller (if any)
            printMemUsage("SymExecEngine::~SymExecEngine");
            continue;
        }

        // function call requested
        // --> we need to nest unless the computed result is already available
        const SymHeap &entry = *engine->callEntry();
        const CodeStorage::Insn &insn = *engine->callInsn();
        SymState &results = *engine->callResults();
        const Fnc *fnc = this->resolveCallInsn(entry, insn, results);
        if (!fnc)
            // the error message should have been already emitted, but there
            // are probably some unknown values in the result; now wake up
            // the caller
            continue;

        // call cache lookup
        SymCallCtx &ctx = this->callCache.getCallCtx(entry, *fnc, insn);
        if (!ctx.needExec()) {
            // call cache hit
            const struct cl_loc *lw = this->bt.topCallLoc();
            CL_DEBUG_MSG(lw, "(x) call of function optimized out: "
                    << nameOf(*fnc) << "()");

            // use the cached result
            ctx.flushCallResults(*engine->callResults());

            // leave backtrace
            this->bt.popCall();

            // wake up the caller
            continue;
        }

        printMemUsage("SymCall::getCallCtx");

        // prepare a new run-time stack item for the call
        StackItem next;
        next.ctx = &ctx;
        next.eng = this->createEngine(ctx);
        printMemUsage("SymExec::createEngine");

        // pass the result back to the caller as soon as we have one
        next.dst = item.eng->callResults();

        // register statistics provider
        this->statsStack.push(next.eng);

        // perform the call now!
        rtStack.push(next);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc, SymState &results) {
    if (!installSignalHandlers())
        CL_WARN("unable to install signal handlers");

    // go through all symbolic heaps of the initial state, merging the results
    // all together
    BOOST_FOREACH(const SymHeap &heap, d->stateZero) {
        // check for bt offset
        CL_BREAK_IF(d->bt.size());

        // initialize backtrace
        d->bt.pushCall(uidOf(fnc), &fnc.def.loc);

        // XXX: synthesize CL_INSN_CALL
        CodeStorage::Insn insn;
        insn.stor = fnc.stor;
        insn.code = CL_INSN_CALL;
        insn.loc  = fnc.def.loc;
        insn.operands.resize(2);
        insn.operands[1] = fnc.def;
        insn.opsToKill.resize(2, false);

        // get call context for the root function
        SymCallCtx &ctx = d->callCache.getCallCtx(heap, fnc, insn);
        if (!ctx.needExec()) {
            // not likely to happen in the way that SymExec is currently used
            CL_WARN_MSG(d->bt.topCallLoc(), "(x) root call optimized out: "
                    << nameOf(fnc) << "()");

            ctx.flushCallResults(results);
            d->bt.popCall();
            continue;
        }

        // root stack item
        StackItem si;
        si.ctx = &ctx;
        si.eng = d->createEngine(ctx);
        si.dst = &results;

        // root call
        d->execLoop(si);
    }

    // uninstall signal handlers
    if (!SignalCatcher::cleanup())
        CL_WARN("unable to restore previous signal handlers");
}

void SymExec::printStats() const {
    // TODO: print SymCallCache stats here as soon as we have implemented some

    typedef Private::TStatsStack TStack;
    for (TStack tmpStack(d->statsStack); !tmpStack.empty(); tmpStack.pop()) {
        const IStatsProvider *provider = tmpStack.top();
        provider->printStats();
    }
}
