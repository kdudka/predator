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
// SymExec implementation
struct SymExec::Private {
    typedef std::set<int /* fnc uid */>                            TBtSet;
    typedef std::set<const CodeStorage::Block *>                   TBlockSet;
    typedef std::map<const CodeStorage::Block *, SymHeapScheduler> TStateMap;

    const CodeStorage::Storage  &stor;
    TBtSet                      *btSet;
    SymBackTrace                *btStack;
    const CodeStorage::Fnc      *fnc;
    const CodeStorage::Block    *bb;
    const CodeStorage::Insn     *insn;
    LocationWriter              lw;
    SymHeapUnion                stateZero;
    TStateMap                   state;
    TBlockSet                   todo;
    SymHeapUnion                *results;
    SymCallCache                *callCache;
    bool                        fastMode;

    Private(const CodeStorage::Storage &stor_);
    Private(const Private &par, const CodeStorage::Fnc &fnc, SymHeapUnion &res);

    void execReturn(SymHeap heap);
    void updateState(const CodeStorage::Block *ofBlock, const SymHeap &heap);
    void updateState(const CodeStorage::Block *ofBlock, SymHeap heap,
                     TValueId valDst, TValueId valSrc);
    void execCondInsn(const SymHeap &heap);
    void execTermInsn(const SymHeap &heap);
    void execCallFailed(SymHeap heap, SymHeapUnion &results,
                        const struct cl_operand &dst);
    void execInsnCall(const SymHeap &heap, SymHeapUnion &results, std::list<SymHeap> &todo );
    void execInsn(SymHeapScheduler &localState);
    void execBb();
    void execFncBody();
    void execFnc(const SymHeap &init);
};

SymExec::Private::Private(const CodeStorage::Storage &stor_):
    stor        (stor_),
    btSet       (0),
    btStack     (0),
    fnc         (0),
    bb          (0),
    insn        (0),
    results     (0),
    callCache   (0),
    fastMode    (false)
{
}

SymExec::Private::Private(const Private &parent, const CodeStorage::Fnc &fnc,
                          SymHeapUnion &res)
:
    stor        (parent.stor),
    btSet       (parent.btSet),
    btStack     (parent.btStack),
    fnc         (&fnc),
    bb          (0),
    insn        (0),
    results     (&res),
    callCache   (parent.callCache),
    fastMode    (parent.fastMode)
{
}

SymExec::SymExec(const CodeStorage::Storage &stor):
    d(new Private(stor))
{
    // create the initial state, consisting of global/static variables
    SymHeap init;
    createGlVars(init, stor);
    d->stateZero.insert(init);
}

SymExec::~SymExec() {
    delete d;
}

bool SymExec::fastMode() const {
    return d->fastMode;
}

void SymExec::setFastMode(bool val) {
    d->fastMode = val;
}

void SymExec::exec(const CodeStorage::Fnc &fnc, SymHeapUnion &results) {
    using CodeStorage::Var;

    // set function ought to be executed
    d->fnc = &fnc;

    // wait, avoid recursion in the first place
    Private::TBtSet btSet;
    const int fncId = uidOf(fnc);
    btSet.insert(fncId);
    d->btSet = &btSet;

    // backtrace shared among all instances of SymExec::Private
    SymBackTrace btStack(d->stor, fncId);
    d->btStack = &btStack;

    // call cache
    SymCallCache callCache(d->btStack);
    d->callCache = &callCache;

    BOOST_FOREACH(const SymHeap &init, d->stateZero) {
        // XXX: synthesize CL_INSN_CALL
        CodeStorage::Insn insn;
        insn.stor = fnc.stor;
        insn.code = CL_INSN_CALL;
        insn.loc  = fnc.def.loc;
        insn.operands.resize(2);
        insn.operands[1] = fnc.def;

        // create call context
        SymCallCtx &ctx = callCache.getCallCtx(init, insn);
        if (ctx.needExec()) {
            d->results = &ctx.rawResults();

            // now perform the call!
            d->execFnc(ctx.entry());
        }

        // merge call results
        ctx.flushCallResults(results);
    }
}

void SymExec::Private::execReturn(SymHeap heap)
{
    const CodeStorage::TOperandList &opList = insn->operands;
    if (1 != opList.size())
        TRAP;

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymHeapProcessor proc(heap, this->btStack);
        proc.setLocation(this->lw);

        const TValueId val = proc.heapValFromOperand(src);
        if (VAL_INVALID == val)
            TRAP;

        proc.heapSetVal(OBJ_RETURN, val);
    }

    SymHeapUnion &res = *(this->results);
    res.insert(heap);
}

void SymExec::Private::updateState(const CodeStorage::Block *ofBlock,
                                   const SymHeap &heap)
{
    SymHeap h(heap); // clone
    Abstract(h);
    // update *target* state
    SymHeapUnion &huni = this->state[ofBlock];
    const size_t last = huni.size();
    huni.insert(h);

    const std::string &name = ofBlock->name();

    // check if anything has changed
    if (huni.size() == last) {
        CL_DEBUG_MSG(this->lw, "--- block " << name << " left intact");

    } else {
        const size_t last = this->todo.size();

        // schedule for next wheel (if not already)
        this->todo.insert(ofBlock);

        const bool already = (this->todo.size() == last);
        CL_DEBUG_MSG(this->lw, ((already) ? "-+-" : "+++")
                << " block " << name
                << ((already)
                    ? " changed, but already scheduled"
                    : " scheduled for next wheel"));
    }
}

void SymExec::Private::updateState(const CodeStorage::Block *ofBlock,
                                   SymHeap heap, TValueId valDst,
                                   TValueId valSrc)
{
    heap.valReplaceUnknown(valDst, valSrc);
    this->updateState(ofBlock, heap);
}

void SymExec::Private::execCondInsn(const SymHeap &heap) {
    const CodeStorage::TOperandList &oplist = insn->operands;
    const CodeStorage::TTargetList &tlist = insn->targets;
    if (2 != tlist.size() || 1 != oplist.size())
        TRAP;

    // IF (operand) GOTO target0 ELSE target1

    SymHeapProcessor proc(const_cast<SymHeap &>(heap), this->btStack);
    proc.setLocation(this->lw);

    const TValueId val = proc.heapValFromOperand(oplist[0]);
    switch (val) {
        case VAL_TRUE:
            CL_DEBUG_MSG(this->lw, ".T. CL_INSN_COND got VAL_TRUE");
            this->updateState(tlist[/* then label */ 0], heap);
            return;

        case VAL_FALSE:
            CL_DEBUG_MSG(this->lw, ".F. CL_INSN_COND got VAL_FALSE");
            this->updateState(tlist[/* else label */ 1], heap);
            return;

        default:
            break;
    }

    // operand value is unknown, go to both targets

    const EUnknownValue code = heap.valGetUnknown(val);
    switch (code) {
        case UV_UNKNOWN:
            CL_DEBUG_MSG(this->lw, "??? CL_INSN_COND got VAL_UNKNOWN");
            break;

        case UV_UNINITIALIZED:
            CL_WARN_MSG(this->lw,
                    "conditional jump depends on uninitialized value");
            this->btStack->printBackTrace();
            break;

        case UV_DEREF_FAILED:
            // error should have been already emitted
            CL_DEBUG_MSG(this->lw, "ignored VAL_DEREF_FAILED");
            break;

        case UV_KNOWN:
            TRAP;
            return;
    }

    this->updateState(tlist[/* then label */ 0], heap, val, VAL_TRUE);
    this->updateState(tlist[/* else label */ 1], heap, val, VAL_FALSE);
}

void SymExec::Private::execTermInsn(const SymHeap &heap) {
    const CodeStorage::TTargetList &tlist = insn->targets;

    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_RET:
            this->execReturn(heap);
            break;

        case CL_INSN_COND:
            this->execCondInsn(heap);
            break;

        case CL_INSN_ABORT:
            CL_DEBUG_MSG(this->lw, "CL_INSN_ABORT reached");
            break;

        case CL_INSN_JMP:
            if (1 == tlist.size()) {
                this->updateState(tlist[0], heap);
                break;
            }
            // go through!

        default:
            TRAP;
    }
}

void SymExec::Private::execCallFailed(SymHeap heap, SymHeapUnion &results,
                                      const struct cl_operand &dst)
{
    // something wrong happened, print the backtrace
    this->btStack->printBackTrace();

    if (CL_OPERAND_VOID != dst.code) {
        SymHeapProcessor proc(heap, this->btStack);
        proc.setLocation(this->lw);

        // set return value to unknown
        const TValueId val = heap.valCreateUnknown(UV_UNKNOWN, dst.type);
        const TObjId obj = proc.heapObjFromOperand(dst);
        heap.objSetValue(obj, val);
    }

    // call failed, so that we have exactly one resulting heap
    results.insert(heap);
}

// TODO: add concretization to argument evaluation (depends on allowed form of arguments)
void SymExec::Private::execInsnCall(const SymHeap &heap, SymHeapUnion &results, std::list<SymHeap> &todo)
{
    using namespace CodeStorage;
    const TOperandList &opList = insn->operands;
    if (CL_INSN_CALL != insn->code || opList.size() < 2)
        TRAP;

    // look for Fnc ought to be called
    SymHeapProcessor proc(const_cast<SymHeap &>(heap), this->btStack);
    proc.setLocation(this->lw);
    const int uid = proc.fncFromOperand(opList[/* fnc */ 1]);
    proc.splice(todo);
    const Fnc *fnc = this->stor.fncs[uid];
    if (!fnc)
        // unable to resolve Fnc by UID
        TRAP;

    if (hasKey(this->btSet, uid)) {
        // *** recursion detected ***
        CL_ERROR_MSG(this->lw,
                "call recursion detected, cg subtree will be excluded");
        this->execCallFailed(heap, results, opList[/* dst */ 0]);
        return;
    }

    if (CL_OPERAND_VOID == fnc->def.code) {
        CL_WARN_MSG(this->lw, "ignoring call of undefined function");
        this->execCallFailed(heap, results, opList[/* dst */ 0]);
        return;
    }

    // enter backtrace
    this->btStack->pushCall(uidOf(*fnc), this->lw);

    // get call context
    SymCallCtx &ctx = this->callCache->getCallCtx(heap, *insn);
    if (!ctx.needExec()) {
        CL_DEBUG_MSG(this->lw, "(x) call of function optimized out");
        ctx.flushCallResults(results);
        return;
    }

    // avoid recursion
    this->btSet->insert(uid);

    // run!
    // FIXME: this approach will sooner or later cause a stack overflow
    // TODO: use an explicit stack instead
    Private subExec(*this, *fnc, ctx.rawResults());
    subExec.execFnc(ctx.entry());
    if (1 != this->btSet->erase(uid))
        TRAP;

    // merge call results
    ctx.flushCallResults(results);

    // leave backtrace
    this->btStack->popCall();
}

void SymExec::Private::execInsn(SymHeapScheduler &localState) {
    // true for terminal instruction
    const bool isTerm = cl_is_term_insn(insn->code);

    // let's begin with empty resulting heap union
    SymHeapUnion nextLocalState;

    // go through all symbolic heaps corresponding to localState
    const int hCnt = localState.size();
    for (int h = 0; h < hCnt; ++h) {
        if (localState.isDone(h))
            // for this particular symbolic heap, we already know the result and
            // the result is already included in the resulting state, skip it
            continue;

        const SymHeap &heap = localState[h];
        if (1 < hCnt) {
            CL_DEBUG_MSG(this->lw, "*** processing heap #" << h
                    << " of BB " << bb->name());
        }

        if (isTerm) {
            // terminal insn
            this->execTermInsn(heap);

        } else {
            // working area for non-term instructions
            SymHeap startHeap(heap);    // clone source heap

            std::list<SymHeap> todo;    // we can add concretized heaps during execution
            todo.push_back(startHeap);  // first heap to analyze
            while(todo.size()>0) {
                SymHeap workingHeap(todo.front()); // use first SH
                todo.pop_front();

                SymHeapProcessor proc(workingHeap, this->btStack);
                proc.setLocation(this->lw);

                // NOTE: this has to be tried *before* execInsnCall() to eventually
                // catch malloc()/free() calls, which are treated differently
                if (!proc.exec(nextLocalState, *insn, this->fastMode))
                    // call insn
                    this->execInsnCall(workingHeap, nextLocalState, todo);
                proc.splice(todo);      // add concretized variants
            }
        }
    }

    if (!isTerm)
        // propagate the result to the entry of next instruction
        localState = nextLocalState;
}

void SymExec::Private::execBb() {
    using namespace CodeStorage;

    const std::string &name = bb->name();
    CL_DEBUG_MSG(lw, "___ entering " << name);

    // state valid for the entry of this BB
    SymHeapScheduler &origin = this->state[this->bb];
    const int origCnt = origin.size();

    // this state will be changed per each instruction
    // NOTE: it may grow significantly on any CL_INSN_CALL instruction
    SymHeapScheduler localState(origin);

    // go through all BB insns
    int iCnt = 0;
    BOOST_FOREACH(const Insn *insn, *(this->bb)) {
        if (0 < insn->loc.line)
            // update location info
            this->lw = &insn->loc;

        // execute current instruction on localState
        CL_DEBUG_MSG(this->lw, "!!! executing insn #" << (++iCnt)
                << " of BB " << name);

        this->insn = insn;
        this->execInsn(localState);
    }

    // Mark symbolic heaps that have been processed as done. They will be
    // omitted on the next call of execBb() for the same BB since there is no
    // chance to get different results for the same symbolic heaps on the input.
    // NOTE: We don't know whether origCnt == origin.size() at this point, but
    //       only                  origCnt <= origin.size()
    for (int h = 0; h < origCnt; ++h)
        origin.setDone(h);
}

void SymExec::Private::execFncBody() {
    if (!this->todo.empty())
        // not implemented yet
        TRAP;

    // start with Fnc entry
    this->todo.insert(this->bb);

    // main loop
    while (!this->todo.empty()) {
        // FIXME: take BBs in some reasonable order instead
        TBlockSet::iterator i = this->todo.begin();
        this->bb = *i;
        this->todo.erase(i);

        this->execBb();
    }
}

void SymExec::Private::execFnc(const SymHeap &init)
{
    using namespace CodeStorage;

    // look for fnc name
    const std::string &fncName = nameOf(*this->fnc);
    this->lw = &fnc->def.loc;
    CL_DEBUG_MSG(this->lw, ">>> entering " << fncName << "()");

    // look for the entry block
    const Block *&entry = this->bb;
    entry = fnc->cfg.entry();
    if (!entry) {
        CL_ERROR_MSG(this->lw, fncName << ": " << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    SymHeapUnion &huni = this->state[entry];
    huni.insert(init);

    // now we are ready for symbolic execution
    this->execFncBody();

    // done
    CL_DEBUG_MSG(this->lw, "<<< leaving " << nameOf(*this->fnc) << "()");
}

// vim: tw=120
