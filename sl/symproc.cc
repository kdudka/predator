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

#include "symabstract.hh"
#include "symbt.hh"
#include "symgc.hh"
#include "symheap.hh"
#include "symplot.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "util.hh"

#include <stack>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymProc implementation
TValueId SymProc::heapValFromCst(const struct cl_operand &op) {
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
            SE_TRAP;
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
                heap_.neqOp(SymHeap::NEQ_ADD, val, VAL_NULL);
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
            SE_TRAP;
            return VAL_INVALID;
    }
}

bool SymProc::checkForInvalidDeref(TObjId obj) {
    switch (obj) {
        case OBJ_LOST:
            CL_ERROR_MSG(lw_, "dereference of non-existing non-heap object");
            break;

        case OBJ_DELETED:
            CL_ERROR_MSG(lw_, "dereference of already deleted heap object");
            break;

        case OBJ_UNKNOWN:
        case OBJ_INVALID:
            SE_TRAP;

        default:
            // valid object
            return false;
    }

    // an invalid dereference has been detected
    bt_->printBackTrace();
    return true;
}

TObjId SymProc::handleDerefCore(TValueId val) {
    // do we really know the value?
    const EUnknownValue code = heap_.valGetUnknown(val);
    switch (code) {
        case UV_ABSTRACT:
#if SE_SELF_TEST
            SE_TRAP;
#endif
        case UV_KNOWN:
            break;

        case UV_UNKNOWN:
            CL_ERROR_MSG(lw_, "dereference of unknown value");
            bt_->printBackTrace();
            return OBJ_DEREF_FAILED;

        case UV_UNINITIALIZED:
            CL_ERROR_MSG(lw_, "dereference of uninitialized value");
            bt_->printBackTrace();
            return OBJ_DEREF_FAILED;
    }

    // look for the target
    const TObjId target = heap_.pointsTo(val);
    if (this->checkForInvalidDeref(target))
        // error already handled
        return OBJ_DEREF_FAILED;

    // all OK
    return target;
}

void SymProc::heapObjHandleAccessorItem(TObjId *pObj,
                                        const struct cl_accessor *ac)
{
    if (*pObj < 0)
        // nothing to do at this level, keep going...
        return;

    // access subObj
    const int id = ac->data.item.id;
    *pObj = heap_.subObj(*pObj, id);
    SE_BREAK_IF(OBJ_INVALID == *pObj);
}

void SymProc::heapObjHandleAccessor(TObjId *pObj,
                                    const struct cl_accessor *ac)
{
    const enum cl_accessor_e code = ac->code;
    switch (code) {
        case CL_ACCESSOR_DEREF:
            // this should have been handled elsewhere
            SE_TRAP;
            return;

        case CL_ACCESSOR_ITEM:
            this->heapObjHandleAccessorItem(pObj, ac);
            return;

        case CL_ACCESSOR_REF:
            // CL_ACCESSOR_REF will be processed within heapValFromObj()
            // on the way out from here ... otherwise we are encountering
            // a bug!
            return;

        case CL_ACCESSOR_DEREF_ARRAY:
            CL_WARN_MSG(lw_, "CL_ACCESSOR_DEREF_ARRAY not implemented yet");
            *pObj = OBJ_DEREF_FAILED;
            return;
    }
}

namespace {

TObjId varFromOperand(const struct cl_operand &op, const SymHeap &sh,
                      const SymBackTrace *bt)
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
            SE_TRAP;
            return OBJ_INVALID;
    }

    const int nestLevel = bt->countOccurrencesOfTopFnc();
    const CVar cVar(uid, nestLevel);
    return sh.objByCVar(cVar);
}

} // namespace

void SymProc::resolveAliasing(TValueId *pVal, const struct cl_type *cltTarget) {
    TValueId val = *pVal;
    const struct cl_type *clt = heap_.valType(val);
    if (!clt)
        // no type-info, giving up...
        return;

    if (*clt == *cltTarget)
        // no aliasing, all seems OK
        return;

    if (CL_TYPE_PTR == clt->code && CL_TYPE_PTR == cltTarget->code
            && CL_TYPE_FNC !=       clt->items[0].type->code
            && CL_TYPE_FNC != cltTarget->items[0].type->code)
        // multi-level dereference may be handled eventually later, as long
        // as both _target_ pointers are data pointers.  Generally it's not
        // guaranteed that sizeof(void *) == sizeof(void (*)())
        return;

    // get all aliases of 'val'
    SymHeap::TContValue aliasing;
    heap_.gatherValAliasing(aliasing, val);

    // go through the list and look for a suitable one
    unsigned cntMatch = 0;
    BOOST_FOREACH(const TValueId alias, aliasing) {
        const struct cl_type *clt = heap_.valType(alias);
        if (!clt)
            continue;

        if (*clt != *cltTarget)
            continue;

        // match!
        cntMatch++;
        val = alias;
#if !SE_SELF_TEST
        break;
#endif
    }

    if (!cntMatch) {
        CL_ERROR_MSG(lw_,
                "type of the pointer being dereferenced does not match "
                "type of the target object");
        return;
    }

    // ensure we've got a _deterministic_ match
    SE_BREAK_IF(1 < cntMatch);

    CL_DEBUG_MSG(lw_, "value alias matched during dereference of a pointer");
    *pVal = val;
}

void SymProc::resolveOffValue(TValueId *pVal, const struct cl_accessor **pAc) {
    const TValueId val = *pVal;
    if (val <= 0 || UV_UNKNOWN != heap_.valGetUnknown(val))
        // we're not interested here in such a value
        return;

    const struct cl_accessor *ac = *pAc;
    if (!ac || ac->code != CL_ACCESSOR_ITEM)
        // no selectors --> no offset to compute
        return;

    // going through the chain of CL_ACCESSOR_ITEM, look for the target
    int off = 0;
    TValueId valTarget = VAL_INVALID;
    while (ac && ac->code == CL_ACCESSOR_ITEM && VAL_INVALID == valTarget) {
        // compute cumulative offset at the current level
        const struct cl_type *clt = ac->type;
        const int nth = ac->data.item.id;
        off += clt->items[nth].offset;

        if (off)
            // attempt to resolve off-value
            valTarget = heap_.valGetByOffset(SymHeapCore::TOffVal(val, off));

        // jump to the next accessor
        ac = ac->next;
    }

    if (VAL_INVALID == valTarget)
        // not found
        return;

    // successfully resolved off-value
    *pAc  = ac;
    *pVal = valTarget;
}

namespace {
    const struct cl_type* targetTypeOfPtr(const struct cl_type *clt) {
        SE_BREAK_IF(!clt || clt->code != CL_TYPE_PTR || clt->item_cnt != 1);
        clt = clt->items[0].type;
        SE_BREAK_IF(!clt);
        return clt;
    }
}

// TODO: there should be probably some by-pass for accessors chains of the form
//       CL_ACCESSOR_DEREF -> ... -> CL_ACCESSOR_REF as we should not claim
//       there is an invalid reference, if only an address is actually being
//       computed
void SymProc::handleDeref(TObjId *pObj, const struct cl_accessor **pAc) {
    // mark the current accessor as done in advance, and move to the next one
    *pAc = (*pAc)->next;

    const TObjId obj = *pObj;
    if (obj < 0)
        // we're already on an error path
        return;

    // derive the target type of the pointer we're dereferencing
    const struct cl_type *clt = heap_.objType(obj);
    const struct cl_type *cltTarget = targetTypeOfPtr(clt);
    SE_BREAK_IF(!clt || !cltTarget || cltTarget->code == CL_TYPE_VOID);

    // read the value inside the pointer
    TValueId val = heap_.valueOf(obj);
    switch (val) {
        case VAL_NULL:
            CL_ERROR_MSG(lw_, "dereference of NULL value");
            bt_->printBackTrace();
            // fall through!

        case VAL_DEREF_FAILED:
            *pObj = OBJ_DEREF_FAILED;
            return;

        case VAL_INVALID:
            SE_TRAP;
        default:
            break;
    }

    // attempt to resolve aliasing
    this->resolveAliasing(&val, cltTarget);

    // attempt to resolve an off-value
    this->resolveOffValue(&val, pAc);

    // now perform the actual dereference
    *pObj = this->handleDerefCore(val);
}

TObjId SymProc::heapObjFromOperand(const struct cl_operand &op) {
    // resolve static variable
    TObjId obj = varFromOperand(op, heap_, bt_);
    SE_BREAK_IF(OBJ_INVALID == obj);

    const struct cl_accessor *ac = op.accessor;
    if (!ac)
        // no accessors, we're done
        return obj;

    // first check for dereference and handle any off-value eventually
    if (ac->code == CL_ACCESSOR_DEREF)
        this->handleDeref(&obj, &ac);

    // we don't support chaining of CL_ACCESSOR_DEREF (yet?)
    SE_BREAK_IF(ac && ac->code == CL_ACCESSOR_DEREF);

    while (ac) {
        this->heapObjHandleAccessor(&obj, ac);
        ac = ac->next;
    }

    return obj;
}

bool SymProc::lhsFromOperand(TObjId *pObj, const struct cl_operand &op) {
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
            SE_TRAP;

        default:
            return true;
    }
}

TValueId SymProc::heapValFromObj(const struct cl_operand &op) {
    const TObjId obj = this->heapObjFromOperand(op);
    switch (obj) {
        case OBJ_INVALID:
            SE_TRAP;
            return VAL_INVALID;

        case OBJ_UNKNOWN:
            return heap_.valCreateUnknown(UV_UNKNOWN, op.type);

        case OBJ_DELETED:
        case OBJ_DEREF_FAILED:
        case OBJ_LOST:
            return VAL_DEREF_FAILED;

        case OBJ_RETURN:
        default:
            break;
    }

    // seek the last accessor
    const struct cl_accessor *ac = op.accessor;
    while (ac && ac->next)
        ac = ac->next;

    // handle CL_ACCESSOR_REF if any
    return (ac && ac->code == CL_ACCESSOR_REF)
        ? heap_.placedAt(obj)
        : heap_.valueOf(obj);
}

TValueId SymProc::heapValFromOperand(const struct cl_operand &op) {
    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
        case CL_OPERAND_REG:
            return this->heapValFromObj(op);

        case CL_OPERAND_CST:
            return this->heapValFromCst(op);

        default:
#if SE_SELF_TEST
            SE_TRAP;
#endif
            return VAL_INVALID;
    }
}

int /* uid */ SymProc::fncFromOperand(const struct cl_operand &op) {
    if (CL_OPERAND_CST == op.code) {
        // direct call
        const struct cl_cst &cst = op.data.cst;
        SE_BREAK_IF(CL_TYPE_FNC != cst.code);

        return cst.data.cst_fnc.uid;

    } else {
        // indirect call
        const TValueId val = this->heapValFromOperand(op);
        if (VAL_INVALID == val)
            // Oops, it does not look as indirect call actually
            SE_TRAP;

        // obtain the inner content of the custom value and check its type-info
        const struct cl_type *clt;
        const int uid = heap_.valGetCustom(&clt, val);
        SE_BREAK_IF(-1 == uid);

#if SE_SELF_TEST
        // check for a pointer
        SE_BREAK_IF(!clt || clt->code != CL_TYPE_PTR);

        // check for a function
        clt = clt->items[0].type;
        SE_BREAK_IF(!clt || clt->code != CL_TYPE_FNC);
#endif
        return uid;
    }
}

void SymProc::heapObjDefineType(TObjId lhs, TValueId rhs) {
    const TObjId var = heap_.pointsTo(rhs);
    SE_BREAK_IF(OBJ_INVALID == var);

    const struct cl_type *clt = heap_.objType(lhs);
    if (!clt)
        return;

    // move to next clt
    // --> what are we pointing to actually?
    clt = targetTypeOfPtr(clt);
    if (CL_TYPE_VOID == clt->code)
        return;

    // anonymous objects of zero size are not allowed
    const int cbGot = heap_.objSizeOfAnon(var);
    SE_BREAK_IF(!cbGot);

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

void SymProc::heapSetSingleVal(TObjId lhs, TValueId rhs) {
    // save the old value, which is going to be overwritten
    const TValueId oldValue = heap_.valueOf(lhs);
    SE_BREAK_IF(VAL_INVALID == oldValue);

    if (0 < rhs) {
        const TObjId target = heap_.pointsTo(rhs);
        if (0 < target && !heap_.objType(target))
            // anonymous object is going to be specified by a type
            this->heapObjDefineType(lhs, rhs);
    }

    heap_.objSetValue(lhs, rhs);
    if (collectJunk(heap_, oldValue, lw_))
        bt_->printBackTrace();
}

struct SpecialValueWriter {
    TValueId valToWrite;

    bool operator()(SymHeap &sh, TObjId sub) {
        sh.objSetValue(sub, valToWrite);
        return /* continue */ true;
    }
};

void SymProc::objSetValue(TObjId lhs, TValueId rhs) {
    // FIXME: handle some other special values also this way?
    if (VAL_DEREF_FAILED == rhs) {
        // we're already on an error path
        const struct cl_type *clt = heap_.objType(lhs);
        if (!clt || clt->code != CL_TYPE_STRUCT) {
            heap_.objSetValue(lhs, rhs);
            return;
        }

        // fill values of all sub-objects by 'rhs'
        SpecialValueWriter writer;
        writer.valToWrite = rhs;
        traverseSubObjs(heap_, lhs, writer, /* leavesOnly */ true);
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

        // check type-info
        const struct cl_type *clt = heap_.objType(rObj);
        SE_BREAK_IF(!clt || clt->code != CL_TYPE_STRUCT || clt != heap_.objType(lhs));

        // iterate through all fields
        for (int i = 0; i < clt->item_cnt; ++i) {
            const TObjId lSub = heap_.subObj(lhs, i);
            const TObjId rSub = heap_.subObj(rObj, i);
            SE_BREAK_IF(lSub <= 0 || rSub <= 0);

            // schedule sub for next wheel
            const TValueId rSubVal = heap_.valueOf(rSub);
            push(todo, lSub, rSubVal);
        }
    }
}

void SymProc::objDestroy(TObjId obj) {
    // gather potentialy destroyed pointer sub-values
    std::vector<TValueId> ptrs;
    getPtrValues(ptrs, heap_, obj);

    // destroy object recursively
    heap_.objDestroy(obj);

    // now check for JUNK
    bool junk = false;
    BOOST_FOREACH(TValueId val, ptrs) {
        if (collectJunk(heap_, val, lw_))
            junk = true;
    }

    if (junk)
        // print backtrace at most once per one call of objDestroy()
        bt_->printBackTrace();
}

void SymProc::execFreeCore(const TValueId val) {
    const EUnknownValue code = heap_.valGetUnknown(val);
    switch (code) {
        case UV_ABSTRACT:
            SE_TRAP;
            // fall through!

        case UV_KNOWN:
            break;

        case UV_UNKNOWN:
            CL_ERROR_MSG(lw_, "free() called on unknown value");
            bt_->printBackTrace();
            return;

        case UV_UNINITIALIZED:
            CL_ERROR_MSG(lw_, "free() called on uninitialized value");
            bt_->printBackTrace();
            return;
    }

    const TObjId obj = heap_.pointsTo(val);
    switch (obj) {
        case OBJ_DELETED:
            CL_ERROR_MSG(lw_, "double free() detected");
            bt_->printBackTrace();
            return;

        case OBJ_LOST:
            // this is a double error in the analyzed program :-)
            CL_ERROR_MSG(lw_, "attempt to free a non-heap object"
                              ", which does not exist anyhow");
            bt_->printBackTrace();
            return;

        case OBJ_UNKNOWN:
        case OBJ_INVALID:
            SE_TRAP;

        default:
            break;
    }

    CVar cVar;
    if (heap_.cVar(&cVar, obj)) {
        CL_DEBUG("about to free var #" << cVar.uid);
        CL_ERROR_MSG(lw_, "attempt to free a non-heap object");
        bt_->printBackTrace();
        return;
    }

    if (OBJ_INVALID != heap_.objParent(obj)) {
        CL_ERROR_MSG(lw_, "attempt to free a non-root object");
        bt_->printBackTrace();
        return;
    }

    CL_DEBUG_MSG(lw_, "executing free()");
    this->objDestroy(obj);
}

void SymProc::seekAliasedRoot(TValueId *pValue) {
    const TValueId val = *pValue;
    const TObjId obj = heap_.pointsTo(val);
    if (obj < 0)
        // no valid target anyway
        return;

    // seek the root
    const TObjId root = objRoot(heap_, obj);
    SE_BREAK_IF(root < 0);
    if (obj == root)
        // we've already got the root object
        return;

    // get root's address
    const TValueId valRoot = heap_.placedAt(root);
    SE_BREAK_IF(valRoot <= 0);

    // finally check if the given value is alias of the root or not
    bool eq;
    if (!heap_.proveEq(&eq, val, valRoot) && !eq)
        return;

    // found!
    CL_DEBUG_MSG(lw_, "alias of root matched during execution of free()");
    *pValue = valRoot;
}

void SymProc::execFree(const CodeStorage::TOperandList &opList) {
    SE_BREAK_IF(/* dst + fnc + ptr */ 3 != opList.size());

    // free() does not usually return a value
    SE_BREAK_IF(CL_OPERAND_VOID != opList[0].code);

    // resolve value to be freed
    TValueId val = heapValFromOperand(opList[/* ptr given to free() */2]);
    SE_BREAK_IF(VAL_INVALID == val);

    switch (val) {
        case VAL_NULL:
            CL_DEBUG_MSG(lw_, "ignoring free() called with NULL value");
            return;

        case VAL_DEREF_FAILED:
            return;

        default:
            break;
    }

    this->seekAliasedRoot(&val);
    this->execFreeCore(val);
}

void SymProc::execMalloc(TState &state, const CodeStorage::TOperandList &opList,
                         bool fastMode)
{
    SE_BREAK_IF(/* dst + fnc + size */ 3 != opList.size());

    // resolve lhs
    const struct cl_operand &dst = opList[0];
    const TObjId varLhs = this->heapObjFromOperand(dst);
    SE_BREAK_IF(OBJ_INVALID == varLhs);

    // amount of allocated memory must be a constant
    const struct cl_operand &amount = opList[2];
    SE_BREAK_IF(CL_OPERAND_CST != amount.code);

    // amount of allocated memory must be a number
    const struct cl_cst &cst = amount.data.cst;
    SE_BREAK_IF(CL_TYPE_INT != cst.code);
    const int cbAmount = cst.data.cst_int.value;
    CL_DEBUG_MSG(lw_, "executing malloc(" << cbAmount << ")");

    // now create a heap object
    const TObjId obj = heap_.objCreateAnon(cbAmount);
    SE_BREAK_IF(OBJ_INVALID == obj);

    const TValueId val = heap_.placedAt(obj);
    SE_BREAK_IF(val <= 0);

    if (!fastMode) {
        // OOM state simulation
        this->objSetValue(varLhs, VAL_NULL);
        state.insert(heap_);
    }

    // store the result of malloc
    this->objSetValue(varLhs, val);
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

    template <int NTH, class TOpList, class THeap, class TBt>
    bool readHeapVal(TValueId *dst, const TOpList opList, const THeap &heap,
                     TBt *bt)
    {
        // FIXME: we might use the already existing instance instead
        SymProc proc(const_cast<THeap &>(heap), bt);

        const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
        const TValueId value = proc.heapValFromOperand(op);
        if (value < 0)
            return false;

        *dst = value;
        return true;
    }

    template <class TInsn, class THeap, class TBt>
    bool readNameAndValue(std::string *pName, TValueId *pValue,
                          const TInsn &insn, const THeap &heap, TBt bt)
    {
        const CodeStorage::TOperandList &opList = insn.operands;
        const LocationWriter lw(&insn.loc);

        if (!chkVoidCall<2>(opList))
            return false;

        if (!readHeapVal<0>(pValue, opList, heap, bt))
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
        SymPlot plotter(stor, heap);
        if (!plotter.plot(plotName))
            emitPlotError(lw, plotName);

        return true;
    }

    template <class TInsn, class THeap, class TBt>
    bool callPlotByPtr(const TInsn &insn, const THeap &heap, TBt *bt) {
        const LocationWriter lw(&insn.loc);

        std::string plotName;
        TValueId value;
        if (!readNameAndValue(&plotName, &value, insn, heap, bt)) {
            emitPrototypeError(lw, "___sl_plot_by_ptr");
            return false;
        }

        const CodeStorage::Storage &stor = *insn.stor;
        SymPlot plotter(stor, heap);
        if (!plotter.plotHeapValue(plotName, value))
            emitPlotError(lw, plotName);

        return true;
    }

    template <class TInsn, class THeap, class TBt>
    bool callPlotStackFrame(const TInsn &insn, const THeap &heap, TBt *bt) {
        const CodeStorage::Storage &stor = *insn.stor;
        const LocationWriter lw(&insn.loc);

        std::string plotName;
        TValueId value;
        const CodeStorage::Fnc *fnc;

        if (!readNameAndValue(&plotName, &value, insn, heap, bt)
                || !fncFromHeapVal(stor, &fnc, value, heap))
        {
            emitPrototypeError(lw, "___sl_plot_stack_frame");
            return false;
        }

        SymPlot plotter(stor, heap);
        if (!plotter.plotStackFrame(plotName, *fnc, bt))
            emitPlotError(lw, plotName);

        return true;
    }
}

// TODO: move all the implementations of built-ins to a separate module and
//       create some reasonable interface for adding new ones
bool SymProc::execCall(TState &dst, const CodeStorage::Insn &insn,
                       SymProcExecParams ep)
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
        this->execMalloc(dst, opList, ep.fastMode);
        return true;
    }

    if (STREQ(fncName, "free")) {
        this->execFree(opList);
        goto call_done;
    }

    if (STREQ(fncName, "abort")) {
        SE_BREAK_IF(opList.size() != 2 || opList[0].code != CL_OPERAND_VOID);

        // do nothing for abort()
        goto call_done;
    }

    if (STREQ(fncName, "___sl_plot")) {
        if (ep.skipPlot)
            CL_DEBUG_MSG(lw_, "___sl_plot skipped per user's request");

        else if (!callPlot(insn, heap_))
            // invalid prototype etc.
            return false;

        goto call_done;
    }

    if (STREQ(fncName, "___sl_plot_stack_frame")) {
        if (ep.skipPlot)
            CL_DEBUG_MSG(lw_,
                    "___sl_plot_stack_frame skipped per user's request");

        else if (!callPlotStackFrame(insn, heap_, bt_))
            // invalid prototype etc.
            return false;

        goto call_done;
    }

    if (STREQ(fncName, "___sl_plot_by_ptr")) {
        if (ep.skipPlot)
            CL_DEBUG_MSG(lw_, "___sl_plot_by_ptr skipped per user's request");

        else if (!callPlotByPtr(insn, heap_, bt_))
            // invalid prototype etc.
            return false;

        goto call_done;
    }

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
    // check type validity wrt. CL_UNOP_TRUTH_NOT
    SE_BREAK_IF(!clt || clt->code != CL_TYPE_BOOL);

    if (handleUnopTruthNotTrivial(val))
        // we are done
        return;

    // the value we got msut be VAL_TRUE, VAL_FALSE, or an unknown value
#if SE_SELF_TEST
    const EUnknownValue code = heap.valGetUnknown(val);
    SE_BREAK_IF(UV_KNOWN == code || UV_ABSTRACT == code);
#else
    (void) clt;
#endif

    const TValueId origValue = val;
    val = heap.valDuplicateUnknown(origValue);

    // FIXME: not tested
#if SE_SELF_TEST
    SE_TRAP;
#endif
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
            SE_TRAP;
            return VAL_INVALID;
    }
    if (v1 == valElim)
        return v2;
    if (v2 == valElim)
        return v1;

    SE_BREAK_IF(v1 < 0 || v2 < 0);

    // FIXME: not tested
#if SE_SELF_TEST
    SE_TRAP;
#endif
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
    SE_BREAK_IF(VAL_INVALID == v1 || VAL_INVALID == v2);

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
            SE_TRAP;
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
    if (VAL_DEREF_FAILED == v1 || VAL_DEREF_FAILED == v2)
        return VAL_DEREF_FAILED;

    SE_BREAK_IF(v1 < 0 || v2 < 0);

    switch (code) {
        case CL_BINOP_EQ:
        case CL_BINOP_NE:
            break;

        default:
            // crazy comparison of pointer values
            SE_TRAP;
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
            SE_TRAP;
            return VAL_INVALID;
    }
}

struct SubByOffsetFinder {
    TObjId                  root;
    TObjId                  subFound;
    const struct cl_type    *cltToSeek;
    int                     offToSeek;

    bool operator()(const SymHeap &sh, TObjId sub) {
        const struct cl_type *clt = sh.objType(sub);
        if (!clt || *clt != *this->cltToSeek)
            return /* continue */ true;

        if (this->offToSeek != subOffsetIn(sh, this->root, sub))
            return /* continue */ true;

        // found!
        this->subFound = sub;
        return /* break */ false;
    }
};

TObjId subSeekByOffset(const SymHeap &sh, TObjId obj,
                       const struct cl_type *clt, int offToSeek)
{
    if (!offToSeek)
        return obj;

    // prepare visitor
    SubByOffsetFinder visitor;
    visitor.root        = obj;
    visitor.cltToSeek   = clt;
    visitor.offToSeek   = offToSeek;
#if !SE_SELF_TEST
    visitor.subFound    = OBJ_INVALID;
#endif

    // look for the requested sub-object
    if (traverseSubObjs(sh, obj, visitor, /* leavesOnly */ false))
        return OBJ_INVALID;
    else
        return visitor.subFound;
}

TValueId handlePointerPlus(SymHeap &sh, const struct cl_type *clt,
                           TValueId ptr, const struct cl_operand &op)
{
    // jump to _target_ type
    SE_BREAK_IF(!clt || clt->code != CL_TYPE_PTR);
    clt = clt->items[0].type;

    // read integral offset
    SE_BREAK_IF(CL_OPERAND_CST != op.code);
    const struct cl_cst &cst = op.data.cst;
    SE_BREAK_IF(CL_TYPE_INT != op.type->code);
    int off = cst.data.cst_int.value;
    CL_DEBUG("handlePointerPlus(): " << off << "b offset requested");

    // seek root object while cumulating the offset
    TObjId obj = sh.pointsTo(ptr);
    TObjId parent;
    int nth;
    while (OBJ_INVALID != (parent = sh.objParent(obj, &nth))) {
        const struct cl_type *cltParent = sh.objType(parent);
        SE_BREAK_IF(cltParent->item_cnt <= nth);

        off += cltParent->items[nth].offset;
        obj = parent;
    }

    if (off < 0) {
        // we need to create an off-value
        const SymHeapCore::TOffVal ov(sh.placedAt(obj), off);
        return sh.valCreateByOffset(ov);
    }

    obj = subSeekByOffset(sh, obj, clt, off);
    if (obj <= 0)
        // TODO: create an unknown value?
        SE_TRAP;

    // get the final address and check type compatibility
    const TValueId addr = sh.placedAt(obj);
#if SE_SELF_TEST
    const struct cl_type *cltDst = sh.valType(addr);
    SE_BREAK_IF(!cltDst || *cltDst != *clt);
#endif

    return addr;
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
                SE_TRAP;
        }

        return val;
    }
};

// binary operator handler
template <class TProc>
struct OpHandler</* binary */ 2, TProc> {
    static TValueId handleOp(TProc &proc, int iCode, const TValueId rhs[2],
                             const struct cl_type *clt[2 + /* dst type */ 1])
    {
        const struct cl_type *const cltA = clt[0];
        const struct cl_type *const cltB = clt[1];
        SE_BREAK_IF(!cltA || !cltB);

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
                SE_TRAP;
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
void SymProc::execOp(const CodeStorage::Insn &insn) {
    // resolve lhs
    TObjId varLhs = OBJ_INVALID;
    const struct cl_operand &dst = insn.operands[/* dst */ 0];
    if (!this->lhsFromOperand(&varLhs, dst))
        return;

    // store cl_type of dst operand
    const struct cl_type *clt[ARITY + /* dst type */ 1];
    clt[/* dst type */ ARITY] = dst.type;

    // gather rhs values (and type-info)
    const CodeStorage::TOperandList &opList = insn.operands;
    TValueId rhs[ARITY];
    for (int i = 0; i < ARITY; ++i) {
        const struct cl_operand &op = opList[i + /* [+dst] */ 1];
        clt[i] = op.type;
        rhs[i] = this->heapValFromOperand(op);
        SE_BREAK_IF(VAL_INVALID == rhs[i]);
    }

    TValueId valResult = VAL_INVALID;
    if (2 == ARITY && CL_BINOP_POINTER_PLUS
            == static_cast<enum cl_binop_e>(insn.subCode))
    {
        // handle pointer plus
        valResult = handlePointerPlus(heap_, clt[/* dst type */ ARITY],
                                      rhs[0], opList[/* src2 */ 2]);
    }
    else
        // handle generic operator
        valResult = handleOp<ARITY>(*this, insn.subCode, rhs, clt);

    // store the result
    this->objSetValue(varLhs, valResult);
}

bool SymProc::concretizeLoop(TState &dst, const CodeStorage::Insn &insn,
                             const struct cl_operand &op)
{
    bool hit = false;

    TSymHeapList todo;
    todo.push_back(heap_);
    while (!todo.empty()) {
        SymHeap &sh = todo.front();
        SymProc proc(sh, bt_);
        proc.setLocation(lw_);

        // we expect a pointer at this point
        const TObjId ptr = varFromOperand(op, sh, bt_);
        const TValueId val = sh.valueOf(ptr);
        if (0 < val && UV_ABSTRACT == sh.valGetUnknown(val)) {
            hit = true;
            concretizeObj(sh, val, todo);
        }

        // process the current heap and move to the next one (if any)
        proc.execCore(dst, insn, SymProcExecParams());
        todo.pop_front();
    }

    return hit;
}

namespace {
bool checkForDeref(const struct cl_operand &op, const CodeStorage::Insn &insn) {
    const struct cl_accessor *ac = op.accessor;
    if (!ac || CL_ACCESSOR_DEREF != ac->code)
        // we expect the dereference only as the first accessor
        return false;

#if SE_SELF_TEST
    const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn.subCode);
    SE_BREAK_IF(CL_INSN_UNOP != insn.code || CL_UNOP_ASSIGN != code);
#else
    (void) insn;
#endif

    // we should go through concretization
    return true;
}
} // namespace

bool SymProc::concretizeIfNeeded(TState &results, const CodeStorage::Insn &insn)
{
    const size_t opCnt = insn.operands.size();
    if (opCnt != /* deref */ 2 && opCnt != /* free() */ 3)
        // neither dereference, nor free()
        return false;

    bool hitDeref = false;
    bool hitConcretize = false;
    BOOST_FOREACH(const struct cl_operand &op, insn.operands) {
        if (!checkForDeref(op, insn))
            continue;

        hitDeref = true;

        if (hitConcretize)
            // FIXME: are we ready for two dereferences within one insn?
            SE_TRAP;

        hitConcretize = this->concretizeLoop(results, insn, op);
    }
    if (hitDeref)
        return true;

    const enum cl_insn_e code = insn.code;
    const struct cl_operand &src = insn.operands[/* src */ 1];
    if (CL_INSN_CALL != code || CL_OPERAND_CST != src.code)
        return false;

    const struct cl_cst &cst = src.data.cst;
    if (CL_TYPE_FNC != cst.code || !STREQ(cst.data.cst_fnc.name, "free"))
        return false;

    // assume call of free()
    this->concretizeLoop(results, insn, insn.operands[/* addr */ 2]);
    return true;
}

bool SymProc::execCore(TState &dst, const CodeStorage::Insn &insn,
                       SymProcExecParams ep)
{
    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execOp<1>(insn);
            break;

        case CL_INSN_BINOP:
            this->execOp<2>(insn);
            break;

        case CL_INSN_CALL:
            return this->execCall(dst, insn, ep);

        default:
            SE_TRAP;
            return false;
    }

    dst.insert(heap_);
    return true;
}

bool SymProc::exec(TState &dst, const CodeStorage::Insn &insn,
                   SymProcExecParams ep)
{
    lw_ = &insn.loc;
    if (this->concretizeIfNeeded(dst, insn))
        // concretization loop done
        return true;

    return this->execCore(dst, insn, ep);
}
