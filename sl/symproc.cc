/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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
#include <cl/clutil.hh>

#include "symabstract.hh"
#include "symbin.hh"
#include "symbt.hh"
#include "symgc.hh"
#include "symheap.hh"
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

        case CL_TYPE_ENUM:
        case CL_TYPE_INT:
        case CL_TYPE_PTR:
            break;

        default:
            CL_TRAP;
    }

    const struct cl_cst &cst = op.data.cst;
    code = cst.code;
    switch (code) {
        case CL_TYPE_ENUM:
            // we don't have any handling for enums, let's treat it as int...

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

        case CL_TYPE_STRING:
            CL_WARN_MSG(lw_, "CL_TYPE_STRING not supported by SymProc");
            // fall through!

        default:
            return heap_.valCreateUnknown(UV_UNKNOWN, op.type);
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
            CL_TRAP;

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
#ifndef NDEBUG
            CL_TRAP;
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

    // Although this may be a regular error in the analysed program, we don't
    // have a meaningful error message for that yet.  It should be safe to
    // ignore the failure in a production build, as it will be caught later.
    CL_BREAK_IF(OBJ_INVALID == *pObj);
}

void SymProc::heapObjHandleAccessor(TObjId *pObj,
                                    const struct cl_accessor *ac)
{
    const enum cl_accessor_e code = ac->code;
    switch (code) {
        case CL_ACCESSOR_DEREF:
            // this should have been handled elsewhere
            CL_TRAP;
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
    const int uid = varIdFromOperand(&op);
    const int nestLevel = bt->countOccurrencesOfTopFnc();
    const CVar cVar(uid, nestLevel);
    return sh.objByCVar(cVar);
}

} // namespace

void SymProc::resolveAliasing(TValueId *pVal, const struct cl_type *cltTarget,
                              bool virtualDefeference)
{
    TValueId val = *pVal;
    const struct cl_type *clt = heap_.valType(val);
    if (!clt)
        // no type-info, giving up...
        return;

    if (*clt == *cltTarget)
        // no aliasing, all seems OK
        return;

    if (CL_TYPE_PTR == clt->code && CL_TYPE_PTR == cltTarget->code
            && CL_TYPE_FNC != targetTypeOfPtr(clt)->code
            && CL_TYPE_FNC != targetTypeOfPtr(cltTarget)->code)
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
#ifdef NDEBUG
        break;
#endif
    }

    if (!cntMatch) {
        if (!virtualDefeference) {
            CL_ERROR_MSG(lw_,
                    "type of the pointer being dereferenced does not match "
                    "type of the target object");

            bt_->printBackTrace();
        }

        return;
    }

    // ensure we've got a _deterministic_ match
    CL_BREAK_IF(1 < cntMatch);

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

void SymProc::handleDeref(TObjId *pObj, const struct cl_accessor **pAc) {
    // mark the current accessor as done in advance, and move to the next one
    const struct cl_accessor *ac = *pAc;
    *pAc = ac->next;

    const TObjId obj = *pObj;
    if (obj < 0)
        // we're already on an error path
        return;

    // derive the target type of the pointer we're dereferencing
    const struct cl_type *clt = heap_.objType(obj);
    const struct cl_type *cltTarget = targetTypeOfPtr(clt);
    CL_BREAK_IF(!clt || !cltTarget || cltTarget->code == CL_TYPE_VOID);

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
            CL_TRAP;
        default:
            break;
    }

    // We need to introduce a less chatty variant of dereference for accessor
    // chains of the form:
    //
    //      CL_ACCESSOR_DEREF -> ... -> CL_ACCESSOR_REF
    //
    // ... as we should not claim there is an invalid dereference, if only
    // an address is actually being computed
    const bool virtualDefeference = seekRefAccessor(ac);

    // attempt to resolve aliasing
    this->resolveAliasing(&val, cltTarget, virtualDefeference);

    // attempt to resolve an off-value
    this->resolveOffValue(&val, pAc);

    // now perform the actual dereference
    *pObj = this->handleDerefCore(val);
}

TObjId SymProc::heapObjFromOperand(const struct cl_operand &op) {
    // resolve static variable
    TObjId obj = varFromOperand(op, heap_, bt_);
    CL_BREAK_IF(OBJ_INVALID == obj);

    const struct cl_accessor *ac = op.accessor;
    if (!ac)
        // no accessors, we're done
        return obj;

    // first check for dereference and handle any off-value eventually
    if (ac->code == CL_ACCESSOR_DEREF)
        this->handleDeref(&obj, &ac);

    // we don't support chaining of CL_ACCESSOR_DEREF (yet?)
    CL_BREAK_IF(ac && ac->code == CL_ACCESSOR_DEREF);

    while (ac) {
        this->heapObjHandleAccessor(&obj, ac);
        ac = ac->next;
    }

    return obj;
}

TValueId SymProc::heapValFromObj(const struct cl_operand &op) {
    const TObjId obj = this->heapObjFromOperand(op);
    switch (obj) {
        case OBJ_INVALID:
            CL_TRAP;
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

    // handle CL_ACCESSOR_REF if any
    return (seekRefAccessor(op.accessor))
        ? heap_.placedAt(obj)
        : heap_.valueOf(obj);
}

TValueId SymProc::heapValFromOperand(const struct cl_operand &op) {
    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
            return this->heapValFromObj(op);

        case CL_OPERAND_CST:
            return this->heapValFromCst(op);

        default:
#ifndef NDEBUG
            CL_TRAP;
#endif
            return VAL_INVALID;
    }
}

int /* uid */ SymProc::fncFromOperand(const struct cl_operand &op) {
    if (CL_OPERAND_CST == op.code) {
        // direct call
        const struct cl_cst &cst = op.data.cst;
        CL_BREAK_IF(CL_TYPE_FNC != cst.code);

        return cst.data.cst_fnc.uid;

    } else {
        // indirect call
        const TValueId val = this->heapValFromOperand(op);
        if (VAL_INVALID == val)
            // Oops, it does not look as indirect call actually
            CL_TRAP;

        // obtain the inner content of the custom value and check its type-info
        const struct cl_type *clt;
        const int uid = heap_.valGetCustom(&clt, val);
        CL_BREAK_IF(-1 != uid && CL_TYPE_FNC != targetTypeOfPtr(clt)->code);
        return uid;
    }
}

void SymProc::heapObjDefineType(TObjId lhs, TValueId rhs) {
    const TObjId var = heap_.pointsTo(rhs);
    CL_BREAK_IF(OBJ_INVALID == var);

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
    CL_BREAK_IF(!cbGot);

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
    CL_BREAK_IF(VAL_INVALID == oldValue);

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

class ValueWriter {
    private:
        SymProc             &proc_;
        const TValueId      valToWrite_;

    public:
        ValueWriter(SymProc *proc, TValueId valToWrite):
            proc_(*proc),
            valToWrite_(valToWrite)
        {
        }

        bool operator()(SymHeap &, TObjId sub) {
            proc_.heapSetSingleVal(sub, valToWrite_);
            return /* continue */ true;
        }
};

class UnionInvalidator {
    private:
        SymProc             &proc_;
        const TObjId        preserverSubtree_;

    public:
        UnionInvalidator(SymProc *proc, TObjId preserverSubtree):
            proc_(*proc),
            preserverSubtree_(preserverSubtree)
        {
        }

        bool operator()(SymHeap &sh, TObjId sub) {
            // first ensure we don't overwrite something we don't want to
            TObjId obj = sub;
            do {
                if (preserverSubtree_ == obj)
                    return /* continue */ true;
            }
            while (OBJ_INVALID != (obj = sh.objParent(obj)));

            const TValueId val = sh.valCreateUnknown(UV_UNKNOWN, /* clt */ 0);
            proc_.heapSetSingleVal(sub, val);
            return /* continue */ true;
        }
};

class ValueMirror {
    private:
        SymProc &proc_;

    public:
        ValueMirror(SymProc *proc): proc_(*proc) { }

        bool operator()(SymHeap &sh, const TObjPair &item) const {
            const TObjId lhs = item.first;
            const TValueId rhs = sh.valueOf(item.second);
            proc_.heapSetSingleVal(lhs, rhs);

            return /* continue */ true;
        }
};

void SymProc::objSetValue(TObjId lhs, TValueId rhs) {
    // seek all surrounding unions on the way to root (if any)
    TObjId parent, obj = lhs;
    for (; OBJ_INVALID != (parent = heap_.objParent(obj)); obj = parent) {
        const struct cl_type *clt = heap_.objType(parent);
        if (!clt || clt->code != CL_TYPE_UNION)
            continue;

        // invalidate all siblings within the surrounding union
        UnionInvalidator visitor(this, obj);
        traverseSubObjs(heap_, parent, visitor, /* leavesOnly */ true);
    }

    // FIXME: handle some other special values also this way?
    if (VAL_DEREF_FAILED == rhs) {
        // we're already on an error path
        const struct cl_type *clt = heap_.objType(lhs);
        if (!clt || clt->code != CL_TYPE_STRUCT) {
            heap_.objSetValue(lhs, rhs);
            return;
        }

        // fill values of all sub-objects by 'rhs'
        ValueWriter writer(this, rhs);
        traverseSubObjs(heap_, lhs, writer, /* leavesOnly */ true);
        return;
    }

    // FIXME: this doesn't work well if we get UV_UNKNOWN of type CL_TYPE_STRUCT
    const TObjId rObj = heap_.valGetCompositeObj(rhs);
    if (OBJ_INVALID == rObj) {
        // non-composite value
        this->heapSetSingleVal(lhs, rhs);
        return;
    }

    // DFS for composite types
    const TObjPair item(lhs, rObj);
    const ValueMirror mirror(this);
    traverseSubObjs(heap_, item, mirror, /* leavesOnly */ true);
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

// /////////////////////////////////////////////////////////////////////////////
// SymExecCore implementation
TValueId SymExecCore::heapValFromOperand(const struct cl_operand &op) {
    const char *name;
    if (!ep_.invCompatMode)
        goto no_nasty_assumptions;

    if (CL_OPERAND_VAR != op.code || !(name = op.data.var->name))
        goto no_nasty_assumptions;
    
    if (STREQ(name, "nondet") || STREQ(name, "__nondet")) {
        CL_WARN_MSG(lw_, "value of '" << name << "' treated as unknown, "
                         "using a nasty assumption!");

        // &nondet and __nodet should pass this check (ugly)
        CL_BREAK_IF(op.accessor && op.accessor->code != CL_ACCESSOR_REF);

        return heap_.valCreateUnknown(UV_UNKNOWN,
                                      /* not always correct */ op.type);
    }

no_nasty_assumptions:
    return SymProc::heapValFromOperand(op);
}

bool SymExecCore::lhsFromOperand(TObjId *pObj, const struct cl_operand &op) {
    *pObj = this->heapObjFromOperand(op);
    switch (*pObj) {
        case OBJ_UNKNOWN:
            CL_ERROR_MSG(lw_, "attempt to use an unknown value as l-value");
            bt_->printBackTrace();
            // fall through!

        case OBJ_DEREF_FAILED:
            return false;

        case OBJ_LOST:
        case OBJ_DELETED:
        case OBJ_INVALID:
            CL_TRAP;

        default:
            return true;
    }
}

void SymExecCore::execFreeCore(const TValueId val) {
    const EUnknownValue code = heap_.valGetUnknown(val);
    switch (code) {
        case UV_ABSTRACT:
            CL_TRAP;
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
            CL_TRAP;

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

void SymExecCore::seekAliasedRoot(TValueId *pValue) {
    const TValueId val = *pValue;
    const TObjId obj = heap_.pointsTo(val);
    if (obj < 0)
        // no valid target anyway
        return;

    // seek the root
    const TObjId root = objRoot(heap_, obj);
    CL_BREAK_IF(root < 0);
    if (obj == root)
        // we've already got the root object
        return;

    // get root's address
    const TValueId valRoot = heap_.placedAt(root);
    CL_BREAK_IF(valRoot <= 0);

    // finally check if the given value is alias of the root or not
    bool eq;
    if (!heap_.proveEq(&eq, val, valRoot) || !eq)
        return;

    // found!
    CL_DEBUG_MSG(lw_, "alias of root matched during execution of free()");
    *pValue = valRoot;
}

void SymExecCore::execFree(const CodeStorage::TOperandList &opList) {
    CL_BREAK_IF(/* dst + fnc + ptr */ 3 != opList.size());

    // free() does not usually return a value
    CL_BREAK_IF(CL_OPERAND_VOID != opList[0].code);

    // resolve value to be freed
    TValueId val = heapValFromOperand(opList[/* ptr given to free() */2]);
    CL_BREAK_IF(VAL_INVALID == val);

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

void SymExecCore::execMalloc(SymState                           &state,
                             const CodeStorage::TOperandList    &opList)
{
    CL_BREAK_IF(/* dst + fnc + size */ 3 != opList.size());

    // resolve lhs
    const struct cl_operand &dst = opList[0];
    const TObjId varLhs = this->heapObjFromOperand(dst);
    CL_BREAK_IF(OBJ_INVALID == varLhs);

    // amount of allocated memory must be a constant
    const struct cl_operand &amount = opList[2];
    const int cbAmount = intCstFromOperand(&amount);
    CL_DEBUG_MSG(lw_, "executing malloc(" << cbAmount << ")");

    // now create a heap object
    const TObjId obj = heap_.objCreateAnon(cbAmount);
    CL_BREAK_IF(OBJ_INVALID == obj);

    const TValueId val = heap_.placedAt(obj);
    CL_BREAK_IF(val <= 0);

    if (!ep_.fastMode) {
        // OOM state simulation
        this->objSetValue(varLhs, VAL_NULL);
        state.insert(heap_);
    }

    // store the result of malloc
    this->objSetValue(varLhs, val);
    state.insert(heap_);
}

bool SymExecCore::execCall(SymState &dst, const CodeStorage::Insn &insn) {
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

    return handleBuiltIn(dst, *this, insn);
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
            CL_TRAP;
            return VAL_INVALID;
    }
    if (v1 == valElim)
        return v2;
    if (v2 == valElim)
        return v1;
    if (v1 == v2 && (v1 == VAL_TRUE || v1 == VAL_FALSE))
        return valElim;

    CL_BREAK_IF(v1 < 0 || v2 < 0);

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
    CL_BREAK_IF(VAL_INVALID == v1 || VAL_INVALID == v2);

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
            CL_TRAP;
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

    CL_BREAK_IF(v1 < 0 || v2 < 0);

    switch (code) {
        case CL_BINOP_EQ:
        case CL_BINOP_NE:
            break;

        default:
            // crazy comparison of pointer values
            CL_TRAP;
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
            // unsupported type
            CL_WARN("binary operator not implemented yet");
            return heap.valCreateUnknown(UV_UNKNOWN, dstClt);
    }
}

TValueId handlePointerPlus(SymHeap &sh, const struct cl_type *cltPtr,
                           TValueId ptr, const struct cl_operand &op,
                           const LocationWriter lw)
{
    if (CL_OPERAND_CST != op.code) {
        CL_ERROR_MSG(lw, "pointer plus offset not known in compile-time");
        return sh.valCreateUnknown(UV_UNKNOWN, cltPtr);
    }

    if (VAL_NULL == ptr) {
        CL_DEBUG_MSG(lw, "pointer plus with NULL treated as unknown value");
        return sh.valCreateUnknown(UV_UNKNOWN, cltPtr);
    }

    // read integral offset
    const int offRequested = intCstFromOperand(&op);
    CL_DEBUG("handlePointerPlus(): " << offRequested << "b offset requested");

    const TObjId target = sh.pointsTo(ptr);
    CL_BREAK_IF(target <= 0);
    return addrQueryByOffset(sh, target, offRequested, cltPtr, &lw);
}

// template for generic (unary, binary, ...) operator handlers
template <int ARITY>
struct OpHandler {
    static TValueId handleOp(SymProc &proc, int code, const TValueId rhs[ARITY],
                             const struct cl_type *clt[ARITY +/* dst */1]);
};

// unary operator handler
template <>
struct OpHandler</* unary */ 1> {
    static TValueId handleOp(SymProc &proc, int iCode, const TValueId rhs[1],
                             const struct cl_type *clt[1 + /* dst type */ 1])
    {
        SymHeap &sh = proc.heap_;
        const TValueId val = rhs[0];

        const enum cl_unop_e code = static_cast<enum cl_unop_e>(iCode);
        switch (code) {
            case CL_UNOP_TRUTH_NOT:
                return handleOpCmpBool(sh, CL_BINOP_EQ, clt[1], VAL_FALSE, val);

            case CL_UNOP_ASSIGN:
                return val;

            default:
                CL_WARN_MSG(proc.lw_, "unary operator not implemented yet");
                return sh.valCreateUnknown(UV_UNKNOWN, clt[/* dst */ 1]);
        }
    }
};

// binary operator handler
template <>
struct OpHandler</* binary */ 2> {
    static TValueId handleOp(SymProc &proc, int iCode, const TValueId rhs[2],
                             const struct cl_type *clt[2 + /* dst type */ 1])
    {
        CL_BREAK_IF(!clt[0] || !clt[1] || !clt[2]);
        SymHeap &sh = proc.heap_;

        const enum cl_binop_e code = static_cast<enum cl_binop_e>(iCode);
        switch (code) {
            case CL_BINOP_EQ:
            case CL_BINOP_NE:
            case CL_BINOP_LT:
            case CL_BINOP_GT:
            case CL_BINOP_LE:
            case CL_BINOP_GE:
                CL_BREAK_IF(clt[/* src1 */ 0]->code != clt[/* src2 */ 1]->code);
                return handleOpCmp(sh, code, clt[2], clt[0], rhs[0], rhs[1]);

            default:
                CL_WARN_MSG(proc.lw_, "binary operator not implemented yet");
                return sh.valCreateUnknown(UV_UNKNOWN, clt[/* dst */ 2]);
        }
    }
};

template <int ARITY>
void SymExecCore::execOp(const CodeStorage::Insn &insn) {
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

        const TValueId val = this->heapValFromOperand(op);
        CL_BREAK_IF(VAL_INVALID == val);
        if (VAL_DEREF_FAILED == val) {
            // we're already on an error path
            this->objSetValue(varLhs, VAL_DEREF_FAILED);
            return;
        }

        rhs[i] = val;
    }

    TValueId valResult = VAL_INVALID;
    if (2 == ARITY && CL_BINOP_POINTER_PLUS
            == static_cast<enum cl_binop_e>(insn.subCode))
    {
        // handle pointer plus
        valResult = handlePointerPlus(heap_, clt[/* dst type */ ARITY],
                                      rhs[0], opList[/* src2 */ 2], &insn.loc);
    }
    else
        // handle generic operator
        valResult = OpHandler<ARITY>::handleOp(*this, insn.subCode, rhs, clt);

    // store the result
    this->objSetValue(varLhs, valResult);
}

template <class TOpList, class TDerefs>
bool SymExecCore::concretizeLoop(SymState                       &dst,
                                 const CodeStorage::Insn        &insn,
                                 const TOpList                  &opList,
                                 const TDerefs                  &derefs)
{
    bool hit = false;

    TSymHeapList todo;
    todo.push_back(heap_);
    while (!todo.empty()) {
        SymHeap &sh = todo.front();
        SymExecCore core(sh, bt_, ep_);
        core.setLocation(lw_);

        // TODO: implement full version of the alg (complexity m*n)
        bool hitLocal = false;
        BOOST_FOREACH(unsigned idx, derefs) {
            const struct cl_operand &op = opList.at(idx);

            // we expect a pointer at this point
            const TObjId ptr = varFromOperand(op, sh, bt_);
            const TValueId val = sh.valueOf(ptr);
            if (0 < val && UV_ABSTRACT == sh.valGetUnknown(val)) {
                CL_BREAK_IF(hitLocal);
                hit = true;
                hitLocal = true;
                concretizeObj(sh, val, todo);
            }
        }

        // process the current heap and move to the next one (if any)
        core.execCore(dst, insn, /* aggressive optimization */1 == todo.size());
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

#ifndef NDEBUG
    const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn.subCode);
    CL_BREAK_IF(CL_INSN_UNOP != insn.code || CL_UNOP_ASSIGN != code);
#else
    (void) insn;
#endif

    // we should go through concretization
    return true;
}
} // namespace

bool SymExecCore::concretizeIfNeeded(SymState                   &results,
                                     const CodeStorage::Insn    &insn)
{
    const size_t opCnt = insn.operands.size();
    if (opCnt != /* deref */ 2 && opCnt != /* free() */ 3)
        // neither dereference, nor free()
        return false;

    // first look for dereferences
    std::vector<unsigned /* idx */> derefs;
    const CodeStorage::TOperandList &opList = insn.operands;
    for (unsigned idx = 0; idx < opList.size(); ++idx)
        if (checkForDeref(opList[idx], insn))
            derefs.push_back(idx);

    if (!derefs.empty()) {
        // handle dereferences
        this->concretizeLoop(results, insn, opList, derefs);
        return true;
    }

    const enum cl_insn_e code = insn.code;
    const struct cl_operand &src = insn.operands[/* src */ 1];
    if (CL_INSN_CALL != code || CL_OPERAND_CST != src.code)
        return false;

    const struct cl_cst &cst = src.data.cst;
    if (CL_TYPE_FNC != cst.code || !STREQ(cst.data.cst_fnc.name, "free"))
        return false;

    // assume call of free()
    derefs.push_back(/* addr */ 2);
    this->concretizeLoop(results, insn, opList, derefs);
    return true;
}

bool SymExecCore::execCore(
        SymState                &dst,
        const CodeStorage::Insn &insn,
        const bool              feelFreeToOverwrite)
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
            return this->execCall(dst, insn);

        default:
            CL_TRAP;
            return false;
    }

    if (feelFreeToOverwrite)
        // aggressive optimization
        dst.insertFast(heap_);
    else
        dst.insert(heap_);

    return true;
}

bool SymExecCore::exec(SymState &dst, const CodeStorage::Insn &insn) {
    lw_ = &insn.loc;
    if (this->concretizeIfNeeded(dst, insn))
        // concretization loop done
        return true;

    return this->execCore(dst, insn, /* aggressive optimization */ true);
}
