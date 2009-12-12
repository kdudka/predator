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

#include <iostream>

#include <boost/foreach.hpp>

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

typedef std::pair<int /* Fnc uid */, int /* BB uid */> TStateId;

// FIXME: the following setup is not ready to deal with recursion
typedef std::map<TStateId, SymHeapUnion> TStateMap;

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct SymExec::Private {
    CodeStorage::Storage        &stor;
    const CodeStorage::Fnc      *fnc;
    const CodeStorage::Block    *bb;
    LocationWriter              lw;
    SymbolicHeap::SymHeap       heap;

    Private(CodeStorage::Storage &stor_):
        stor(stor_)
    {
    }

    void execUnary(const CodeStorage::Insn &insn);
    int /* val */ heapValFromCst(const struct cl_operand &op);
    int /* var */ heapVarFromOperand(const struct cl_operand &op);
    int /* val */ heapValFromOperand(const struct cl_operand &op);
};

SymExec::SymExec(CodeStorage::Storage &stor):
    d(new Private(stor))
{
}

SymExec::~SymExec() {
    delete d;
}

int /* val */ SymExec::Private::heapValFromCst(const struct cl_operand &op) {
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

int /* var */ SymExec::Private::heapVarFromOperand(const struct cl_operand &op)
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

    return heap.varByCVar(uid);
}

int /* val */ SymExec::Private::heapValFromOperand(const struct cl_operand &op)
{
    using namespace SymbolicHeap;

    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
        case CL_OPERAND_REG: {
            int var = heapVarFromOperand(op);
            if (OBJ_INVALID == var)
                TRAP;

            return heap.valueOf(var);
        }

        case CL_OPERAND_CST:
            return heapValFromCst(op);

        default:
            TRAP;
            return OBJ_INVALID;
    }
}

void SymExec::Private::execUnary(const CodeStorage::Insn &insn) {
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

    heap.objSetValue(varLhs, valRhs);
}

void SymExec::exec(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;
    d->lw = &insn.loc;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: executing insn...");
    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            d->execUnary(insn);
            break;

        default:
            TRAP;
    }
}

void SymExec::exec(const CodeStorage::Block &bb) {
    using namespace CodeStorage;
    d->bb = &bb;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: entering " << bb.name() << "...");
    BOOST_FOREACH(const Insn *insn, bb) {
        this->exec(*insn);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using namespace CodeStorage;
    d->fnc = &fnc;
    d->lw = &fnc.def.loc;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: entering "
            << nameOf(fnc) << "()...");

    using namespace CodeStorage;
    BOOST_FOREACH(const Var &var, fnc.vars) {
        CL_DEBUG("--- creating stack variable: #" << var.uid
                << " (" << var.name << ")" );
        d->heap.varCreate(var.clt, var.uid);
    }

    CL_DEBUG("looking for entry block...");
    const ControlFlow &cfg = fnc.cfg;
    const Block *entry = cfg.entry();
    if (!entry) {
        CL_MSG_STREAM(cl_error, d->lw << "error: "
                << nameOf(fnc) << ": "
                << "entry block not found");
        return;
    }
    this->exec(*entry);
}
