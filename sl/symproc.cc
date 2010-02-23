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
    bool handleUnopTruthNotTrivial(int &val) {
        using namespace SymbolicHeap;

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
void handleUnopTruthNot(THeap &heap, int &val, const struct cl_type *clt) {
    using namespace SymbolicHeap;

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

    val = heap.valDuplicateUnknown(val);
    // TODO: remember relation among unknown values ... challenge? :-)
}

namespace {
    int /* val */ handleOpCmpBool(enum cl_binop_e code, int v1, int v2)
    {
        using namespace SymbolicHeap;

        // TODO: describe the following magic somehow
        int /* val */ valElim = VAL_FALSE;
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

        // trivial value comparison
        bool result = (v1 == v2);
        if (CL_BINOP_NE == code)
            result = !result;

        return (result)
            ? VAL_TRUE
            : VAL_FALSE;
    }
}

template <class THeap>
int /* val */ handleOpCmpInt(THeap &heap, enum cl_binop_e code, int v1, int v2,
                             const struct cl_type *clt)
{
        using namespace SymbolicHeap;
        if (v1 < 0 || v2 < 0)
            TRAP;

        // trivial value comparison
        bool result = (v1 == v2);

        switch (code) {
            case CL_BINOP_EQ:
                break;

            case CL_BINOP_NE:
                result = !result;
                break;

            default:
                return heap.valCreateUnknown(UV_UNKNOWN, clt);
        }

        return (result)
            ? VAL_TRUE
            : VAL_FALSE;
}

namespace {
    int /* val */ handleOpCmpPtr(enum cl_binop_e code, int v1, int v2)
    {
        using namespace SymbolicHeap;
        if (v1 < 0 || v2 < 0)
            TRAP;

        // trivial value comparison
        bool result = (v1 == v2);

        switch (code) {
            case CL_BINOP_EQ:
                break;

            case CL_BINOP_NE:
                result = !result;
                break;

            default:
                // crazy comparison of pointer values
                TRAP;
                return VAL_INVALID;
        }

        return (result)
            ? VAL_TRUE
            : VAL_FALSE;
    }
}

template <class THeap>
int /* val */ handleOpCmp(THeap &heap, enum cl_binop_e code,
                          const struct cl_type *clt, int v1, int v2)
{
    // clt is assumed to be valid at this point
    switch (clt->code) {
        case CL_TYPE_PTR:
            return handleOpCmpPtr(code, v1, v2);

        case CL_TYPE_BOOL:
            return handleOpCmpBool(code, v1, v2);

        case CL_TYPE_INT:
            return handleOpCmpInt(heap, code, v1, v2, clt);

        default:
            // unexpected clt->code
            TRAP;
            return SymbolicHeap::VAL_INVALID;
    }
}

// template for generic (unary, binary, ...) operator handlers
template <int ARITY, class THeap>
struct OpHandler {
    static int /* val */ handleOp(THeap &heap, int code, const int rhs[ARITY],
                                  const struct cl_type *clt[ARITY]);
};

// unary operator handler
template <class THeap>
struct OpHandler</* unary */ 1, THeap> {
    static int handleOp(THeap &heap, int iCode, const int rhs[1],
                        const struct cl_type *clt[1])
    {
        int val = rhs[0];

        const enum cl_unop_e code = static_cast<enum cl_unop_e>(iCode);
        switch (code) {
            case CL_UNOP_TRUTH_NOT:
                handleUnopTruthNot(heap, val, clt[0]);
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
template <class THeap>
struct OpHandler</* binary */ 2, THeap> {
    static int handleOp(THeap &heap, int iCode, const int rhs[2],
                        const struct cl_type *clt[2])
    {
        using namespace SymbolicHeap;

        const struct cl_type *const cltA = clt[0];
        const struct cl_type *const cltB = clt[1];
        if (!cltA || !cltB)
            // type-info is missing
            TRAP;

        if (*cltA != *cltB)
            // we don't support arrays, pointer arithmetic and the like,
            // the types therfore have to match with each other for a binary
            // operator
            TRAP;

        const enum cl_binop_e code = static_cast<enum cl_binop_e>(iCode);
        switch (code) {
            case CL_BINOP_EQ:
            case CL_BINOP_NE:
            case CL_BINOP_LT:
            case CL_BINOP_GT:
            case CL_BINOP_LE:
            case CL_BINOP_GE:
                return handleOpCmp(heap, code, cltA, rhs[0], rhs[1]);

            case CL_BINOP_PLUS:
            case CL_BINOP_MINUS:
                CL_WARN("binary operator not implemented yet");
                return heap.valCreateUnknown(UV_UNKNOWN, cltA);

            default:
                TRAP;
                return VAL_INVALID;
        }
    }
};

// C++ does not support partial specialisation of function templates, this helps
template <int ARITY, class THeap>
int handleOp(THeap &heap, int code, const int rhs[ARITY],
             const struct cl_type *clt[ARITY])
{
    return OpHandler<ARITY, THeap>::handleOp(heap, code, rhs, clt);
}

template <int ARITY>
void SymHeapProcessor::execOp(const CodeStorage::Insn &insn) {
    // resolve lhs
    int varLhs;
    if (!this->lhsFromOperand(&varLhs, insn.operands[/* dst */ 0]))
        return;

    // gather rhs values (and type-info)
    int rhs[ARITY];
    const struct cl_type *clt[ARITY];
    for (int i = 0; i < ARITY; ++i) {
        const struct cl_operand &op = insn.operands[i + /* [+dst] */ 1];
        clt[i] = op.type;
        rhs[i] = this->heapValFromOperand(op);
        if (SymbolicHeap::VAL_INVALID == rhs[i])
            TRAP;
    }

    // handle generic operator and store result
    const int valResult = handleOp<ARITY>(heap_, insn.subCode, rhs, clt);
    this->heapSetVal(varLhs, valResult);
}

bool SymHeapProcessor::exec(TState &dst, const CodeStorage::Insn &insn,
                            bool fastMode)
{
    lw_ = &insn.loc;
    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execOp<1>(insn);
            break;

        case CL_INSN_BINOP:
            this->execOp<2>(insn);
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
