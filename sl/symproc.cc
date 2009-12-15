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

#include "symproc.hh"

#include "cl_private.hh"
#include "symheap.hh"

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

        case CL_TYPE_FNC: {
            // wrap fnc uid as SymHeap value
            const int uid = cst.data.cst_fnc.uid;
            return heap_.valCreateCustom(op.type, uid);
        }

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
        // CL_ACCESSOR_REF will be processed right after this function finishes
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
