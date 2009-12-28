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

#include "btprint.hh"
#include "cl_private.hh"
#include "symheap.hh"
#include "symstate.hh"

#include <set>
#include <stack>
#include <vector>

#include <boost/foreach.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymHeapProcessor implementation
void SymHeapProcessor::printBackTrace() {
    if (!btPrinter_)
        TRAP;

    btPrinter_->printBackTrace();
}

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

        case CL_TYPE_STRING: {
            // FIXME: this temporary workaround is highly suboptimal, subtle
            // and error-prone !!!
            const int uid = reinterpret_cast<long>(cst.data.cst_string.value);
            return heap_.valCreateCustom(op.type, uid);
        }

        default:
            TRAP;
            return SymbolicHeap::VAL_INVALID;
    }
}

void SymHeapProcessor::heapVarHandleAccessorDeref(int *pObj)
{
    using namespace SymbolicHeap;

    // attempt to dereference
    const int val = heap_.valueOf(*pObj);
    switch (val) {
        case VAL_NULL:
            CL_MSG_STREAM(cl_error, lw_ << "error: dereference of NULL value");
            this->printBackTrace();
            goto fail;

        case VAL_UNINITIALIZED:
            CL_MSG_STREAM(cl_error, lw_ << "error: dereference of uninitialized value");
            this->printBackTrace();
            goto fail;

        case VAL_UNKNOWN:
            *pObj = OBJ_UNKNOWN;
            return;

        case VAL_INVALID:
            TRAP;
            // go through!

        case VAL_DEREF_FAILED:
            goto fail;

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

fail:
    *pObj = OBJ_DEREF_FAILED;
}

void SymHeapProcessor::heapVarHandleAccessorItem(int *pObj,
                                                 const struct cl_accessor *ac)
{
    using namespace SymbolicHeap;

    // access subVar
    const int id = ac->data.item.id;
    *pObj = heap_.subVar(*pObj, id);

    // check result of the SymHeap operation
    if (OBJ_INVALID == *pObj)
        *pObj = /* FIXME: misleading */ OBJ_DEREF_FAILED;
}

void SymHeapProcessor::heapVarHandleAccessor(int *pObj,
                                             const struct cl_accessor *ac)
{
    const enum cl_accessor_e code = ac->code;
    switch (code) {
        case CL_ACCESSOR_DEREF:
            this->heapVarHandleAccessorDeref(pObj);
            return;

        case CL_ACCESSOR_ITEM:
            this->heapVarHandleAccessorItem(pObj, ac);
            return;

        case CL_ACCESSOR_REF:
        case CL_ACCESSOR_DEREF_ARRAY:
            TRAP;
    }
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
            // fall through!
        case OBJ_DEREF_FAILED:
            return false;

        case OBJ_LOST:
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

namespace {
    template <class TStack, class TSet, class THeap>
    void digPointingObjects(TStack &todo, TSet &done, THeap &heap, int val) {
        using namespace SymbolicHeap;

        // go through all objects having the value
        SymHeap::TCont cont;
        heap.haveValue(cont, val);
        BOOST_FOREACH(int obj, cont) {

            // go through all super objects
            while (0 < obj) {
                if (!hasKey(done, obj))
                    todo.push(obj);

                obj = heap.varParent(obj);
            }
        }
    }

    template <class THeap>
    bool killJunk(THeap &heap, int ptrVal) {
        const int obj = heap.pointsTo(ptrVal);
        if (obj < 0)
            // invalid object simply can't be JUNK
            return false;

        if (-1 != heap.cVar(obj))
            // non-heap object simply can't be JUNK
            return false;

        std::stack<int /* var */> todo;
        std::set<int /* var */> done;
        digPointingObjects(todo, done, heap, ptrVal);
        while (!todo.empty()) {
            const int obj = todo.top();
            todo.pop();
            if (SymbolicHeap::OBJ_RETURN == obj)
                return false;

            if (-1 != heap.cVar(obj))
                return false;

            const int val = heap.placedAt(obj);
            if (val <= 0)
                TRAP;

            digPointingObjects(todo, done, heap, val);
        }

        // TODO: now kill all the JUNK somehow :-)
        return true;
    }

    template <class TCont, class THeap>
    void getPtrValues(TCont &dst, THeap &heap, int obj) {
        std::stack<int /* obj */> todo;
        todo.push(obj);
        while (!todo.empty()) {
            const int obj = todo.top();
            todo.pop();

            const struct cl_type *clt = heap.objType(obj);
            const enum cl_type_e code = (clt)
                ? clt->code
                : /* anonymous object of known size */ CL_TYPE_PTR;

            switch (code) {
                case CL_TYPE_PTR: {
                    const int val = heap.valueOf(obj);
                    if (0 < val)
                        dst.push_back(val);

                    break;
                }

                case CL_TYPE_STRUCT:
                    for (int i = 0; i < clt->item_cnt; ++i) {
                        const int subVar = heap.subVar(obj, i);
                        if (subVar < 0)
                            TRAP;

                        todo.push(subVar);
                    }
                    break;

                case CL_TYPE_BOOL:
                    break;

                default:
                    // other type of values should be safe to ignore here
                    // but worth to check by a debugger at least once anyway
                    TRAP;
            }
        }
    }
}

void SymHeapProcessor::checkForJunk(int val) {
    using namespace SymbolicHeap;
    bool detected = false;

    std::stack<int /* val */> todo;
    todo.push(val);
    while (!todo.empty()) {
        const int val = todo.top();
        todo.pop();

        if (killJunk(heap_, val)) {
            detected = true;
            const int obj = heap_.pointsTo(val);
            if (obj <= 0)
                TRAP;

            // gather all values inside the junk object
            std::vector<int /* val */> ptrs;
            getPtrValues(ptrs, heap_, obj);

            // destroy junk
            CL_MSG_STREAM(cl_warn, lw_ << "warning: killing junk");
            heap_.objDestroy(obj);

            // schedule just created junk candidates for next wheel
            BOOST_FOREACH(int ptrVal, ptrs) {
                todo.push(ptrVal);
            }
        }
    }

    if (detected)
        // we would like to print the backtrace only once for a bunch of objects
        // but this feature seems to be rarely useful in practice :-(
        this->printBackTrace();
}

void SymHeapProcessor::heapSetVal(int /* obj */ lhs, int /* val */ rhs) {
    using namespace SymbolicHeap;

    // save the old value, which is going to be overwritten
    const int oldValue = heap_.valueOf(lhs);
    if (VAL_INVALID == oldValue)
        TRAP;

    if (heap_.valPointsToAnon(rhs)) {
        // anonymous object is going to be specified by a type
        const int var = heap_.pointsTo(rhs);
        if (OBJ_INVALID == var)
            TRAP;

        const struct cl_type *clt = heap_.objType(lhs);
        if (!clt)
            goto clt_done;

        if (clt->code != CL_TYPE_PTR)
            TRAP;

        // move to next clt
        // --> what are we pointing to actually?
        clt = clt->items[0].type;
        if (!clt)
            TRAP;

        if (CL_TYPE_VOID == clt->code)
            goto clt_done;

        const int cbGot = heap_.varSizeOfAnon(var);
        const int cbNeed = clt->size;
        if (cbGot != cbNeed) {
            const bool isErr = (cbGot < cbNeed);
            CL_MSG_STREAM(((isErr) ? cl_error : cl_warn), lw_
                    << ((isErr) ? "error" : "warning")
                    << ": amount of allocated memory not accurate");
            CL_MSG_STREAM(cl_note, lw_ << "note: allocated: "
                    << cbGot  << " bytes");
            CL_MSG_STREAM(cl_note, lw_ << "note:  expected: "
                    << cbNeed << " bytes");
        }

        heap_.varDefineType(var, clt);
    }

clt_done:
    heap_.objSetValue(lhs, rhs);
    this->checkForJunk(oldValue);
}

void SymHeapProcessor::destroyObj(int obj) {
    // gather destroyed values
    std::vector<int> ptrs;
    getPtrValues(ptrs, heap_, obj);

    // destroy object recursively
    heap_.objDestroy(obj);

    // now check for JUNK
    BOOST_FOREACH(int val, ptrs) {
        this->checkForJunk(val);
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
            // go through!

        case VAL_DEREF_FAILED:
            return;

        case VAL_INVALID:
        case VAL_UNINITIALIZED:
        case VAL_UNKNOWN:
            CL_MSG_STREAM(cl_error, lw_
                    << "error: invalid free() detected");
            this->printBackTrace();
            return;

        default:
            break;
    }

    const int obj = heap_.pointsTo(val);
    switch (obj) {
        case OBJ_DELETED:
            CL_MSG_STREAM(cl_error, lw_
                    << "error: double free() detected");
            this->printBackTrace();
            return;

        case OBJ_UNKNOWN:
        case OBJ_INVALID:
            TRAP;
        default:
            break;
    }

    CL_MSG_STREAM(cl_debug, lw_ << "debug: executing free()");
    // TODO: check for possible free() of non-heap object
    // TODO: check for possible free() of non-root
    this->destroyObj(obj);
}

void SymHeapProcessor::execMalloc(TState &state,
                                  const CodeStorage::TOperandList &opList)
{
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

    const int cbAmount = cst.data.cst_int.value;
    CL_MSG_STREAM(cl_debug, lw_ << "debug: executing malloc(" << cbAmount << ")");
    const int obj = heap_.varCreateAnon(cbAmount);
    if (OBJ_INVALID == obj)
        // unable to create dynamic variable
        TRAP;

    const int val = heap_.placedAt(obj);
    if (val <= 0)
        TRAP;

    // OOM state simulation
    // FIXME: this feature should be strictly optional, it may explode badly
    this->heapSetVal(varLhs, VAL_NULL);
    state.insert(heap_);

    // store the result of malloc
    this->heapSetVal(varLhs, val);
    state.insert(heap_);
}

bool SymHeapProcessor::execCall(TState &dst, const CodeStorage::Insn &insn) {
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
        this->execMalloc(dst, opList);
        return true;
    }

    if (STREQ(fncName, "free")) {
        this->execFree(opList);
        dst.insert(heap_);
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

    this->heapSetVal(varLhs, valRhs);
}

namespace {
    int handleSpecialValues(int val1, int val2) {
        using namespace SymbolicHeap;

        if (VAL_NULL <= val1 && VAL_NULL <= val2)
            return VAL_FALSE;

        const int lower = (val1 < val2) ? val1 : val2;
        switch (lower) {
            case VAL_INVALID:
            case VAL_UNINITIALIZED:
            case VAL_UNKNOWN:
            case VAL_DEREF_FAILED:
                return lower;

            default:
                TRAP;
                return VAL_INVALID;
        }
    }
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
    int val = handleSpecialValues(val1, val2);

    // execute CL_BINOP_EQ/CL_BINOP_NE
    bool result = (val1 == val2);
    if (neg)
        result = !result;

    // convert bool result to a heap value
    if (!val)
        val = (result)
            ? VAL_TRUE
            : VAL_FALSE;

    // store resulting value
    heap_.objSetValue(dst, val);
}

bool SymHeapProcessor::exec(TState &dst, const CodeStorage::Insn &insn) {
    using namespace CodeStorage;

    lw_ = &insn.loc;

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execUnary(insn);
            dst.insert(heap_);
            return true;

        case CL_INSN_BINOP:
            this->execBinary(insn);
            dst.insert(heap_);
            return true;

        case CL_INSN_CALL:
            return this->execCall(dst, insn);

        default:
            TRAP;
            return true;
    }
}
