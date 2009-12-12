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
#include "location.hh"
#include "storage.hh"
#include "symheap.hh"

#include <map>
#include <set>

#include <boost/foreach.hpp>

class SymHeapProcessor {
    public:
        SymHeapProcessor(SymbolicHeap::SymHeap &heap):
            heap_(heap)
        {
        }

        void exec(const CodeStorage::Insn &insn);

    private:
        void execUnary(const CodeStorage::Insn &insn);
        int /* val */ heapValFromCst(const struct cl_operand &op);
        int /* var */ heapVarFromOperand(const struct cl_operand &op);
        int /* val */ heapValFromOperand(const struct cl_operand &op);

    private:
        SymbolicHeap::SymHeap &heap_;
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
    if (CL_TYPE_PTR != op.type->code)
        // not implemented yet
        TRAP;

    const struct cl_cst &cst = op.data.cst;
    const enum cl_type_e code = cst.code;
    switch (code) {
        case CL_TYPE_INT:
            if (0 == cst.data.cst_int.value)
                return SymbolicHeap::VAL_NULL;

        default:
            TRAP;
            return SymbolicHeap::VAL_INVALID;
    }
}

int /* var */ SymHeapProcessor::heapVarFromOperand(const struct cl_operand &op)
{
    const enum cl_operand_e code = op.code;
    int uid;
    switch (code) {
        case CL_OPERAND_VAR:
            uid = op.data.var.id;
            break;

        case CL_OPERAND_REG:
            uid = op.data.reg.id;
            break;

        default:
            TRAP;
            return SymbolicHeap::OBJ_INVALID;
    }

    const cl_accessor *ac = op.accessor;
    if (ac)
        // not implemented yet
        TRAP;

    return heap_.varByCVar(uid);
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

            return heap_.valueOf(var);
        }

        case CL_OPERAND_CST:
            return heapValFromCst(op);

        default:
            TRAP;
            return OBJ_INVALID;
    }
}

void SymHeapProcessor::execUnary(const CodeStorage::Insn &insn) {
    using namespace SymbolicHeap;

    enum cl_unop_e code = static_cast<enum cl_unop_e> (insn.subCode);
    if (CL_UNOP_ASSIGN != code)
        // not implemented yet
        TRAP;

    int varLhs = heapVarFromOperand(insn.operands[0]);
    if (OBJ_INVALID == varLhs)
        // could not resolve lhs
        TRAP;

    int valRhs = heapValFromOperand(insn.operands[1]);
    if (VAL_INVALID == valRhs)
        // could not resolve rhs
        TRAP;

    heap_.objSetValue(varLhs, valRhs);
}

void SymHeapProcessor::exec(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;

    LocationWriter lw(&insn.loc);
    CL_MSG_STREAM(cl_debug, lw << "debug: executing insn...");

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execUnary(insn);
            break;

        default:
            TRAP;
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymHeapUnion implementation
void SymHeapUnion::insert(const SymbolicHeap::SymHeap &heap) {
    // TODO: check for identity and/or entailment
    heaps_.push_back(heap);
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct SymExec::Private {
    CodeStorage::Storage        &stor;
    const CodeStorage::Fnc      *fnc;
    const CodeStorage::Block    *bb;
    LocationWriter              lw;
    TStateMap                   state;
    TBlockSet                   todo;

    Private(CodeStorage::Storage &stor_):
        stor(stor_)
    {
    }

    void execTermInsn(const CodeStorage::Insn &insn,
                      const SymbolicHeap::SymHeap &heap);
    void execBb();
    void execFncBody();
    void execFnc();
};

SymExec::SymExec(CodeStorage::Storage &stor):
    d(new Private(stor))
{
}

SymExec::~SymExec() {
    delete d;
}

void SymExec::Private::execTermInsn(const CodeStorage::Insn &insn,
                                    const SymbolicHeap::SymHeap &heap)
{
    // TODO
    TRAP;
}

void SymExec::Private::execBb() {
    using namespace CodeStorage;
    using SymbolicHeap::SymHeap;

    // some debugging stuff
    int bbCnt = 0;
    const std::string &name = bb->name();
    CL_MSG_STREAM(cl_debug, lw << "debug: >>> entering " << name << "...");

    // go through all symbolic heaps corresponding to entry of this BB
    SymHeapUnion &huni = this->state[this->bb];
    BOOST_FOREACH(const SymHeap &heap, huni) {
        CL_MSG_STREAM(cl_debug, this->lw << "debug: *** processing heap #"
                << (++bbCnt) << " of BB " << name << "...");

        // TODO: cow semantic
        SymHeap workingHeap(heap);
        SymHeapProcessor proc(workingHeap);

        // go through all BB insns
        int insnCnt = 0;
        BOOST_FOREACH(const Insn *insn, *bb) {
            // update location info
            this->lw = &insn->loc;

            if (cl_is_term_insn(insn->code))
                // terminal insn
                this->execTermInsn(*insn, workingHeap);
            else
                // non-terminal insn
                proc.exec(*insn);
        }
    }
}

void SymExec::Private::execFncBody() {
    if (!todo.empty())
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

void SymExec::Private::execFnc() {
    using namespace CodeStorage;
    using SymbolicHeap::SymHeap;

    // create initial state for called function
    // TODO: handle fnc args somehow
    SymHeap init;
    BOOST_FOREACH(const Var &var, fnc->vars) {
        CL_DEBUG("--- creating stack variable: #" << var.uid
                << " (" << var.name << ")" );
        init.varCreate(var.clt, var.uid);
    }

    // insert initial state to the corresponding union
    SymHeapUnion &huni = this->state[this->bb];
    huni.insert(init);

    this->execFncBody();
}

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using namespace CodeStorage;
    using SymbolicHeap::SymHeap;

    d->fnc = &fnc;
    d->lw = &fnc.def.loc;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: >>> entering "
            << nameOf(fnc) << "()...");

    CL_DEBUG("looking for entry block...");
    const ControlFlow &cfg = fnc.cfg;
    d->bb = cfg.entry();
    if (d->bb) {
        // we are indeed ready to execute the function
        d->execFnc();
        return;
    }

    CL_MSG_STREAM(cl_error, d->lw << "error: "
            << nameOf(fnc) << ": "
            << "entry block not found");
}
