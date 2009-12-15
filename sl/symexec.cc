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

#include "symexec.hh"

#include "cl_private.hh"
#include "storage.hh"
#include "symheap.hh"
#include "symproc.hh"

#include <map>
#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

class SymHeapUnion {
    private:
        typedef std::vector<SymbolicHeap::SymHeap> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef TList::iterator iterator;

    public:
        void insert(const SymbolicHeap::SymHeap &heap);

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator begin() const { return heaps_.begin(); }

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator end()   const { return heaps_.end();   }

        /**
         * return count of object stored in the container
         */
        size_t size()          const { return heaps_.size();  }

        iterator begin()             { return heaps_.begin(); }
        iterator end()               { return heaps_.end();   }

    private:
        TList heaps_;
};

typedef std::set<const CodeStorage::Block *>                TBlockSet;
typedef std::map<const CodeStorage::Block *, SymHeapUnion>  TStateMap;

// utilities
namespace {
    void createStackFrame(SymbolicHeap::SymHeap &heap,
                          const CodeStorage::Fnc &fnc)
    {
        using CodeStorage::Var;
        CL_DEBUG(">>> creating stack frame for " << nameOf(fnc) << "():");

        BOOST_FOREACH(const Var &var, fnc.vars) {
            CL_DEBUG("--- creating stack variable: #" << var.uid
                    << " (" << var.name << ")" );

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
            const int val = proc.heapValFromOperand(op);
            if (SymbolicHeap::VAL_INVALID == val)
                TRAP;

            const int lhs = heap.varByCVar(arg);
            if (SymbolicHeap::OBJ_INVALID == lhs)
                TRAP;

            // set arg's value
            heap.objSetValue(lhs, val);
        }
    }

    void assignReturnValue(SymbolicHeap::SymHeap &heap,
                           const struct cl_operand &op)
    {
        if (CL_OPERAND_VOID == op.code)
            // fnc returns void, thus we are done
            return;

        // FIXME: the following code has never run, use the debugger first
        TRAP;

        SymHeapProcessor proc(heap);
        const int obj = proc.heapVarFromOperand(op);
        if (SymbolicHeap::OBJ_INVALID == obj)
            TRAP;

        const int val = heap.getReturnValue();
        if (SymbolicHeap::VAL_INVALID == val)
            TRAP;

        // we are ready to assign the return value
        heap.objSetValue(obj, val);
    }

    void destroyStackFrame(SymbolicHeap::SymHeap &heap,
                           const CodeStorage::Fnc &fnc)
    {
        using CodeStorage::Var;
        BOOST_FOREACH(const Var &var, fnc.vars) {
            CL_DEBUG("--- destroying stack variable: #" << var.uid
                    << " (" << var.name << ")" );

            const int obj = heap.varByCVar(var.uid);
            if (obj < 0)
                TRAP;

            heap.objDestroy(obj);
        }
    }

    void destroyStackFrame(SymHeapUnion huni, const CodeStorage::Fnc &fnc)
    {
        using SymbolicHeap::SymHeap;
        CL_DEBUG("<<< destroying stack frame for " << nameOf(fnc) << "():");

        int hCnt = 0;
        BOOST_FOREACH(SymHeap &heap, huni) {
            CL_DEBUG("*** destroying stack frame in result #" << (++hCnt));
            destroyStackFrame(heap, fnc);
        }
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymHeapUnion implementation
namespace {
    bool checkNonPosValues(int a, int b) {
        if (0 < a && 0 < b)
            // we'll need to properly compare positive values
            return false;

        // non-positive values always have to match, bail out otherwise
        return a != b;
    }

    template <class TSubst>
    bool matchValues(TSubst &subst, int v1, int v2) {
        if (checkNonPosValues(v1, v2))
            // null vs. non-null, etc.
            return false;

        typename TSubst::iterator iter = subst.find(v1);
        if (iter != subst.end())
            // substitution already defined, check if it applies seamlessly
            return iter->second == v2;

        // define a new substitution
        subst[v1] = v2;
        return true;
    }
}

namespace SymbolicHeap {
    template <class TStack, class TSubst>
    bool dfsCmp(TStack &todo,
                TSubst &valSubst,
                const SymHeap &heap1,
                const SymHeap &heap2)
    {
        // FIXME: not very efficient implementation of DFS
        std::set<int> done;

        // DFS loop
        while (!todo.empty()) {
            int value1, value2;
            boost::tie(value1, value2) = todo.top();
            todo.pop();
            done.insert(value1);

            // TODO: distinguish among SLS and single dynamic variables here
            const int obj1 = heap1.pointsTo(value1);
            const int obj2 = heap2.pointsTo(value2);
            if (checkNonPosValues(obj1, obj2))
                // variable mismatch
                return false;

            // TODO: here handle structured variables
            value1 = heap1.valueOf(obj1);
            value2 = heap2.valueOf(obj2);
            if (!matchValues(valSubst, value1, value2))
                // value mismatch
                return false;

            if (value1 <= 0)
                // no need for next wheel (special values already handled)
                continue;

            if (heap1.valIsCustom(value1))
                // we can't follow fnc pointers by pointsTo() since they are
                // sort of virtual from this ascpect (and we of course do
                // not need to follow them)
                continue;

            if (!hasKey(done, value1))
                // schedule values for next wheel
                todo.push(value1, value2);
        }

        // heaps are equal (isomorphism)
        return true;
    }

    bool operator== (const SymHeap &heap1, const SymHeap &heap2) {
        // DFS stack
        typedef std::pair<int, int> TValuePair;
        typedef std::stack<TValuePair> TValueStack;
        TValueStack dfsStack;

        // value substitution (isomorphism)
        typedef std::map<int, int> TSubst;
        TSubst valSubst;

        // NOTE: we do not check cVars themselves among heaps
        // they are *supposed* to be the same
        SymHeap::TCont cVars;
        heap1.gatherCVars(cVars);
        BOOST_FOREACH(int uid, cVars) {
            const int var1 = heap1.varByCVar(uid);
            const int var2 = heap2.varByCVar(uid);
            if (var1 < 0 || var2 < 0)
                // heap corruption detected
                TRAP;

            // retrieve values of static variables
            const int value1 = heap1.valueOf(var1);
            const int value2 = heap2.valueOf(var2);
            if (!matchValues(valSubst, value1, value2))
                // value mismatch, bail out now
                return false;

            if (value1 <= 0)
                // no need for next wheel (special values already handled)
                continue;

            if (heap1.valIsCustom(value1))
                // read the big comment within dfsStack() to see why this
                // condition is useful/necessary
                continue;

            // schedule for DFS
            dfsStack.push(value1, value2);
        }

        // bad luck, we need to run DFS
        return dfsCmp(dfsStack, valSubst, heap1, heap2);
    }
}

void SymHeapUnion::insert(const SymbolicHeap::SymHeap &heap) {
    using SymbolicHeap::SymHeap;

    // FIXME: not very efficient implementation of union :-)
    // TODO: implement the container as either hash or tree data structure
    BOOST_FOREACH(const SymHeap &current, heaps_) {
        // TODO: check for entailment instead
        if (heap == current)
            return;
    }

    // add given heap to union
    heaps_.push_back(heap);
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct SymExec::Private {
    CodeStorage::Storage        &stor;
    const CodeStorage::Fnc      *fnc;
    const CodeStorage::Block    *bb;
    const CodeStorage::Insn     *insn;
    LocationWriter              lw;
    SymHeapUnion                stateZero;
    TStateMap                   state;
    TBlockSet                   todo;
    SymHeapUnion                *results;

    Private(CodeStorage::Storage &stor_):
        stor(stor_),
        fnc(0),
        bb(0),
        insn(0)
    {
    }

    void execReturn(SymbolicHeap::SymHeap heap);
    void updateState(const CodeStorage::Block *ofBlock,
                     const SymbolicHeap::SymHeap &heap);
    void execCondInsn(const SymbolicHeap::SymHeap &heap);
    void execTermInsn(const SymbolicHeap::SymHeap &heap);
    CodeStorage::Fnc* resolveCallee(const SymbolicHeap::SymHeap &heap,
                                    const struct cl_operand &op);
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

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using CodeStorage::Var;
    using SymbolicHeap::SymHeap;

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

    // TODO: process results somehow (generate points-to graph, etc.)
}

void SymExec::Private::execReturn(SymbolicHeap::SymHeap heap)
{
    using namespace SymbolicHeap;

    const CodeStorage::TOperandList &opList = insn->operands;
    if (1 != opList.size())
        TRAP;

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymHeapProcessor proc(heap);
        const int val = proc.heapValFromOperand(src);
        if (VAL_INVALID == val)
            TRAP;

        heap.setReturnValue(val);
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

    // check if anything has changed
    if (huni.size() != last)

        // schedule for next wheel
        todo.insert(ofBlock);
}
        
void SymExec::Private::execCondInsn(const SymbolicHeap::SymHeap &heap)
{
    using namespace SymbolicHeap;

    const CodeStorage::TOperandList &oplist = insn->operands;
    const CodeStorage::TTargetList &tlist = insn->targets;
    if (2 != tlist.size() || 1 != oplist.size())
        TRAP;

    SymHeapProcessor proc(const_cast<SymHeap &>(heap));
    const int val = proc.heapValFromOperand(oplist[0]);
    switch (val) {
        case VAL_TRUE:
            this->updateState(tlist[0], heap);
            break;

        case VAL_FALSE:
            this->updateState(tlist[1], heap);
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

CodeStorage::Fnc*
SymExec::Private::resolveCallee(const SymbolicHeap::SymHeap &heap,
                                const struct cl_operand &op)
{
    using namespace SymbolicHeap;
    int uid;

    if (CL_OPERAND_CST == op.code) {
        // direct call
        const struct cl_cst &cst = op.data.cst;
        if (CL_TYPE_FNC != cst.code)
            TRAP;

        uid = cst.data.cst_fnc.uid;

    } else {

        // indirect call
        SymHeapProcessor proc(const_cast<SymHeap &>(heap));
        const int val = proc.heapValFromOperand(op);
        if (VAL_INVALID == val)
            // Oops, it does not really look as indirect call
            TRAP;

        uid = heap.valGetCustom(/* TODO: check type */ 0, val);
    }

    return this->stor.anyFncById[uid];
}

void SymExec::Private::execCallInsn(SymbolicHeap::SymHeap heap,
                                    SymHeapUnion &results)
{
    using namespace CodeStorage;
    using namespace SymbolicHeap;

    const TOperandList &opList = insn->operands;
    if (CL_INSN_CALL != insn->code || opList.size() < 2)
        TRAP;

    const Fnc *fnc = this->resolveCallee(heap, opList[/* fnc */ 1]);
    if (!fnc)
        // unable to resolve Fnc by UID
        TRAP;

    // crate local variables of called fnc
    createStackFrame(heap, *fnc);
    setCallArgs(heap, *fnc, opList);

    // FIXME: this approach will sooner or later cause a stack overflow
    // TODO: use an explicit stack instead
    Private subExec(this->stor);
    subExec.fnc = fnc;
    subExec.results = &results;

    // TODO: here detect recursion somehow!
    subExec.execFnc(heap);

    const struct cl_operand &opRet = opList[0];
    if (CL_OPERAND_VOID != opRet.code) {
        // go through results and perform assignment of the return value
        BOOST_FOREACH(SymHeap &res, results) {
            assignReturnValue(res, opRet);
        }
    }

    // FIXME: we are going to free non-heap objects, and SymHeap tends to think
    // it's a bug in the analyzed program :-)
    destroyStackFrame(results, *fnc);
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
        CL_MSG_STREAM(cl_debug, this->lw << "debug: *** processing heap #"
                << (++hCnt) << " of BB " << bb->name() << "...");

        if (isTerm) {
            // terminal insn
            this->execTermInsn(heap);

        } else {
            // working area for non-term instructions
            SymHeap workingHeap(heap);
            SymHeapProcessor proc(workingHeap);

            // NOTE: this has to be tried *before* execCallInsn() to eventually
            // catch malloc()/free() calls, which are treated differently
            if (proc.exec(*insn))
                // non-terminal insn
                nextLocalState.insert(workingHeap);

            else
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
    CL_MSG_STREAM(cl_debug, lw << "debug: >>> entering " << name << "...");

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
        CL_MSG_STREAM(cl_debug, this->lw << "debug: --- executing insn #"
                << (++iCnt) << " of BB " << name << "...");

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
    while (!todo.empty()) {
        // FIXME: take BBs in some reasonable order instead
        TBlockSet::iterator i = this->todo.begin();
        this->bb = *i;
        this->todo.erase(i);

        this->execBb();
    }

    CL_DEBUG("execFncBody(): main loop terminated correctly...");
}

void SymExec::Private::execFnc(const SymbolicHeap::SymHeap &init)
{
    using namespace CodeStorage;
    using SymbolicHeap::SymHeap;

    const std::string &fncName = nameOf(*this->fnc);
    this->lw = &fnc->def.loc;
    CL_MSG_STREAM(cl_debug, this->lw << "debug: >>> entering "
            << fncName << "()...");

    CL_DEBUG("looking for entry block...");
    const ControlFlow &cfg = fnc->cfg;
    this->bb = cfg.entry();
    if (!this->bb) {
        CL_MSG_STREAM(cl_error, this->lw << "error: "
                << fncName << ": "
                << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    SymHeapUnion &huni = this->state[this->bb];
    huni.insert(init);

    // now we are ready for symbolic execution
    this->execFncBody();
}
