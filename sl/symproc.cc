/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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
#include "symproc.hh"

#include <cl/cl_msg.hh>

#include "btprint.hh"
#include "symheap.hh"
#include "symplot.hh"
#include "symstate.hh"
#include "util.hh"
#include "worklist.hh"

#include <stack>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymHeapProcessor implementation
void SymHeapProcessor::printBackTrace() {
    if (!btPrinter_)
        return;

    btPrinter_->printBackTrace();
}

TValueId SymHeapProcessor::heapValFromCst(const struct cl_operand &op) {
    bool isBool = false;
    enum cl_type_e code = op.type->code;
    switch (code) {
        case CL_TYPE_BOOL:
            isBool = true;
            break;

        case CL_TYPE_INT:
        case CL_TYPE_PTR:
            break;

        default:
            TRAP;
    }

    const struct cl_cst &cst = op.data.cst;
    code = cst.code;
    switch (code) {
        case CL_TYPE_INT:
            if (isBool) {
                return (cst.data.cst_int.value)
                    ? VAL_TRUE
                    : VAL_FALSE;
            } else {
                if (!cst.data.cst_int.value)
                    return VAL_NULL;

                // create a new unknown non-NULL value
                TValueId val = heap_.valCreateUnknown(UV_UNKNOWN, op.type);
                heap_.addNeq(val, VAL_NULL);
                return val;
            }

        case CL_TYPE_FNC: {
            // wrap fnc uid as SymHeap value
            const int uid = cst.data.cst_fnc.uid;
            return heap_.valCreateCustom(op.type, uid);
        }

        case CL_TYPE_STRING: {
            // FIXME: this temporary workaround is highly suboptimal, subtle
            // and error-prone !!!
            CL_WARN_MSG(lw_,
                    "CL_TYPE_STRING not supported by heapValFromCst()");
            const int uid = reinterpret_cast<long>(cst.data.cst_string.value);
            return heap_.valCreateCustom(op.type, uid);
        }

        default:
            TRAP;
            return VAL_INVALID;
    }
}

void SymHeapProcessor::heapObjHandleAccessorDeref(TObjId *pObj)
{
    EUnknownValue code;

    // attempt to dereference
    const TValueId val = heap_.valueOf(*pObj);
    switch (val) {
        case VAL_NULL:
            CL_ERROR_MSG(lw_, "dereference of NULL value");
            goto fail_with_bt;

        case VAL_INVALID:
            TRAP;
            goto fail;

        default:
            break;
    }

    // do we really know the value?
    code = heap_.valGetUnknown(val);
    switch (code) {
        case UV_KNOWN:
            break;

        case UV_UNKNOWN:
            *pObj = OBJ_UNKNOWN;
            return;

        case UV_UNINITIALIZED:
            CL_ERROR_MSG(lw_, "dereference of uninitialized value");
            goto fail_with_bt;

        case UV_DEREF_FAILED:
            goto fail;
    }

    // value lookup
    *pObj = heap_.pointsTo(val);
    switch (*pObj) {
        case OBJ_LOST:
            CL_ERROR_MSG(lw_, "dereference of non-existing non-heap object");
            goto fail_with_bt;

        case OBJ_DELETED:
            CL_ERROR_MSG(lw_, "dereference of already deleted heap object");
            goto fail_with_bt;

        case OBJ_UNKNOWN:
        case OBJ_INVALID:
            TRAP;

        default:
            return;
    }

fail_with_bt:
    this->printBackTrace();

fail:
    *pObj = OBJ_DEREF_FAILED;
}

void SymHeapProcessor::heapObjHandleAccessorItem(TObjId *pObj,
                                                 const struct cl_accessor *ac)
{
    // access subObj
    const int id = ac->data.item.id;
    *pObj = heap_.subObj(*pObj, id);

    // check result of the SymHeap operation
    if (OBJ_INVALID == *pObj)
        *pObj = /* FIXME: misleading */ OBJ_DEREF_FAILED;
}

void SymHeapProcessor::heapObjHandleAccessor(TObjId *pObj,
                                             const struct cl_accessor *ac)
{
    const enum cl_accessor_e code = ac->code;
    switch (code) {
        case CL_ACCESSOR_DEREF:
            this->heapObjHandleAccessorDeref(pObj);
            return;

        case CL_ACCESSOR_ITEM:
            this->heapObjHandleAccessorItem(pObj, ac);
            return;

        case CL_ACCESSOR_REF:
            // CL_ACCESSOR_REF will be processed wihtin heapValFromOperand()
            // on the way out from here ... otherwise we are encountering
            // a bug!
            return;

        case CL_ACCESSOR_DEREF_ARRAY:
            CL_WARN_MSG(lw_, "CL_ACCESSOR_DEREF_ARRAY not implemented yet");
            *pObj = OBJ_DEREF_FAILED;
            return;
    }
}

TObjId SymHeapProcessor::heapObjFromOperand(const struct cl_operand &op)
{
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

    TObjId var = heap_.objByCVar(uid);
    if (OBJ_INVALID == var)
        // unable to resolve static variable
        TRAP;

    // process all accessors
    const struct cl_accessor *ac = op.accessor;
    while (ac) {
        this->heapObjHandleAccessor(&var, ac);
        ac = ac->next;
    }

    return var;
}

bool SymHeapProcessor::lhsFromOperand(TObjId *pObj, const struct cl_operand &op)
{
    *pObj = this->heapObjFromOperand(op);
    switch (*pObj) {
        case OBJ_UNKNOWN:
            CL_DEBUG_MSG(lw_,
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
            if(this->heap_.objIsAbstract(*pObj)) 
                return false;
            return true;
    }
}

namespace {
    template <class THeap>
    TValueId valueFromVar(THeap &heap, TObjId var, const struct cl_type *clt,
                          const struct cl_accessor *ac)
    {
        switch (var) {
            case OBJ_INVALID:
                TRAP;
                return VAL_INVALID;

            case OBJ_UNKNOWN:
                return heap.valCreateUnknown(UV_UNKNOWN, clt);

            case OBJ_DELETED:
            case OBJ_DEREF_FAILED:
            case OBJ_LOST:
                return heap.valCreateUnknown(UV_DEREF_FAILED, clt);

            case OBJ_RETURN:
            default:
                if(heap.objIsAbstract(var)) // should be concretized
                    TRAP;
                break;
        }

        // seek the last accessor
        while (ac && ac->next)
            ac = ac->next;

        // handle CL_ACCESSOR_REF if any
        return (ac && ac->code == CL_ACCESSOR_REF)
            ? heap.placedAt(var)
            : heap.valueOf(var);
    }
}

TValueId SymHeapProcessor::heapValFromOperand(const struct cl_operand &op)
{
    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
        case CL_OPERAND_REG:
            return valueFromVar(heap_,
                    this->heapObjFromOperand(op),
                    op.type, op.accessor);

        case CL_OPERAND_CST:
            return this->heapValFromCst(op);

        default:
            TRAP;
            return VAL_INVALID;
    }
}

int /* uid */ SymHeapProcessor::fncFromOperand(const struct cl_operand &op) {
    if (CL_OPERAND_CST == op.code) {
        // direct call
        const struct cl_cst &cst = op.data.cst;
        if (CL_TYPE_FNC != cst.code)
            TRAP;

        return cst.data.cst_fnc.uid;

    } else {
        // indirect call
        const TValueId val = this->heapValFromOperand(op);
        if (VAL_INVALID == val)
            // Oops, it does not look as indirect call actually
            TRAP;

        return heap_.valGetCustom(/* TODO: check type */ 0, val);
    }
}

namespace {
    template <class TWL, class THeap>
    void digPointingObjects(TWL &wl, THeap &heap, TValueId val) {
        // go through all objects having the value
        SymHeap::TContObj cont;
        heap.usedBy(cont, val);
        BOOST_FOREACH(TObjId obj, cont) {

            // go through all super objects
            while (0 < obj) {
                wl.schedule(obj);
                obj = heap.objParent(obj);
            }
        }
    }

    template <class THeap>
    bool isHeapObject(const THeap &heap, TObjId obj) {
        if (obj <= 0)
            return false;

        for (; OBJ_INVALID != obj; obj = heap.objParent(obj))
            if (-1 != heap.cVar(obj))
                return false;

        return true;
    }

    template <class THeap>
    void digRootObject(THeap &heap, TValueId *pValue) {
        TObjId obj = heap.pointsTo(*pValue);
        if (obj < 0)
            TRAP;

        TObjId parent;
        while (OBJ_INVALID != (parent = heap.objParent(obj)))
            obj = parent;

        TValueId val = heap.placedAt(obj);
        if (val <= 0)
            TRAP;

        *pValue = val;
    }

    template <class THeap>
    bool digJunk(THeap &heap, TValueId *ptrVal) {
        if (*ptrVal <= 0 || UV_KNOWN != heap.valGetUnknown(*ptrVal))
            return false;

        if (VAL_INVALID != heap.valGetCustom(0, *ptrVal))
            // ignore custom values (e.g. fnc pointers)
            return false;

        TObjId obj = heap.pointsTo(*ptrVal);
        if (!isHeapObject(heap, obj))
            // non-heap object simply can't be JUNK
            return false;

        // only root objects can be destroyed
        digRootObject(heap, ptrVal);

        WorkList<TObjId> wl;
        digPointingObjects(wl, heap, *ptrVal);
        while (wl.next(obj)) {
            if (!isHeapObject(heap, obj))
                return false;

            const TValueId val = heap.placedAt(obj);
            if (val <= 0)
                TRAP;

            digPointingObjects(wl, heap, val);
        }

        return true;
    }

    template <class TCont, class THeap>
    void getPtrValues(TCont &dst, THeap &heap, TObjId obj) {
        std::stack<TObjId> todo;
        todo.push(obj);
        while (!todo.empty()) {
            const TObjId obj = todo.top();
            todo.pop();

            const struct cl_type *clt = heap.objType(obj);
            const enum cl_type_e code = (clt)
                ? clt->code
                : /* anonymous object of known size */ CL_TYPE_PTR;

            switch (code) {
                case CL_TYPE_PTR: {
                    const TValueId val = heap.valueOf(obj);
                    if (0 < val)
                        dst.push_back(val);

                    break;
                }

                case CL_TYPE_STRUCT:
                    for (int i = 0; i < clt->item_cnt; ++i) {
                        const TObjId subObj = heap.subObj(obj, i);
                        if (subObj < 0)
                            TRAP;

                        todo.push(subObj);
                    }
                    break;

                case CL_TYPE_ARRAY:
                case CL_TYPE_CHAR:
                case CL_TYPE_BOOL:
                case CL_TYPE_INT:
                    break;

                default:
                    // other types of value should be safe to ignore here
                    // but worth to check by a debugger at least once anyway
                    TRAP;
            }
        }
    }
}

bool SymHeapProcessor::checkForJunk(TValueId val) {
    bool detected = false;

    std::stack<TValueId> todo;
    todo.push(val);
    while (!todo.empty()) {
        TValueId val = todo.top();
        todo.pop();

        if (digJunk(heap_, &val)) {
            detected = true;
            const TObjId obj = heap_.pointsTo(val);
            if (obj <= 0)
                TRAP;

            // gather all values inside the junk object
            std::vector<TValueId> ptrs;
            getPtrValues(ptrs, heap_, obj);

            // destroy junk
            CL_WARN_MSG(lw_, "killing junk");
            heap_.objDestroy(obj);

            // schedule just created junk candidates for next wheel
            BOOST_FOREACH(TValueId ptrVal, ptrs) {
                todo.push(ptrVal);
            }
        }
    }

    return detected;
}

void SymHeapProcessor::heapObjDefineType(TObjId lhs, TValueId rhs) {
    const TObjId var = heap_.pointsTo(rhs);
    if (OBJ_INVALID == var)
        TRAP;

    const struct cl_type *clt = heap_.objType(lhs);
    if (!clt)
        return;

    if (clt->code != CL_TYPE_PTR)
        TRAP;

    // move to next clt
    // --> what are we pointing to actually?
    clt = clt->items[0].type;
    if (!clt)
        TRAP;

    if (CL_TYPE_VOID == clt->code)
        return;

    const int cbGot = heap_.objSizeOfAnon(var);
    const int cbNeed = clt->size;
    if (cbGot != cbNeed) {
        static const char szMsg[] =
            "amount of allocated memory not accurate";
        if (cbGot < cbNeed)
            CL_ERROR_MSG(lw_, szMsg);
        else
            CL_WARN_MSG(lw_, szMsg);

        CL_NOTE_MSG(lw_, "allocated: " << cbGot  << " bytes");
        CL_NOTE_MSG(lw_, " expected: " << cbNeed << " bytes");
    }

    heap_.objDefineType(var, clt);
}

void SymHeapProcessor::heapSetSingleVal(TObjId lhs, TValueId rhs) {
    // save the old value, which is going to be overwritten
    const TValueId oldValue = heap_.valueOf(lhs);
    if (VAL_INVALID == oldValue)
        TRAP;

    if (heap_.valPointsToAnon(rhs))
        // anonymous object is going to be specified by a type
        this->heapObjDefineType(lhs, rhs);

    heap_.objSetValue(lhs, rhs);
    if (this->checkForJunk(oldValue))
        this->printBackTrace();
}

void SymHeapProcessor::heapSetVal(TObjId lhs, TValueId rhs) {
    if (0 < rhs && UV_DEREF_FAILED == heap_.valGetUnknown(rhs)) {
        // we're already on an error path
        heap_.objSetValue(lhs, rhs);
        return;
    }

    // DFS for composite types
    typedef std::pair<TObjId, TValueId> TItem;
    std::stack<TItem> todo;
    push(todo, lhs, rhs);
    while (!todo.empty()) {
        TObjId lhs;
        TValueId rhs;
        boost::tie(lhs, rhs) = todo.top();
        todo.pop();

        const TObjId rObj = heap_.valGetCompositeObj(rhs);
        if (OBJ_INVALID == rObj) {
            // non-composite value
            this->heapSetSingleVal(lhs, rhs);
            continue;
        }

        const struct cl_type *clt = heap_.objType(rObj);
        if (!clt || clt->code != CL_TYPE_STRUCT || clt != heap_.objType(lhs))
            // type-info problem
            TRAP;

        // iterate through all fields
        for (int i = 0; i < clt->item_cnt; ++i) {
            const TObjId lSub = heap_.subObj(lhs, i);
            const TObjId rSub = heap_.subObj(rObj, i);
            if (lSub <= 0 || rSub <= 0)
                // composition problem
                TRAP;

            // schedule sub for next wheel
            const TValueId rSubVal = heap_.valueOf(rSub);
            push(todo, lSub, rSubVal);
        }
    }
}

void SymHeapProcessor::destroyObj(TObjId obj) {
    // gather destroyed values
    std::vector<TValueId> ptrs;
    getPtrValues(ptrs, heap_, obj);

    // destroy object recursively
    heap_.objDestroy(obj);

    // now check for JUNK
    bool junk = false;
    BOOST_FOREACH(TValueId val, ptrs) {
        if (this->checkForJunk(val))
            junk = true;
    }

    if (junk)
        // print backtrace at most once per one call of destroyObj()
        this->printBackTrace();
}

void SymHeapProcessor::execFree(const CodeStorage::TOperandList &opList) {
    if (/* dst + fnc + ptr */ 3 != opList.size())
        TRAP;

    if (CL_OPERAND_VOID != opList[0].code)
        // Oops, free() does not usually return a value
        TRAP;

    const TValueId val = heapValFromOperand(opList[/* ptr given to free() */2]);
    if (VAL_INVALID == val)
        // could not resolve value to be freed
        TRAP;

    switch (val) {
        case VAL_NULL:
            CL_DEBUG_MSG(lw_, "ignoring free() called with NULL value");
            return;

        default:
            break;
    }

    const EUnknownValue code = heap_.valGetUnknown(val);
    switch (code) {
        case UV_KNOWN:
            break;

        case UV_UNKNOWN:
            CL_DEBUG_MSG(lw_, "ignoring free() called on unknown value");
            return;

        case UV_UNINITIALIZED:
            CL_ERROR_MSG(lw_, "free() called on uninitialized value");
            this->printBackTrace();
            return;

        case UV_DEREF_FAILED:
            return;
    }

    const TObjId obj = heap_.pointsTo(val);
    switch (obj) {
        case OBJ_DELETED:
            CL_ERROR_MSG(lw_, "double free() detected");
            this->printBackTrace();
            return;

        case OBJ_LOST:
            // this is a double error in the analyzed program :-)
            CL_ERROR_MSG(lw_, "attempt to free a non-heap object"
                              ", which does not exist anyhow");
            this->printBackTrace();
            return;

        case OBJ_UNKNOWN:
        case OBJ_INVALID:
            TRAP;
        default:
            break;
    }

    const int cVar = heap_.cVar(obj);
    if (-1 != cVar) {
        CL_DEBUG("about to free var #" << cVar);
        CL_ERROR_MSG(lw_, "attempt to free a non-heap object");
        this->printBackTrace();
        return;
    }

    if (OBJ_INVALID != heap_.objParent(obj)) {
        CL_ERROR_MSG(lw_, "attempt to free a non-root object");
        this->printBackTrace();
        return;
    }

    CL_DEBUG_MSG(lw_, "executing free()");
    this->destroyObj(obj);
}

void SymHeapProcessor::execMalloc(TState &state,
                                  const CodeStorage::TOperandList &opList,
                                  bool fastMode)
{
    if (/* dst + fnc + size */ 3 != opList.size())
        TRAP;

    const struct cl_operand &dst = opList[0];
    const TObjId varLhs = this->heapObjFromOperand(dst);
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
    CL_DEBUG_MSG(lw_, "executing malloc(" << cbAmount << ")");
    const TObjId obj = heap_.objCreateAnon(cbAmount);
    if (OBJ_INVALID == obj)
        // unable to create a heap object
        TRAP;

    const TValueId val = heap_.placedAt(obj);
    if (val <= 0)
        TRAP;

    if (!fastMode) {
        // OOM state simulation
        this->heapSetVal(varLhs, VAL_NULL);
        state.insert(heap_);
    }

    // store the result of malloc
    this->heapSetVal(varLhs, val);
    state.insert(heap_);
}

namespace {
    template <int N_ARGS, class TOpList>
    bool chkVoidCall(const TOpList &opList)
    {
        if (/* dst + fnc */ 2 != opList.size() - N_ARGS)
            return false;
        else
            return (CL_OPERAND_VOID == opList[0].code);
    }

    template <int NTH, class TOpList>
    bool readPlotName(std::string *dst, const TOpList opList)
    {
        const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
        if (CL_OPERAND_CST != op.code)
            return false;

        const cl_cst &cst = op.data.cst;
        if (CL_TYPE_STRING != cst.code)
            return false;

        *dst = cst.data.cst_string.value;
        return true;
    }

    template <int NTH, class TOpList, class THeap>
    bool readHeapVal(TValueId *dst, const TOpList opList, const THeap &heap)
    {
        // FIXME: we might use the already existing instance instead
        SymHeapProcessor proc(const_cast<THeap &>(heap));

        const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
        const TValueId value = proc.heapValFromOperand(op);
        if (value < 0)
            return false;

        *dst = value;
        return true;
    }

    template <class TInsn, class THeap>
    bool readNameAndValue(std::string *pName, TValueId *pValue,
                          const TInsn &insn, const THeap &heap)
    {
        const CodeStorage::TOperandList &opList = insn.operands;
        const LocationWriter lw(&insn.loc);

        if (!chkVoidCall<2>(opList))
            return false;

        if (!readHeapVal<0>(pValue, opList, heap))
            return false;

        if (!readPlotName<1>(pName, opList))
            return false;

        return true;
    }

    template <class TStor, class TFnc, class THeap>
    bool fncFromHeapVal(const TStor &stor, const TFnc **dst, TValueId value,
                        const THeap &heap)
    {
        const int uid = heap.valGetCustom(/* pClt */ 0, value);
        if (-1 == uid)
            return false;

        // FIXME: get rid of the const_cast
        const TFnc *fnc = const_cast<TStor &>(stor).fncs[uid];
        if (!fnc)
            return false;

        *dst = fnc;
        return true;
    }

    void emitPrototypeError(const LocationWriter &lw, const std::string &fnc) {
        CL_WARN_MSG(lw, "incorrectly called "
                << fnc << "() not recognized as built-in");
    }

    void emitPlotError(const LocationWriter &lw, const std::string &plotName) {
        CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");
    }

    template <class TInsn, class THeap>
    bool callPlot(const TInsn &insn, const THeap &heap) {
        const CodeStorage::TOperandList &opList = insn.operands;
        const LocationWriter lw(&insn.loc);

        std::string plotName;
        if (!chkVoidCall<1>(opList) || !readPlotName<0>(&plotName, opList)) {
            emitPrototypeError(lw, "___sl_plot");
            return false;
        }

        const CodeStorage::Storage &stor = *insn.stor;
        SymHeapPlotter plotter(stor, heap);
        if (!plotter.plot(plotName))
            emitPlotError(lw, plotName);

        return true;
    }

    template <class TInsn, class THeap>
    bool callPlotByPtr(const TInsn &insn, const THeap &heap) {
        const LocationWriter lw(&insn.loc);

        std::string plotName;
        TValueId value;
        if (!readNameAndValue(&plotName, &value, insn, heap)) {
            emitPrototypeError(lw, "___sl_plot_by_ptr");
            return false;
        }

        const CodeStorage::Storage &stor = *insn.stor;
        SymHeapPlotter plotter(stor, heap);
        if (!plotter.plotHeapValue(plotName, value))
            emitPlotError(lw, plotName);

        return true;
    }

    template <class TInsn, class THeap>
    bool callPlotStackFrame(const TInsn &insn, const THeap &heap) {
        const CodeStorage::Storage &stor = *insn.stor;
        const LocationWriter lw(&insn.loc);

        std::string plotName;
        TValueId value;
        const CodeStorage::Fnc *fnc;

        if (!readNameAndValue(&plotName, &value, insn, heap)
                || !fncFromHeapVal(stor, &fnc, value, heap))
        {
            emitPrototypeError(lw, "___sl_plot_stack_frame");
            return false;
        }

        SymHeapPlotter plotter(stor, heap);
        if (!plotter.plotStackFrame(plotName, *fnc))
            emitPlotError(lw, plotName);

        return true;
    }
}

bool SymHeapProcessor::execCall(TState &dst, const CodeStorage::Insn &insn,
                                bool fastMode)
{
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
        this->execMalloc(dst, opList, fastMode);
        return true;
    }

    if (STREQ(fncName, "free")) {
        this->execFree(opList);
        goto call_done;
    }

    if (STREQ(fncName, "abort")) {
        if (opList.size() != 2 || opList[0].code != CL_OPERAND_VOID)
            TRAP;

        // do nothing for abort()
        goto call_done;
    }

    if (STREQ(fncName, "___sl_plot")
            && callPlot(insn, heap_))
        goto call_done;

    if (STREQ(fncName, "___sl_plot_stack_frame")
            && callPlotStackFrame(insn, heap_))
        goto call_done;

    if (STREQ(fncName, "___sl_plot_by_ptr")
            && callPlotByPtr(insn, heap_))
        goto call_done;

    // no built-in has been matched
    return false;

call_done:
    dst.insert(heap_);
    return true;
}

namespace {
    bool handleUnopTruthNotTrivial(TValueId &val) {
        switch (val) {
            case VAL_FALSE:
                val = VAL_TRUE;
                return true;

            case VAL_TRUE:
                val = VAL_FALSE;
                return true;

            case VAL_INVALID:
                return true;

            default:
                return false;
        }
    }
}

template <class THeap>
void handleUnopTruthNot(THeap &heap, TValueId &val, const struct cl_type *clt) {
    if (!clt || clt->code != CL_TYPE_BOOL)
        // inappropriate type for CL_UNOP_TRUTH_NOT
        TRAP;

    if (handleUnopTruthNotTrivial(val))
        // we are done
        return;

    const EUnknownValue code = heap.valGetUnknown(val);
    if (UV_KNOWN == code)
        // the value we got is not VAL_TRUE, VAL_FALSE, nor an unknown value
        TRAP;

    const TValueId origValue = val;
    val = heap.valDuplicateUnknown(origValue);
    // FIXME: not tested
    TRAP;
    heap.addEqIf(origValue, val, VAL_TRUE, /* neg */ true);
}

template <class THeap>
TValueId handleOpCmpBool(THeap &heap, enum cl_binop_e code,
                         const struct cl_type *dstClt, TValueId v1, TValueId v2)
{
    // TODO: describe the following magic somehow
    TValueId valElim = VAL_FALSE;
    switch (code) {
        case CL_BINOP_EQ:
            valElim = VAL_TRUE;
            // fall through!

        case CL_BINOP_NE:
            break;

        default:
            // crazy comparison of bool values
            TRAP;
            return VAL_INVALID;
    }
    if (v1 == valElim)
        return v2;
    if (v2 == valElim)
        return v1;

    if (v1 < 0 || v2 < 0)
        TRAP;

    // FIXME: not tested
    TRAP;
    bool result;
    if (!heap.proveEq(&result, v1, v2))
        return heap.valCreateUnknown(UV_UNKNOWN, dstClt);

    // invert if needed
    if (CL_BINOP_NE == code)
        result = !result;

    return (result)
        ? VAL_TRUE
        : VAL_FALSE;
}

template <class THeap>
TValueId handleOpCmpInt(THeap &heap, enum cl_binop_e code,
                        const struct cl_type *dstClt, TValueId v1, TValueId v2)
{
    if (v1 < 0 || v2 < 0)
        TRAP;

    // check if the values are equal
    bool eq;
    if (!heap.proveEq(&eq, v1, v2))
        // we don't know if the values are equal or not
        goto who_knows;

    switch (code) {
        case CL_BINOP_LT:
        case CL_BINOP_GT:
            if (eq)
                // we got either (x < x), or (x > x)
                return VAL_FALSE;
            else
                // bad luck, hard to compare unknown values for < >
                goto who_knows;

        case CL_BINOP_LE:
        case CL_BINOP_GE:
            if (eq)
                // we got either (x <= x), or (x >= x)
                return VAL_TRUE;
            else
                // bad luck, hard to compare unknown values for <= >=
                goto who_knows;

        case CL_BINOP_NE:
            eq = !eq;
            // fall through!

        case CL_BINOP_EQ:
            return (eq)
                ? VAL_TRUE
                : VAL_FALSE;

        default:
            TRAP;
    }

who_knows:
    // unknown result of int comparison
    TValueId val = heap.valCreateUnknown(UV_UNKNOWN, dstClt);
    switch (code) {
        case CL_BINOP_EQ:
            heap.addEqIf(val, v1, v2, /* neg */ false);
            return val;

        case CL_BINOP_NE:
            heap.addEqIf(val, v1, v2, /* neg */ true);
            return val;

        default:
            // EqIf predicate is not suitable for <, <=, >, >=
            return val;
    }
}

template <class THeap>
TValueId handleOpCmpPtr(THeap &heap, enum cl_binop_e code,
                        const struct cl_type *dstClt, TValueId v1, TValueId v2)
{
    if (v1 < 0 || v2 < 0)
        TRAP;

    switch (code) {
        case CL_BINOP_EQ:
        case CL_BINOP_NE:
            break;

        default:
            // crazy comparison of pointer values
            TRAP;
            return VAL_INVALID;
    }

    // check if the values are equal
    bool result;
    if (!heap.proveEq(&result, v1, v2)) {
        // we don't know if the values are equal or not
        const TValueId val = heap.valCreateUnknown(UV_UNKNOWN, dstClt);

        // store the relation over the triple (val, v1, v2) for posteriors
        heap.addEqIf(val, v1, v2, /* neg */ CL_BINOP_NE == code);
        return val;
    }

    // invert if needed
    if (CL_BINOP_NE == code)
        result = !result;

    return (result)
        ? VAL_TRUE
        : VAL_FALSE;
}

template <class THeap>
TValueId handleOpCmp(THeap &heap, enum cl_binop_e code,
                     const struct cl_type *dstClt, const struct cl_type *clt,
                     TValueId v1, TValueId v2)
{
    // clt is assumed to be valid at this point
    switch (clt->code) {
        case CL_TYPE_PTR:  return handleOpCmpPtr (heap, code, dstClt, v1, v2);
        case CL_TYPE_BOOL: return handleOpCmpBool(heap, code, dstClt, v1, v2);
        case CL_TYPE_INT:  return handleOpCmpInt (heap, code, dstClt, v1, v2);
        default:
            // unexpected clt->code
            TRAP;
            return VAL_INVALID;
    }
}

// template for generic (unary, binary, ...) operator handlers
template <int ARITY, class TProc>
struct OpHandler {
    static TValueId handleOp(TProc &proc, int code, const TValueId rhs[ARITY],
                             const struct cl_type *clt[ARITY +/* dst */1]);
};

// unary operator handler
template <class TProc>
struct OpHandler</* unary */ 1, TProc> {
    static TValueId handleOp(TProc &proc, int iCode, const TValueId rhs[1],
                             const struct cl_type *clt[1 + /* dst type */ 1])
    {
        TValueId val = rhs[0];

        const enum cl_unop_e code = static_cast<enum cl_unop_e>(iCode);
        switch (code) {
            case CL_UNOP_TRUTH_NOT:
                handleUnopTruthNot(proc.heap_, val, clt[0]);
                // fall through!

            case CL_UNOP_ASSIGN:
                break;

            default:
                TRAP;
        }

        return val;
    }
};

namespace {
    bool operator==(const struct cl_type &cltA, const struct cl_type &cltB) {
        if (cltA.uid == cltB.uid)
            return true;

        const enum cl_type_e code = cltA.code;
        if (cltB.code != code)
            return false;

        switch (code) {
            case CL_TYPE_PTR: {
                const struct cl_type *nextA = cltA.items[0].type;
                const struct cl_type *nextB = cltB.items[0].type;

                // FIXME: use an explicit stack instead of recursion
                return operator==(*nextA, *nextB);
            }

            case CL_TYPE_STRUCT:
                // TODO: dive into fields

            default:
                // TODO
                TRAP;
                return false;
        }
    }

    bool operator!=(const struct cl_type &cltA, const struct cl_type &cltB) {
        return !(cltA == cltB);
    }
}

// binary operator handler
template <class TProc>
struct OpHandler</* binary */ 2, TProc> {
    static TValueId handleOp(TProc &proc, int iCode, const TValueId rhs[2],
                             const struct cl_type *clt[2 + /* dst type */ 1])
    {
        const struct cl_type *const cltA = clt[0];
        const struct cl_type *const cltB = clt[1];
        if (!cltA || !cltB)
            // type-info is missing
            TRAP;

        SymHeap &heap = proc.heap_;
        if (*cltA != *cltB) {
            // we don't support arrays, pointer arithmetic and the like,
            // the types therefor have to match with each other for a binary
            // operator
            CL_ERROR_MSG(proc.lw_,
                    "mixing of types for a binary operator not supported yet");
            CL_NOTE_MSG(proc.lw_,
                    "the analysis may crash because of the error above");
            return heap.valCreateUnknown(UV_UNKNOWN, cltA);
        }

        const enum cl_binop_e code = static_cast<enum cl_binop_e>(iCode);
        switch (code) {
            case CL_BINOP_EQ:
            case CL_BINOP_NE:
            case CL_BINOP_LT:
            case CL_BINOP_GT:
            case CL_BINOP_LE:
            case CL_BINOP_GE:
                return handleOpCmp(heap, code, clt[2], cltA, rhs[0], rhs[1]);

            case CL_BINOP_PLUS:
            case CL_BINOP_MINUS:
                CL_WARN_MSG(proc.lw_, "binary operator not implemented yet");
                return heap.valCreateUnknown(UV_UNKNOWN, cltA);

            default:
                TRAP;
                return VAL_INVALID;
        }
    }
};

// C++ does not support partial specialisation of function templates, this helps
template <int ARITY, class TProc>
TValueId handleOp(TProc &proc, int code, const TValueId rhs[ARITY],
                  const struct cl_type *clt[ARITY + /* dst type */ 1])
{
    return OpHandler<ARITY, TProc>::handleOp(proc, code, rhs, clt);
}


template <int ARITY>
void SymHeapProcessor::execOp(const CodeStorage::Insn &insn, std::list<SymHeap> &todo) {
    // resolve lhs
    TObjId varLhs = OBJ_INVALID;
    const struct cl_operand &dst = insn.operands[/* dst */ 0];
    if (!this->lhsFromOperand(&varLhs, dst)) {
        if(heap_.objIsAbstract(varLhs))
            Concretize(heap_, varLhs, todo); // add to todo-list if abstract variant possible
        else
            return;
    }

    // store cl_type of dst operand
    const struct cl_type *clt[ARITY + /* dst type */ 1];
    clt[/* dst type */ ARITY] = dst.type;

    // gather rhs values (and type-info)
    TValueId rhs[ARITY];
    for (int i = 0; i < ARITY; ++i) {
        const struct cl_operand &op = insn.operands[i + /* [+dst] */ 1];
        clt[i] = op.type;
        rhs[i] = this->heapValFromOperand(op);
        if (VAL_INVALID == rhs[i])
            TRAP;
        if(heap_.valIsAbstract(rhs[i])) {
            TObjId o = heap_.pointsTo(rhs[i]);    // target object
            if(heap_.objIsAbstract(o))
                Concretize(heap_,o,todo);   // add to todo-list if abstract variant possible
        }
    }

    // ASSERT: all operands are non-abstract

    // handle generic operator and store result
    const TValueId valResult = handleOp<ARITY>(*this, insn.subCode, rhs, clt);
    this->heapSetVal(varLhs, valResult);
}

bool SymHeapProcessor::exec(TState &dst, std::list<SymHeap> &todo, const CodeStorage::Insn &insn,
                            bool fastMode)
{
    lw_ = &insn.loc;
    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execOp<1>(insn, todo);
            break;

        case CL_INSN_BINOP:
            this->execOp<2>(insn, todo); 
            break;

        case CL_INSN_CALL:
            return this->execCall(dst, insn, fastMode);

        default:
            TRAP;
    }

    // we've got only one resulting heap, insert it into the target state
    dst.insert(heap_);
    return true;
}
