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

#include <map>
#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

class SymHeapProcessor {
    public:
        SymHeapProcessor(SymbolicHeap::SymHeap &heap):
            heap_(heap)
        {
        }

        bool exec(const CodeStorage::Insn &insn);

    public:
        int /* val */ heapValFromCst(const struct cl_operand &op);
        int /* var */ heapVarFromOperand(const struct cl_operand &op);
        int /* val */ heapValFromOperand(const struct cl_operand &op);

    private:
        void heapVarHandleAccessor(int *pVar, const struct cl_accessor *ac);
        bool lhsFromOperand(int *pVar, const struct cl_operand &op);
        void execUnary(const CodeStorage::Insn &insn);
        void execBinary(const CodeStorage::Insn &insn);
        void execMalloc(const CodeStorage::TOperandList &opList);
        void execFree(const CodeStorage::TOperandList &opList);
        bool execCall(const CodeStorage::Insn &insn);

    private:
        SymbolicHeap::SymHeap       &heap_;
        LocationWriter              lw_;
};

class SymHeapUnion {
    private:
        typedef std::vector<SymbolicHeap::SymHeap> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

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

    private:
        TList heaps_;
};

typedef std::set<const CodeStorage::Block *>                TBlockSet;
typedef std::map<const CodeStorage::Block *, SymHeapUnion>  TStateMap;

// /////////////////////////////////////////////////////////////////////////////
// SymHeapProcessor implementation
int /* val */ SymHeapProcessor::heapValFromCst(const struct cl_operand &op) {
    enum cl_type_e code = op.type->code;
    switch (code) {
        case CL_TYPE_INT:
            CL_DEBUG("CL_TYPE_INT treated as pointer");
            // go through!

        case CL_TYPE_PTR:
            break;

        default:
            TRAP;
    }

    const struct cl_cst &cst = op.data.cst;
    code = cst.code;
    switch (code) {
        case CL_TYPE_INT:
            if (0 == cst.data.cst_int.value)
                return SymbolicHeap::VAL_NULL;
            // go through!

        default:
            TRAP;
            return SymbolicHeap::VAL_INVALID;
    }
}

void SymHeapProcessor::heapVarHandleAccessor(int *pObj,
                                             const struct cl_accessor *ac)
{
    using namespace SymbolicHeap;
    if (CL_ACCESSOR_DEREF != ac->code)
        // not implemented yet
        TRAP;

    // attempt to dereference
    const int val = heap_.valueOf(*pObj);
    switch (val) {
        case VAL_NULL:
            CL_MSG_STREAM(cl_error, lw_ << "error: dereference of NULL value");
            goto unknown_obj;

        case VAL_UNINITIALIZED:
            CL_MSG_STREAM(cl_error, lw_ << "error: dereference of uninitialized value");
            goto unknown_obj;

        // TODO
        case VAL_UNKNOWN:
        case VAL_INVALID:
            TRAP;

        default:
            break;
    }

    // value lookup
    *pObj = heap_.pointsTo(val);
    switch (*pObj) {
        // TODO
        case OBJ_DELETED:
        case OBJ_INVALID:
            TRAP;

        default:
            return;
    }

unknown_obj:
    *pObj = OBJ_UNKNOWN;
}

int /* var */ SymHeapProcessor::heapVarFromOperand(const struct cl_operand &op)
{
    using SymbolicHeap::OBJ_INVALID;
    int uid;

    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
            uid = op.data.var.id;
            break;

        case CL_OPERAND_REG:
            uid = op.data.reg.id;
            break;

        default:
            TRAP;
            return OBJ_INVALID;
    }

    int var = heap_.varByCVar(uid);
    if (OBJ_INVALID == var)
        // unable to resolve static variable
        TRAP;

    // go through the list of accessors
    const struct cl_accessor *ac = op.accessor;
    if (ac && ac->code == CL_ACCESSOR_REF)
        // CL_ACCESSOR_REF will be processed as soon as this function finishes
        // ... otherwise we are encountering a bug!
        ac = ac->next;

    // process all other accessors (only CL_ACCESSOR_DEREF for now)
    while (ac) {
        this->heapVarHandleAccessor(&var, ac);
        ac = ac->next;
    }

    return var;
}

bool /* var */ SymHeapProcessor::lhsFromOperand(int *pVar,
                                                const struct cl_operand &op)
{
    using namespace SymbolicHeap;

    *pVar = heapVarFromOperand(op);
    switch (*pVar) {
        case OBJ_UNKNOWN:
            CL_MSG_STREAM(cl_debug, lw_ << "debug: "
                    "ignoring OBJ_UNKNOWN as lhs, this is definitely a bug "
                    "if there is no error reported above...");
            return false;

        case OBJ_DELETED:
        case OBJ_INVALID:
            TRAP;

        default:
            return true;
    }
}

int /* val */ SymHeapProcessor::heapValFromOperand(const struct cl_operand &op)
{
    using namespace SymbolicHeap;

    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
        case CL_OPERAND_REG: {
            int var = heapVarFromOperand(op);
            if (OBJ_INVALID == var)
                TRAP;

            const struct cl_accessor *ac = op.accessor;
            return (ac && ac->code == CL_ACCESSOR_REF)
                ? heap_.placedAt(var)
                : heap_.valueOf(var);
        }

        case CL_OPERAND_CST:
            return heapValFromCst(op);

        default:
            TRAP;
            return OBJ_INVALID;
    }
}

void SymHeapProcessor::execFree(const CodeStorage::TOperandList &opList) {
    using namespace SymbolicHeap;
    if (/* dst + fnc + ptr */ 3 != opList.size())
        TRAP;

    if (CL_OPERAND_VOID != opList[0].code)
        // Oops, free() does not usually return a value
        TRAP;

    const int val = heapValFromOperand(opList[/* ptr given to free() */ 2]);
    if (VAL_INVALID == val)
        // could not resolve value to be freed
        TRAP;

    switch (val) {
        case VAL_NULL:
            CL_MSG_STREAM(cl_debug, lw_
                    << "debug: ignoring free() called with NULL value");
            return;

        case VAL_INVALID:
        case VAL_UNINITIALIZED:
        case VAL_UNKNOWN:
            CL_MSG_STREAM(cl_error, lw_
                    << "error: invalid free() detected");
            return;

        default:
            break;
    }

    const int obj = heap_.pointsTo(val);
    switch (obj) {
        case OBJ_DELETED:
            CL_MSG_STREAM(cl_error, lw_
                    << "error: double free() detected");
            return;

        case OBJ_UNKNOWN:
        case OBJ_INVALID:
            TRAP;
        default:
            break;
    }

    CL_MSG_STREAM(cl_debug, lw_ << "debug: executing free()");
    heap_.objDestroy(obj);
}

void SymHeapProcessor::execMalloc(const CodeStorage::TOperandList &opList) {
    using namespace SymbolicHeap;
    if (/* dst + fnc + size */ 3 != opList.size())
        TRAP;

    const struct cl_operand &dst = opList[0];
    int varLhs = heapVarFromOperand(dst);
    if (OBJ_INVALID == varLhs)
        // could not resolve lhs
        TRAP;

    const struct cl_operand &amount = opList[2];
    if (CL_OPERAND_CST != amount.code)
        // amount of allocated memory not constant
        TRAP;

    const struct cl_cst &cst = amount.data.cst;
    if (CL_TYPE_INT != cst.code)
        // amount of allocated memory not a number
        TRAP;

    // FIXME: we simply ignore the ammount of allocated memory
    const int cbAmount = cst.data.cst_int.value;
    CL_MSG_STREAM(cl_debug, lw_ << "debug: executing malloc(" << cbAmount << ")");

    // FIXME: we can't use dst.type as type of the created obj in most cases :-(
    const int obj = heap_.varCreate(dst.type, /* heap obj */ -1);
    if (OBJ_INVALID == obj)
        // unable to create dynamic variable
        TRAP;

    // TODO: delayed var creation?
    const int val = heap_.placedAt(obj);
    switch (val) {
        case VAL_NULL:
        case VAL_INVALID:
        case VAL_UNINITIALIZED:
        case VAL_UNKNOWN:
            TRAP;

        default:
            break;
    }

    // store the result of malloc
    // TODO: check for possible JUNK here!
    heap_.objSetValue(varLhs, val);
}

bool SymHeapProcessor::execCall(const CodeStorage::Insn &insn) {
    using namespace SymbolicHeap;

    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_operand &fnc = opList[1];
    if (CL_OPERAND_CST != fnc.code)
        return false;

    const struct cl_cst &cst = fnc.data.cst;
    if (CL_TYPE_FNC != cst.code)
        return false;

    if (CL_SCOPE_GLOBAL != fnc.scope || !cst.data.cst_fnc.is_extern)
        return false;

    const char *fncName = cst.data.cst_fnc.name;
    if (!fncName)
        return false;

    if (STREQ(fncName, "malloc")) {
        this->execMalloc(opList);
        return true;
    }

    if (STREQ(fncName, "free")) {
        this->execFree(opList);
        return true;
    }

    return false;
}

void SymHeapProcessor::execUnary(const CodeStorage::Insn &insn) {
    using namespace SymbolicHeap;

    const enum cl_unop_e code = static_cast<enum cl_unop_e> (insn.subCode);
    if (CL_UNOP_ASSIGN != code)
        // not implemented yet
        TRAP;

    int varLhs;
    if (!lhsFromOperand(&varLhs, insn.operands[0]))
        return;

    int valRhs = heapValFromOperand(insn.operands[1]);
    if (VAL_INVALID == valRhs)
        // could not resolve rhs
        TRAP;

    // TODO: check for possible JUNK here!
    heap_.objSetValue(varLhs, valRhs);
}

void SymHeapProcessor::execBinary(const CodeStorage::Insn &insn) {
    using namespace SymbolicHeap;

    const enum cl_binop_e code = static_cast<enum cl_binop_e> (insn.subCode);
    bool neg = false;
    switch (code) {
        case CL_BINOP_NE:
            neg = true;
            // fall through!
        case CL_BINOP_EQ:
            break;

        default:
            TRAP;
    }

    // resolve dst
    int dst;
    if (!lhsFromOperand(&dst, insn.operands[0]))
        return;

    // resolve src1, src2
    const int val1 = heapValFromOperand(insn.operands[1]);
    const int val2 = heapValFromOperand(insn.operands[2]);
    if (val1 < 0 || val2 < 0)
        // TODO: handle special values here
        TRAP;

    // execute CL_BINOP_EQ/CL_BINOP_NE
    bool result = (val1 == val2);
    if (neg)
        result = !result;

    // convert bool result to a heap value
    const int val = (result)
        ? VAL_TRUE
        : VAL_FALSE;

    // store resulting value
    heap_.objSetValue(dst, val);
}

bool SymHeapProcessor::exec(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;

    lw_ = &insn.loc;

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execUnary(insn);
            return true;

        case CL_INSN_BINOP:
            this->execBinary(insn);
            return true;

        case CL_INSN_CALL:
            return this->execCall(insn);

        default:
            TRAP;
            return true;
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymHeapUnion implementation
namespace {
    bool checkNonPosValues(int a, int b) {
        if (0 < a && 0 < b)
            // we'll need to properly compare positive values
            return false;

        // non-positive value always have to match, bail out otherwise
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

    Private(CodeStorage::Storage &stor_):
        stor(stor_),
        fnc(0),
        bb(0),
        insn(0)
    {
    }

    void updateState(const CodeStorage::Block *ofBlock,
                     const SymbolicHeap::SymHeap &heap);
    void execCondInsn(const SymbolicHeap::SymHeap &heap);
    void execTermInsn(const SymbolicHeap::SymHeap &heap);
    void execCallInsn(const SymbolicHeap::SymHeap &heap, SymHeapUnion &result);
    void execInsn(SymHeapUnion &localState);
    void execBb();
    void execFncBody();
    void execFnc(const SymbolicHeap::SymHeap &init, SymHeapUnion &result);
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

    BOOST_FOREACH(const SymHeap &zero, d->stateZero) {
        SymHeap init(zero);

        // prepare local variables
        // TODO: move to function and share with CL_INSN_CALL exec code
        BOOST_FOREACH(const Var &var, fnc.vars) {
            CL_DEBUG("--- creating stack variable: #" << var.uid
                    << " (" << var.name << ")" );

            init.varCreate(var.clt, var.uid);
        }

        // well, we have successfully prepared the initial state,
        // now please execute the function!
        d->execFnc(init, results);
    }

    // TODO: process results somehow (generate points-to graph, etc.)
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
            CL_MSG_STREAM(cl_warn, this->lw
                    << "warning: return statement ignored [not implemented]");
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

void SymExec::Private::execCallInsn(const SymbolicHeap::SymHeap &heap,
                                    SymHeapUnion &result)
{
    if (CL_INSN_CALL != insn->code)
        TRAP;

    const CodeStorage::TOperandList &opList = insn->operands;
    const struct cl_operand &fnc = opList[1];
    if (CL_OPERAND_CST != fnc.code)
        TRAP;

    const struct cl_cst &cst = fnc.data.cst;
    if (CL_TYPE_FNC != cst.code)
        TRAP;

    // create args
    // set args values
    // create local variables
    // do call
    // destroy args
    // destroy local variables
    TRAP;
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
    // NOTE: it may grow badly on any CL_INSN_CALL instruction
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

void SymExec::Private::execFnc(const SymbolicHeap::SymHeap &init,
                               SymHeapUnion &results)
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
