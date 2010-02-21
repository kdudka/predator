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
#include "symproc.hh"

#include "btprint.hh"
#include "cl_private.hh"
#include "symheap.hh"
#include "symplot.hh"
#include "symstate.hh"

#include <set>
#include <stack>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymHeapProcessor implementation
void SymHeapProcessor::printBackTrace() {
    if (!btPrinter_)
        TRAP;

    btPrinter_->printBackTrace();
}

int /* val */ SymHeapProcessor::heapValFromCst(const struct cl_operand &op) {
    using namespace SymbolicHeap;

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
                return (cst.data.cst_int.value)
                    ? heap_.valCreateUnknown(UV_UNKNOWN, op.type)
                    : VAL_NULL;
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

void SymHeapProcessor::heapVarHandleAccessorDeref(int *pObj)
{
    using namespace SymbolicHeap;
    EUnknownValue code;

    // attempt to dereference
    const int val = heap_.valueOf(*pObj);
    switch (val) {
        case VAL_NULL:
            CL_ERROR_MSG(lw_, "dereference of NULL value");
            this->printBackTrace();
            goto fail;

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
            this->printBackTrace();
            // fall through!

        case UV_DEREF_FAILED:
            goto fail;
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
            // CL_ACCESSOR_REF will be processed wihtin heapValFromOperand()
            // on the way out from here ... otherwise we are encountering
            // a bug!
            return;

        case CL_ACCESSOR_DEREF_ARRAY:
            CL_WARN_MSG(lw_, "CL_ACCESSOR_DEREF_ARRAY not implemented yet");
            *pObj = SymbolicHeap::OBJ_DEREF_FAILED;
            return;
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

    // process all accessors (only CL_ACCESSOR_DEREF for now)
    const struct cl_accessor *ac = op.accessor;
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

    *pVar = this->heapVarFromOperand(op);
    switch (*pVar) {
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
            return true;
    }
}

namespace {
    template <class THeap>
    int /* val */ valueFromVar(THeap &heap, int var, const struct cl_type *clt,
                               const struct cl_accessor *ac)
    {
        using namespace SymbolicHeap;
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

int /* val */ SymHeapProcessor::heapValFromOperand(const struct cl_operand &op)
{
    using namespace SymbolicHeap;

    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
        case CL_OPERAND_REG:
            return valueFromVar(heap_,
                    this->heapVarFromOperand(op),
                    op.type, op.accessor);

        case CL_OPERAND_CST:
            return this->heapValFromCst(op);

        default:
            TRAP;
            return VAL_INVALID;
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
    bool isHeapObject(const THeap &heap, int obj) {
        using SymbolicHeap::OBJ_INVALID;
        if (obj <= 0)
            return false;

        for (; OBJ_INVALID != obj; obj = heap.varParent(obj))
            if (-1 != heap.cVar(obj))
                return false;

        return true;
    }

    template <class THeap>
    bool digJunk(THeap &heap, int ptrVal) {
        using namespace SymbolicHeap;
        if (ptrVal <= 0 || UV_KNOWN != heap.valGetUnknown(ptrVal))
            return false;

        if (VAL_INVALID != heap.valGetCustom(0, ptrVal))
            // ignore custom values (e.g. fnc pointers)
            return false;

        const int obj = heap.pointsTo(ptrVal);
        if (!isHeapObject(heap, obj))
            // non-heap object simply can't be JUNK
            return false;

        if (-1 != heap.varParent(obj))
            // ignore non-roots
            return false;

        std::stack<int /* var */> todo;
        std::set<int /* var */> done;
        digPointingObjects(todo, done, heap, ptrVal);
        while (!todo.empty()) {
            const int obj = todo.top();
            todo.pop();
            done.insert(obj);
            if (!isHeapObject(heap, obj))
                return false;

            const int val = heap.placedAt(obj);
            if (val <= 0)
                TRAP;

            digPointingObjects(todo, done, heap, val);
        }

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

bool SymHeapProcessor::checkForJunk(int val) {
    using namespace SymbolicHeap;
    bool detected = false;

    std::stack<int /* val */> todo;
    todo.push(val);
    while (!todo.empty()) {
        const int val = todo.top();
        todo.pop();

        if (digJunk(heap_, val)) {
            detected = true;
            const int obj = heap_.pointsTo(val);
            if (obj <= 0)
                TRAP;

            // gather all values inside the junk object
            std::vector<int /* val */> ptrs;
            getPtrValues(ptrs, heap_, obj);

            // destroy junk
            CL_WARN_MSG(lw_, "killing junk");
            heap_.objDestroy(obj);

            // schedule just created junk candidates for next wheel
            BOOST_FOREACH(int ptrVal, ptrs) {
                todo.push(ptrVal);
            }
        }
    }

    return detected;
}

void SymHeapProcessor::heapVarDefineType(int /* obj */ lhs, int /* val */ rhs) {
    const int var = heap_.pointsTo(rhs);
    if (SymbolicHeap::OBJ_INVALID == var)
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

    const int cbGot = heap_.varSizeOfAnon(var);
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

    heap_.varDefineType(var, clt);
}

void SymHeapProcessor::heapSetSingleVal(int /* obj */ lhs, int /* val */ rhs) {
    // save the old value, which is going to be overwritten
    const int oldValue = heap_.valueOf(lhs);
    if (SymbolicHeap::VAL_INVALID == oldValue)
        TRAP;

    if (heap_.valPointsToAnon(rhs))
        // anonymous object is going to be specified by a type
        this->heapVarDefineType(lhs, rhs);

    heap_.objSetValue(lhs, rhs);
    if (this->checkForJunk(oldValue))
        this->printBackTrace();
}

void SymHeapProcessor::heapSetVal(int /* obj */ lhs, int /* val */ rhs) {
    using namespace SymbolicHeap;

    // DFS for composite types
    typedef std::pair<int /* obj */, int /* val */> TItem;
    std::stack<TItem> todo;
    push(todo, lhs, rhs);
    while (!todo.empty()) {
        int lhs, rhs;
        boost::tie(lhs, rhs) = todo.top();
        todo.pop();

        const int rObj = heap_.valGetCompositeObj(rhs);
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
            const int lSub = heap_.subVar(lhs, i);
            const int rSub = heap_.subVar(rObj, i);
            if (lSub <= 0 || rSub <= 0)
                // composition problem
                TRAP;

            // schedule sub for next wheel
            const int rSubVal = heap_.valueOf(rSub);
            push(todo, lSub, rSubVal);
        }
    }
}

void SymHeapProcessor::destroyObj(int obj) {
    // gather destroyed values
    std::vector<int> ptrs;
    getPtrValues(ptrs, heap_, obj);

    // destroy object recursively
    heap_.objDestroy(obj);

    // now check for JUNK
    bool junk = false;
    BOOST_FOREACH(int val, ptrs) {
        if (this->checkForJunk(val))
            junk = true;
    }

    if (junk)
        // print backtrace at most once per one call of destroyObj()
        this->printBackTrace();
}

int SymHeapProcessor::heapCmpVals(int val1, int val2) {
    using namespace SymbolicHeap;

    if (val1 < 0 || val2 <0)
        // special values are not intended to be handled here
        TRAP;

    const int co1 = heap_.valGetCompositeObj(val1);
    const int co2 = heap_.valGetCompositeObj(val2);
    if (OBJ_INVALID != co1 || OBJ_INVALID != co2)
        // composite objects are not allowed to be compared for now
        TRAP;

    if (val1 < val2)
        return -1;
    else if (val1 > val2)
        return 1;
    else
        return 0;
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
            CL_DEBUG_MSG(lw_, "ignoring free() called with NULL value");
            return;
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

    const int obj = heap_.pointsTo(val);
    switch (obj) {
        case OBJ_DELETED:
            CL_ERROR_MSG(lw_, "double free() detected");
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

    CL_DEBUG_MSG(lw_, "executing free()");
    // TODO: check for possible free() of non-root
    this->destroyObj(obj);
}

void SymHeapProcessor::execMalloc(TState &state,
                                  const CodeStorage::TOperandList &opList,
                                  bool fastMode)
{
    using namespace SymbolicHeap;
    if (/* dst + fnc + size */ 3 != opList.size())
        TRAP;

    const struct cl_operand &dst = opList[0];
    const int varLhs = this->heapVarFromOperand(dst);
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
    const int obj = heap_.varCreateAnon(cbAmount);
    if (OBJ_INVALID == obj)
        // unable to create dynamic variable
        TRAP;

    const int val = heap_.placedAt(obj);
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
    bool readHeapVal(int *dst, const TOpList opList, const THeap &heap)
    {
        // FIXME: we might use the already existing instance instead
        SymHeapProcessor proc(const_cast<THeap &>(heap));

        const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
        const int value = proc.heapValFromOperand(op);
        if (value < 0)
            return false;

        *dst = value;
        return true;
    }

    template <class TInsn, class THeap>
    bool readNameAndValue(std::string *pName, int *pValue,
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
    bool fncFromHeapVal(const TStor &stor, const TFnc **dst, int value,
                        const THeap &heap)
    {
        const int uid = heap.valGetCustom(/* pClt */ 0, value);
        if (-1 == uid)
            return false;

        // FIXME: get rid of the const_cast
        const TFnc *fnc = const_cast<TStor &>(stor).anyFncById[uid];
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
            emitPrototypeError(lw, "sl_plot");
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
        int value;
        if (!readNameAndValue(&plotName, &value, insn, heap)) {
            emitPrototypeError(lw, "sl_plot_by_ptr");
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
        int value;
        const CodeStorage::Fnc *fnc;

        if (!readNameAndValue(&plotName, &value, insn, heap)
                || !fncFromHeapVal(stor, &fnc, value, heap))
        {
            emitPrototypeError(lw, "sl_plot_stack_frame");
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

    if (STREQ(fncName, "sl_plot")
            && callPlot(insn, heap_))
        goto call_done;

    if (STREQ(fncName, "sl_plot_stack_frame")
            && callPlotStackFrame(insn, heap_))
        goto call_done;

    if (STREQ(fncName, "sl_plot_by_ptr")
            && callPlotByPtr(insn, heap_))
        goto call_done;

    // no built-in has been matched
    return false;

call_done:
    dst.insert(heap_);
    return true;
}

namespace {
    template <class THeap>
    void handleUnopTruthNot(THeap &heap, int &val) {
        using namespace SymbolicHeap;

        switch (val) {
            case VAL_FALSE:
                val = VAL_TRUE;
                return;

            case VAL_TRUE:
                val = VAL_FALSE;
                return;

            case VAL_INVALID:
                return;

            default:
                break;
        }

        const EUnknownValue code = heap.valGetUnknown(val);
        if (UV_KNOWN == code)
            // the value we got is not VAL_TRUE, VAL_FALSE, nor an unknown value
            TRAP;

        val = heap.valDuplicateUnknown(val);
        // TODO: remember relation among unknown values ... challenge? :-)
    }

    template <class THeap>
    void handleUnop(THeap &heap, enum cl_unop_e code, int &val) {
        switch (code) {
            case CL_UNOP_TRUTH_NOT:
                handleUnopTruthNot(heap, val);
                // fall through!

            case CL_UNOP_ASSIGN:
                break;

            default:
                TRAP;
        }
    }
}

void SymHeapProcessor::execUnary(const CodeStorage::Insn &insn) {
    // resolve lhs
    int varLhs;
    if (!this->lhsFromOperand(&varLhs, insn.operands[0]))
        return;

    // resolve rhs
    int valRhs = this->heapValFromOperand(insn.operands[1]);
    if (SymbolicHeap::VAL_INVALID == valRhs)
        TRAP;

    // handle unary operator
    const enum cl_unop_e code = static_cast<enum cl_unop_e> (insn.subCode);
    handleUnop(heap_, code, valRhs);

    // store result
    this->heapSetVal(varLhs, valRhs);
}

namespace {
    template <class THeap>
    int handleSpecialValues(THeap &/*heap*/, int val1, int val2) {
        using namespace SymbolicHeap;

        if (VAL_NULL <= val1 && VAL_NULL <= val2)
            return VAL_FALSE;

        // TODO: handle unknown values here
        const int lower = (val1 < val2) ? val1 : val2;
        switch (lower) {
            case VAL_INVALID:
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

    // resolve dst
    int dst;
    if (!lhsFromOperand(&dst, insn.operands[0]))
        return;

    // resolve src1, src2
    const int val1 = heapValFromOperand(insn.operands[1]);
    const int val2 = heapValFromOperand(insn.operands[2]);
    int val = handleSpecialValues(heap_, val1, val2);

    bool neg = false;
    bool result;
    if (val)
        goto done;

    switch (code) {
        case CL_BINOP_NE:
            neg = true;
            // fall through!
        case CL_BINOP_EQ:
            break;

        default:
            CL_WARN_MSG(lw_, "binary operator not implemented yet");
            val = heap_.valCreateUnknown(UV_UNKNOWN, insn.operands[0].type);
            goto done;
    }

    // execute CL_BINOP_EQ/CL_BINOP_NE
    result = (0 == this->heapCmpVals(val1, val2));
    if (neg)
        result = !result;

    // convert bool result to a heap value
    val = (result)
        ? VAL_TRUE
        : VAL_FALSE;

done:
    // store resulting value
    // FIXME: should we use SymHeapProcessor here?
    heap_.objSetValue(dst, val);
}

bool SymHeapProcessor::exec(TState &dst, const CodeStorage::Insn &insn,
                            bool fastMode)
{
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
            return this->execCall(dst, insn, fastMode);

        default:
            TRAP;
            return true;
    }
}
