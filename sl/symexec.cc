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

#include "symbt.hh"
#include "symcall.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <set>
#include <stack>
#include <list>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// utilities
namespace {

void createGlVar(SymHeap &heap, const CodeStorage::Var &var) {
    // create the corresponding heap object
    const struct cl_type *clt = var.clt;
    const CVar cVar(var.uid, /* gl variable */ 0);
    const TObjId obj = heap.objCreate(clt, cVar);

    // now attempt to initialize the variable since it is a global/static var
    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_INT:
        case CL_TYPE_PTR:
            heap.objSetValue(obj, VAL_NULL);
            break;

        case CL_TYPE_BOOL:
            heap.objSetValue(obj, VAL_FALSE);
            break;

        case CL_TYPE_STRUCT:
            // TODO: go through the struct recursively and initialize
            // fall through!

        default:
            // only a few types are supported in case of gl variables for now
            TRAP;
    }
}

void createGlVars(SymHeap &heap, const CodeStorage::Storage &stor) {
    using namespace CodeStorage;
    BOOST_FOREACH(const Var &var, stor.vars) {
        if (VAR_GL == var.code) {
            const LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, "(g) creating global variable: #" << var.uid
                    << " (" << var.name << ")");
            createGlVar(heap, var);
        }
    }
}

} // namespace

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine
class SymExecEngine {
    public:
        SymExecEngine(const SymExec &se, const SymBackTrace &bt,
                      const SymHeap &src, SymHeapUnion &dst):
            stor_(se.stor()),
            params_(se.params()),
            bt_(bt),
            dst_(dst),
            block_(0),
            insnIdx_(0),
            heapIdx_(0),
            waiting_(false)
        {
            this->initEngine(src);
        }

    public:
        bool /* complete */ run();

        // TODO: describe the interface briefly
        const SymHeap*              callEntry() const;
        const CodeStorage::Insn*    callInsn() const;
        SymHeapUnion*               callResults();

    private:
        typedef const CodeStorage::Block                   *TBlock;
        typedef std::set<TBlock>                            TBlockSet;
        typedef std::map<TBlock, SymHeapScheduler>          TStateMap;

        const CodeStorage::Storage      &stor_;
        SymExecParams                   params_;
        const SymBackTrace              &bt_;
        SymHeapUnion                    &dst_;

        TStateMap                       stateMap_;
        TBlockSet                       todo_;
        TBlock                          block_;
        unsigned                        insnIdx_;
        unsigned                        heapIdx_;
        bool                            waiting_;

        SymHeapScheduler                localState_;
        SymHeapUnion                    nextLocalState_;
        LocationWriter                  lw_;

    private:
        void initEngine(const SymHeap &init);
        void execReturn();
        void updateState(const CodeStorage::Block *ofBlock, const SymHeap &);
        void updateState(const CodeStorage::Block *ofBlock);
        void updateState(const CodeStorage::Block *ofBlock, TValueId valDst,
                         TValueId valSrc);
        void execCondInsn();
        void execTermInsn();
        bool execInsnLoop();
        bool execInsn();
        bool execBlock();
};

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine implementation
void SymExecEngine::initEngine(const SymHeap &init)
{
    // look for fnc name
    const CodeStorage::Fnc *fnc = bt_.topFnc();
    const std::string &fncName = nameOf(*fnc);
    lw_ = &fnc->def.loc;
    CL_DEBUG_MSG(lw_, ">>> entering " << fncName << "()");

    // look for the entry block
    TBlock entry = fnc->cfg.entry();
    if (!entry) {
        CL_ERROR_MSG(lw_, fncName << ": " << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    SymHeapUnion &huni = stateMap_[entry];
    huni.insert(init);

    // schedule the entry block for processing
    todo_.insert(entry);
}

void SymExecEngine::execReturn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &opList = insn->operands;
    if (1 != opList.size())
        TRAP;

    SymHeap heap(localState_[heapIdx_]);

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymHeapProcessor proc(heap, &bt_);
        proc.setLocation(lw_);

        const TValueId val = proc.heapValFromOperand(src);
        if (VAL_INVALID == val)
            TRAP;

        proc.heapSetVal(OBJ_RETURN, val);
    }

    // commit one of the function results
    dst_.insert(heap);
}

void SymExecEngine::updateState(const CodeStorage::Block *ofBlock,
                                const SymHeap &heap)
{
    SymHeap h(heap); // clone
    Abstract(h);

    // update *target* state
    SymHeapUnion &huni = stateMap_[ofBlock];
    const size_t last = huni.size();
    huni.insert(h);

    const std::string &name = ofBlock->name();

    // check if anything has changed
    if (huni.size() == last) {
        CL_DEBUG_MSG(lw_, "--- block " << name << " left intact");

    } else {
        const size_t last = todo_.size();

        // schedule for next wheel (if not already)
        todo_.insert(ofBlock);

        const bool already = (todo_.size() == last);
        CL_DEBUG_MSG(lw_, ((already) ? "-+-" : "+++")
                << " block " << name
                << ((already)
                    ? " changed, but already scheduled"
                    : " scheduled for next wheel"));
    }
}

void SymExecEngine::updateState(const CodeStorage::Block *ofBlock) {
    const SymHeap &heap = localState_[heapIdx_];
    this->updateState(ofBlock, heap);
}

void SymExecEngine::updateState(const CodeStorage::Block *ofBlock,
                                TValueId valDst, TValueId valSrc)
{
    SymHeap heap(localState_[heapIdx_]);
    heap.valReplaceUnknown(valDst, valSrc);
    this->updateState(ofBlock, heap);
}

void SymExecEngine::execCondInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &oplist = insn->operands;
    const CodeStorage::TTargetList &tlist = insn->targets;
    if (2 != tlist.size() || 1 != oplist.size())
        TRAP;

    // IF (operand) GOTO target0 ELSE target1

    const SymHeap &heap = localState_[heapIdx_];
    SymHeapProcessor proc(const_cast<SymHeap &>(heap), &bt_);
    proc.setLocation(lw_);

    const TValueId val = proc.heapValFromOperand(oplist[0]);
    switch (val) {
        case VAL_TRUE:
            CL_DEBUG_MSG(lw_, ".T. CL_INSN_COND got VAL_TRUE");
            this->updateState(tlist[/* then label */ 0]);
            return;

        case VAL_FALSE:
            CL_DEBUG_MSG(lw_, ".F. CL_INSN_COND got VAL_FALSE");
            this->updateState(tlist[/* else label */ 1]);
            return;

        default:
            break;
    }

    // operand value is unknown, go to both targets

    const EUnknownValue code = heap.valGetUnknown(val);
    switch (code) {
        case UV_UNKNOWN:
            CL_DEBUG_MSG(lw_, "??? CL_INSN_COND got VAL_UNKNOWN");
            break;

        case UV_UNINITIALIZED:
            CL_WARN_MSG(lw_,
                    "conditional jump depends on uninitialized value");
            bt_.printBackTrace();
            break;

        case UV_DEREF_FAILED:
            // error should have been already emitted
            CL_DEBUG_MSG(lw_, "ignored VAL_DEREF_FAILED");
            break;

        case UV_KNOWN:
            TRAP;
            return;
    }

    this->updateState(tlist[/* then label */ 0], val, VAL_TRUE);
    this->updateState(tlist[/* else label */ 1], val, VAL_FALSE);
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
            break;

        case CL_INSN_JMP:
            if (1 == tlist.size()) {
                this->updateState(tlist[0]);
                break;
            }
            // go through!

        default:
            TRAP;
    }
}

bool /* handled */ SymExecEngine::execInsnLoop() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const SymHeap &src = localState_[heapIdx_];

    std::list<SymHeap> todo;    // we can add concretized heaps during execution
    todo.push_back(src);        // first heap to analyze
    while(todo.size()>0) {
        SymHeap &workingHeap = todo.front(); // use first SH

        SymHeapProcessor proc(workingHeap, &bt_);
        proc.setLocation(lw_);

        if (!proc.exec(nextLocalState_, *insn, params_.fastMode)) {
            // assume CL_INSN_CALL
            if (1 != todo.size())
                TRAP;

            return false;
        }

        todo.pop_front();
        proc.splice(todo);      // add concretized variants
    }

    return true;
}

bool /* complete */ SymExecEngine::execInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // true for terminal instruction
    const bool isTerm = cl_is_term_insn(insn->code);

    if (!heapIdx_) {
        // let's begin with empty resulting heap union
        // TODO: implement SymHeapUnion::clear()
        nextLocalState_ = SymHeapUnion();
        CL_DEBUG_MSG(lw_, "!!! executing insn #" << insnIdx_);
    }

    // go through the remainder of symbolic heaps corresponding to localState_
    const unsigned hCnt = localState_.size();
    for (/* we allow resume */; heapIdx_ < hCnt; ++heapIdx_) {
        if (localState_.isDone(heapIdx_))
            // for this particular symbolic heap, we already know the result and
            // the result is already included in the resulting state, skip it
            continue;

        if (1 < hCnt) {
            CL_DEBUG_MSG(lw_, "*** processing heap #" << heapIdx_
                    << " of BB " << block_->name());
        }

        if (isTerm) {
            // terminal insn
            this->execTermInsn();
            continue;
        }

        if (this->execInsnLoop())
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
    // state valid for the entry of this BB
    SymHeapScheduler &origin = stateMap_[block_];
    const int origCnt = origin.size();

    if (!insnIdx_)
        // fresh run, let's initialize the local state by BB entry
        localState_ = origin;

    // go through the remainder of BB insns
    for (; insnIdx_ < block_->size(); ++insnIdx_) {

        const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
        if (0 < insn->loc.line)
            // update location info
            lw_ = &insn->loc;

        // execute current instruction on localState
        if (!this->execInsn())
            // fucntion call reached, we should stand by
            return false;

        // swap states in order to be ready for next insn
        localState_ = nextLocalState_;
    }

    // Mark symbolic heaps that have been processed as done. They will be
    // omitted on the next call of execBlock() for the same BB since there is no
    // chance to get different results for the same symbolic heaps on the input.
    // NOTE: We don't know whether origCnt == origin.size() at this point, but
    //       only                  origCnt <= origin.size()
    for (int h = 0; h < origCnt; ++h)
        origin.setDone(h);

    // the whole block should be processed now
    insnIdx_ = 0;
    return true;
}

bool /* complete */ SymExecEngine::run() {
    const CodeStorage::Fnc *fnc = bt_.topFnc();
    const LocationWriter lw(&fnc->def.loc);

    if (waiting_) {
        // we're on the way from a just completed function call...
        if (!this->execBlock())
            // ... and we've just hit another one
            return false;
    }
    else {
        // we have a fresh instance of SymExecEngine
        waiting_ = true;
        if (1 != todo_.size())
            // protocol error
            TRAP;
    }

    // main loop of SymExecEngine
    while (!todo_.empty()) {
        // FIXME: take BBs in some reasonable order instead
        TBlockSet::iterator i = todo_.begin();
        block_ = *i;
        todo_.erase(i);

        const std::string &name = block_->name();
        const CodeStorage::Insn *first = block_->operator[](0);
        const LocationWriter lw(&first->loc);
        CL_DEBUG_MSG(lw_, "___ entering " << name);
        insnIdx_ = 0;
        heapIdx_ = 0;

        // process the basic block until the first function call
        if (!this->execBlock())
            // function call reached, suspend the execution for now
            return false;
    }

    // we should be done with this function
    CL_DEBUG_MSG(lw, "<<< leaving " << nameOf(*fnc) << "()");
    waiting_ = false;
    return true;
}

const SymHeap* SymExecEngine::callEntry() const {
    if (heapIdx_ < 1)
        TRAP;

    return &localState_[heapIdx_ - /* already incremented for next wheel*/ 1];
}

const CodeStorage::Insn* SymExecEngine::callInsn() const {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    if (CL_INSN_CALL != insn->code)
        // protocol error
        TRAP;

    return insn;
}

SymHeapUnion* SymExecEngine::callResults() {
    return &nextLocalState_;
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct StackItem {
    SymCallCtx      *ctx;
    SymExecEngine   *engine;
    SymHeapUnion    *results;
};

struct SymExec::Private {
    SymExec                     &se;
    const CodeStorage::Storage  &stor;
    SymExecParams               params;
    SymHeapUnion                stateZero;
    SymBackTrace                bt;
    SymCallCache                callCache;

    Private(SymExec &se_, const CodeStorage::Storage &stor_):
        se(se_),
        stor(stor_),
        bt(stor, /* no root for now */ -1),
        callCache(&bt)
    {
    }

    const CodeStorage::Fnc* resolveCallInsn(SymHeap                     entry,
                                            const CodeStorage::Insn     &insn,
                                            SymHeapUnion                &dst);

    void execRoot(const StackItem &item);
};

SymExec::SymExec(const CodeStorage::Storage &stor, const SymExecParams &params):
    d(new Private(*this, stor))
{
    d->params = params;

    // create the initial state, consisting of global/static variables
    SymHeap init;
    createGlVars(init, stor);
    d->stateZero.insert(init);
}

SymExec::~SymExec() {
    delete d;
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
        SymHeapUnion                &results)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (CL_INSN_CALL != insn.code || opList.size() < 2)
        TRAP;

    // look for Fnc ought to be called
    SymHeapProcessor proc(heap, &this->bt);
    const LocationWriter lw(&insn.loc);
    proc.setLocation(lw);
    const int uid = proc.fncFromOperand(opList[/* fnc */ 1]);
    const CodeStorage::Fnc *fnc = this->stor.fncs[uid];
    if (!fnc)
        // unable to resolve Fnc by UID
        TRAP;

    if (SE_MAX_CALL_DEPTH < this->bt.size()) {
        CL_ERROR_MSG(lw, "call depth exceeds SE_MAX_CALL_DEPTH"
                << " (" << SE_MAX_CALL_DEPTH << ")");
        goto fail;
    }

    if (CL_OPERAND_VOID == fnc->def.code) {
        CL_WARN_MSG(lw, "ignoring call of undefined function");
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
        SymHeapProcessor proc(heap, &this->bt);
        proc.setLocation(this->bt.topCallLoc());

        // set return value to unknown
        const TValueId val = heap.valCreateUnknown(UV_UNKNOWN, dst.type);
        const TObjId obj = proc.heapObjFromOperand(dst);
        heap.objSetValue(obj, val);
    }

    // call failed, so that we have exactly one resulting heap
    results.insert(heap);
    return 0;
}

void SymExec::Private::execRoot(const StackItem &item) {
    using CodeStorage::Fnc;

    // run-time stack
    typedef std::stack<StackItem> TStack;
    TStack rtStack;
    rtStack.push(item);

    // main loop
    while (!rtStack.empty()) {
        const StackItem &item = rtStack.top();

        if (item.engine->run()) {
            // call done at this level
            item.ctx->flushCallResults(*item.results);
            this->bt.popCall();

            // remove top of the stack
            delete item.engine;
            rtStack.pop();
            continue;
        }

        // next call
        const SymHeap &entry = *item.engine->callEntry();
        const CodeStorage::Insn &insn = *item.engine->callInsn();
        const Fnc *fnc = this->resolveCallInsn(entry, insn, *item.results);
        if (!fnc)
            // the error message should have been already emitted
            continue;

        // get call context
        SymCallCtx &ctx = this->callCache.getCallCtx(entry, insn);
        if (!ctx.needExec()) {
            const LocationWriter lw(this->bt.topCallLoc());
            CL_DEBUG_MSG(lw, "(x) call of function optimized out: "
                    << nameOf(*fnc) << "()");

            // use the cached result
            ctx.flushCallResults(*item.engine->callResults());

            // leave backtrace
            this->bt.popCall();
            continue;
        }

        // enter a new function call
        StackItem next;
        next.ctx = &ctx;
        next.engine = new SymExecEngine(this->se, this->bt, ctx.entry(),
                                        ctx.rawResults());
        next.results = item.engine->callResults();
        rtStack.push(next);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc, SymHeapUnion &results) {
    BOOST_FOREACH(const SymHeap &init, d->stateZero) {
        if (d->bt.size())
            // *** bt offset detected ***
            TRAP;

        // FIXME: suboptimal inteface of SymBackTrace
        // TODO: implement something like SymBackTrace::declareRoot()
        d->bt = SymBackTrace(d->stor, uidOf(fnc));

        // XXX: synthesize CL_INSN_CALL
        CodeStorage::Insn insn;
        insn.stor = fnc.stor;
        insn.code = CL_INSN_CALL;
        insn.loc  = fnc.def.loc;
        insn.operands.resize(2);
        insn.operands[1] = fnc.def;

        // get call context
        SymCallCtx &ctx = d->callCache.getCallCtx(init, insn);
        if (!ctx.needExec()) {
            CL_WARN_MSG(d->bt.topCallLoc(), "(x) root function optimized out: "
                    << nameOf(fnc) << "()");

            ctx.flushCallResults(results);
            continue;
        }

        // root stack item
        StackItem si;
        si.ctx = &ctx;
        si.engine = new SymExecEngine(*this, d->bt, ctx.entry(),
                                      ctx.rawResults());
        si.results = &results;

        // root call
        d->execRoot(si);
    }
}
