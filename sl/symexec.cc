/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "symexec.hh"

#include "btprint.hh"
#include "cl_private.hh"
#include "storage.hh"
#include "symproc.hh"
#include "symstate.hh"

#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifndef DEBUG_SE_STACK_FRAME
#   define DEBUG_SE_STACK_FRAME 0
#endif

// utilities
namespace {
    void createStackFrame(SymbolicHeap::SymHeap &heap,
                          const CodeStorage::Fnc &fnc)
    {
        using CodeStorage::Var;
#if DEBUG_SE_STACK_FRAME
        LocationWriter lw(&fnc.def.loc);
        CL_DEBUG_MSG(lw,
                ">>> creating stack frame for " << nameOf(fnc) << "():");
#endif

        BOOST_FOREACH(const Var &var, fnc.vars) {
#if DEBUG_SE_STACK_FRAME
            lw = &var.loc;
            CL_DEBUG_MSG(lw, ">>> creating stack variable: #" << var.uid
                    << " (" << var.name << ")" );
#endif

            heap.varCreate(var.clt, var.uid);
        }
    }

    void setCallArgs(SymbolicHeap::SymHeap &heap,
                     const CodeStorage::Fnc &fnc,
                     const CodeStorage::TOperandList &opList)
    {
        // get called fnc's args
        const CodeStorage::TArgByPos &args = fnc.args;
        if (args.size() + 2 != opList.size())
            TRAP;

        // set args' values
        int pos = /* dst + fnc */ 2;
        SymHeapProcessor proc(heap);
        BOOST_FOREACH(int arg, args) {
            const struct cl_operand &op = opList[pos++];
            const Location last(&fnc.def.loc);
            const LocationWriter lw(&op.loc, &last);
            proc.setLocation(lw);

            const int val = proc.heapValFromOperand(op);
            if (SymbolicHeap::VAL_INVALID == val)
                TRAP;

            const int lhs = heap.varByCVar(arg);
            if (SymbolicHeap::OBJ_INVALID == lhs)
                TRAP;

            // set arg's value
            proc.heapSetVal(lhs, val);
        }
    }

    void assignReturnValue(SymbolicHeap::SymHeap &heap,
                           const struct cl_operand &op)
    {
        using namespace SymbolicHeap;

        if (CL_OPERAND_VOID == op.code)
            // fnc returns void, thus we are done
            return;

        SymHeapProcessor proc(heap);
        proc.setLocation(&op.loc);

        const int obj = proc.heapVarFromOperand(op);
        if (OBJ_INVALID == obj)
            TRAP;

        const int val = heap.valueOf(OBJ_RETURN);
        if (VAL_INVALID == val)
            TRAP;

        // we are ready to assign the return value
        heap.objSetValue(obj, val);
    }

    void assignReturnValue(SymHeapUnion &state,
                           const struct cl_operand &op)
    {
        if (CL_OPERAND_VOID == op.code)
            return;

        // go through results and perform assignment of the return value
        BOOST_FOREACH(SymbolicHeap::SymHeap &res, state) {
            assignReturnValue(res, op);
        }
    }

    void destroyStackFrame(IBtPrinter *bt, SymbolicHeap::SymHeap &heap,
                           const CodeStorage::Fnc &fnc)
    {
        using CodeStorage::Var;
        SymHeapProcessor proc(heap, bt);

        BOOST_FOREACH(const Var &var, fnc.vars) {
            const LocationWriter lw(&var.loc);
#if DEBUG_SE_STACK_FRAME
            CL_DEBUG_MSG(lw, "<<< destroying stack variable: #"
                    << var.uid << " (" << var.name << ")" );
#endif

            const int obj = heap.varByCVar(var.uid);
            if (obj < 0)
                TRAP;

            proc.setLocation(lw);
            proc.destroyObj(obj);
        }

        heap.objDestroy(SymbolicHeap::OBJ_RETURN);
    }

    void destroyStackFrame(IBtPrinter *bt, SymHeapUnion &huni,
                           const CodeStorage::Fnc &fnc)
    {
        using SymbolicHeap::SymHeap;

#if DEBUG_SE_STACK_FRAME
        const LocationWriter lw(&fnc.def.loc);
        CL_DEBUG_MSG(lw, "<<< destroying stack frame of "
                << nameOf(fnc) << "():");

        int hCnt = 0;
#endif

        BOOST_FOREACH(SymHeap &heap, huni) {
#if DEBUG_SE_STACK_FRAME
            CL_DEBUG_MSG(lw, "*** destroying stack frame in result #"
                    << (++hCnt));
#endif
            destroyStackFrame(bt, heap, fnc);
        }
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct SymExec::Private: public IBtPrinter {
    typedef std::set<int /* fnc uid */>                         TBtSet;
    typedef std::pair<const CodeStorage::Fnc *, LocationWriter> TBtStackItem;
    typedef std::stack<TBtStackItem>                            TBtStack;
    typedef std::set<const CodeStorage::Block *>                TBlockSet;
    typedef std::map<const CodeStorage::Block *, SymHeapUnion>  TStateMap;

    CodeStorage::Storage        &stor;
    TBtSet                      *btSet;
    TBtStack                    *btStack;
    const CodeStorage::Fnc      *fnc;
    const CodeStorage::Block    *bb;
    const CodeStorage::Insn     *insn;
    LocationWriter              lw;
    SymHeapUnion                stateZero;
    TStateMap                   state;
    TBlockSet                   todo;
    SymHeapUnion                *results;
    bool                        fastMode;

    Private(CodeStorage::Storage &stor_):
        stor(stor_),
        btSet(0),
        btStack(0),
        fnc(0),
        bb(0),
        insn(0),
        results(0),
        fastMode(false)
    {
    }

    // IBtPrinter implementation
    virtual void printBackTrace();

    void execReturn(SymbolicHeap::SymHeap heap);
    void updateState(const CodeStorage::Block *ofBlock,
                     const SymbolicHeap::SymHeap &heap);
    void execCondInsn(const SymbolicHeap::SymHeap &heap);
    void execTermInsn(const SymbolicHeap::SymHeap &heap);
    int resolveCallee(const SymbolicHeap::SymHeap &heap,
                      const struct cl_operand &op);
    void execCallInsn(const CodeStorage::Fnc *fnc,
                      const SymbolicHeap::SymHeap &heap,
                      SymHeapUnion &results);
    void execCallInsn(SymbolicHeap::SymHeap heap, SymHeapUnion &results);
    void execInsn(SymHeapUnion &localState);
    void execBb();
    void execFncBody();
    void execFnc(const SymbolicHeap::SymHeap &init);
};

SymExec::SymExec(CodeStorage::Storage &stor):
    d(new Private(stor))
{
    using SymbolicHeap::SymHeap;

    // TODO: create and initialize global/static variables (stateZero)
    d->stateZero.insert(SymHeap());
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

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using CodeStorage::Var;
    using SymbolicHeap::SymHeap;

    // wait, avoid recursion in the first place
    Private::TBtSet btSet;
    btSet.insert(uidOf(fnc));
    d->btSet = &btSet;

    // we would also like to print backtraces
    Private::TBtStack btStack;
    push(btStack, &fnc, &fnc.def.loc);
    d->btStack = &btStack;

    // set function ought to be executed
    d->fnc = &fnc;

    // container for resulting state
    SymHeapUnion results;
    d->results = &results;

    BOOST_FOREACH(const SymHeap &zero, d->stateZero) {
        SymHeap init(zero);
        createStackFrame(init, fnc);

        // well, we have successfully prepared the initial state,
        // now please execute the function!
        d->execFnc(init);
    }

    // TODO: process return value
    destroyStackFrame(d, results, fnc);

    // TODO: process results somehow (generate points-to graph, etc.)
}

void SymExec::Private::printBackTrace() {
    using namespace CodeStorage;

    const TBtStack &ref = *this->btStack;
    if (ref.size() < 2)
        return;

    TBtStack bt(ref);
    while (!bt.empty()) {
        const Fnc *btFnc;
        LocationWriter btLw;
        boost::tie(btFnc, btLw) = bt.top();
        bt.pop();

        CL_NOTE_MSG(btLw, "from call of " << nameOf(*btFnc) << "()");
    }
}

void SymExec::Private::execReturn(SymbolicHeap::SymHeap heap)
{
    using namespace SymbolicHeap;

    const CodeStorage::TOperandList &opList = insn->operands;
    if (1 != opList.size())
        TRAP;

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymHeapProcessor proc(heap, this);
        proc.setLocation(this->lw);

        const int val = proc.heapValFromOperand(src);
        if (VAL_INVALID == val)
            TRAP;

        proc.heapSetVal(OBJ_RETURN, val);
    }

    SymHeapUnion &res = *(this->results);
    res.insert(heap);
}

void SymExec::Private::updateState(const CodeStorage::Block *ofBlock,
                                   const SymbolicHeap::SymHeap &heap)
{
    // update *target* state
    SymHeapUnion &huni = this->state[ofBlock];
    const size_t last = huni.size();
    huni.insert(heap);

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
        
void SymExec::Private::execCondInsn(const SymbolicHeap::SymHeap &heap)
{
    using namespace SymbolicHeap;

    const CodeStorage::TOperandList &oplist = insn->operands;
    const CodeStorage::TTargetList &tlist = insn->targets;
    if (2 != tlist.size() || 1 != oplist.size())
        TRAP;

    SymHeapProcessor proc(const_cast<SymHeap &>(heap), this);
    proc.setLocation(this->lw);

    const int val = proc.heapValFromOperand(oplist[0]);
    switch (val) {
        case VAL_TRUE:
            CL_DEBUG_MSG(this->lw, ".T. CL_INSN_COND got VAL_TRUE");
            this->updateState(tlist[/* then label */ 0], heap);
            break;

        case VAL_FALSE:
            CL_DEBUG_MSG(this->lw, ".F. CL_INSN_COND got VAL_FALSE");
            this->updateState(tlist[/* else label */ 1], heap);
            break;

        case VAL_UNINITIALIZED:
            CL_WARN_MSG(this->lw,
                    "conditional jump depends on uninitialized value");
            this->printBackTrace();
            // fall through!

        case VAL_UNKNOWN:
            CL_DEBUG_MSG(this->lw, "??? CL_INSN_COND got VAL_UNKNOWN");
            // TODO: check for inconsistency here!
            // TODO: set val to VAL_TRUE in target 0
            // TODO: set val to VAL_FALSE in target 1
            this->updateState(tlist[/* then label */ 0], heap);
            this->updateState(tlist[/* else label */ 1], heap);
            break;

        case VAL_DEREF_FAILED:
            // error should have been already emitted
            CL_DEBUG_MSG(this->lw, "ignored VAL_DEREF_FAILED");
            break;

        default:
            // TODO: handle special values here
            TRAP;
    }
}

void SymExec::Private::execTermInsn(const SymbolicHeap::SymHeap &heap)
{
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

int SymExec::Private::resolveCallee(const SymbolicHeap::SymHeap &heap,
                                    const struct cl_operand &op)
{
    using namespace SymbolicHeap;

    if (CL_OPERAND_CST == op.code) {
        // direct call
        const struct cl_cst &cst = op.data.cst;
        if (CL_TYPE_FNC != cst.code)
            TRAP;

        return cst.data.cst_fnc.uid;

    } else {
        // indirect call
        SymHeapProcessor proc(const_cast<SymHeap &>(heap), this);
        proc.setLocation(this->lw);

        const int val = proc.heapValFromOperand(op);
        if (VAL_INVALID == val)
            // Oops, it does not look as indirect call actually
            TRAP;

        return heap.valGetCustom(/* TODO: check type */ 0, val);
    }
}

void SymExec::Private::execCallInsn(const CodeStorage::Fnc *fnc,
                                    const SymbolicHeap::SymHeap &heap,
                                    SymHeapUnion &results)
{
    const int uid = uidOf(*fnc);

    // avoid recursion
    this->btSet->insert(uid);

    // FIXME: this approach will sooner or later cause a stack overflow
    // TODO: use an explicit stack instead
    Private subExec(this->stor);
    subExec.btSet       = this->btSet;
    subExec.btStack     = this->btStack;
    subExec.fastMode    = this->fastMode;
    subExec.fnc         = fnc;
    subExec.results     = &results;

    // run!
    subExec.execFnc(heap);
    if (1 != this->btSet->erase(uid))
        TRAP;
}

void SymExec::Private::execCallInsn(SymbolicHeap::SymHeap heap,
                                    SymHeapUnion &results)
{
    using namespace CodeStorage;
    using namespace SymbolicHeap;

    const TOperandList &opList = insn->operands;
    if (CL_INSN_CALL != insn->code || opList.size() < 2)
        TRAP;

    // look for lhs
    const struct cl_operand &dst = opList[0];
    const bool haveLhs = (CL_OPERAND_VOID != dst.code);

    // look for Fnc ought to be called
    const int uid = this->resolveCallee(heap, opList[/* fnc */ 1]);
    const Fnc *fnc = this->stor.anyFncById[uid];
    if (!fnc)
        // unable to resolve Fnc by UID
        TRAP;

    // FIXME: this may stop working once we decide for hash or tree container
    SymHeapUnion tmp;

    if (hasKey(this->btSet, uid)) {
        // *** recursion detected ***
        CL_ERROR_MSG(this->lw,
                "call recursion detected, cg subtree will be excluded");
        goto fail;
    }

    if (CL_OPERAND_VOID == fnc->def.code) {
        CL_WARN_MSG(this->lw, "ignoring call of undefined function");
        goto fail;
    }

    // we are ready to call a function, change backtrace stack accordingly
    push(this->btStack, fnc, this->lw);

    // create an object for return value (if needed)
    if (haveLhs) {
        // FIXME: improve the interface of SymHeap for the return value
        heap.objDestroy(OBJ_RETURN);
        heap.varDefineType(OBJ_RETURN, dst.type);
    }

    // crate local variables of called fnc
    // TODO: wrap createStackFrame/destroyStackFrame to an object?
    createStackFrame(heap, *fnc);
    setCallArgs(heap, *fnc, opList);

    // now please perform the call
    this->execCallInsn(fnc, heap, tmp);

    // go through results and perform assignment of the return value
    assignReturnValue(tmp, dst);

    // final cleanup
    destroyStackFrame(this, tmp, *fnc);
    this->btStack->pop();

    // we are done, merge call results
    results.insert(tmp);
    return;

fail:
    // something wrong happened
    this->printBackTrace();

    if (haveLhs) {
        // set return value to unknown
        SymHeapProcessor proc(heap, this);
        proc.setLocation(this->lw);

        const int obj = proc.heapVarFromOperand(dst);
        heap.objSetValue(obj, VAL_UNKNOWN);
    }

    // call failed, so that we have exactly one resulting heap
    results.insert(heap);
}

void SymExec::Private::execInsn(SymHeapUnion &localState) {
    using SymbolicHeap::SymHeap;

    // true for terminal instruction
    const bool isTerm = cl_is_term_insn(insn->code);

    // let's begin with empty resulting heap union
    SymHeapUnion nextLocalState;

    // go through all symbolic heaps corresponding to localState
    int hCnt = 0;
    BOOST_FOREACH(const SymHeap &heap, localState) {
        if (1 < localState.size()) {
            CL_DEBUG_MSG(this->lw, "*** processing heap #"
                    << (++hCnt) << " of BB " << bb->name());
        }

        if (isTerm) {
            // terminal insn
            this->execTermInsn(heap);

        } else {
            // working area for non-term instructions
            SymHeap workingHeap(heap);
            SymHeapProcessor proc(workingHeap, this);
            proc.setLocation(this->lw);

            // NOTE: this has to be tried *before* execCallInsn() to eventually
            // catch malloc()/free() calls, which are treated differently
            if (!proc.exec(nextLocalState, *insn, this->fastMode))
                // call insn
                this->execCallInsn(workingHeap, nextLocalState);
        }
    }

    if (!isTerm)
        // save result for next instruction
        localState = nextLocalState;
}

void SymExec::Private::execBb() {
    using namespace CodeStorage;
    using SymbolicHeap::SymHeap;

    const std::string &name = bb->name();
    CL_DEBUG_MSG(lw, "___ entering " << name);

    // this state will be changed per each instruction
    // NOTE: it may grow significantly on any CL_INSN_CALL instruction
    SymHeapUnion localState(this->state[this->bb]);

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

void SymExec::Private::execFnc(const SymbolicHeap::SymHeap &init)
{
    using namespace CodeStorage;
    using SymbolicHeap::SymHeap;

    // look for fnc name
    const std::string &fncName = nameOf(*this->fnc);
    this->lw = &fnc->def.loc;
    CL_DEBUG_MSG(this->lw, ">>> entering " << fncName << "()");

    // look for entry block
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
