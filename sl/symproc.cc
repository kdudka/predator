/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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
TValId SymProc::heapValFromCst(const struct cl_operand &op) {
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
                TValId val = sh_.valCreate(VT_UNKNOWN, VO_ASSIGNED);
                sh_.neqOp(SymHeap::NEQ_ADD, val, VAL_NULL);
                return val;
            }

        case CL_TYPE_FNC: {
            // wrap fnc uid as SymHeap value
            const int uid = cst.data.cst_fnc.uid;
            return sh_.valCreateCustom(uid);
        }

        case CL_TYPE_STRING:
            CL_WARN_MSG(lw_, "CL_TYPE_STRING not supported by SymProc");
            // fall through!

        default:
            return sh_.valCreate(VT_UNKNOWN, VO_ASSIGNED);
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
            CL_ERROR_MSG(lw_,
                    "type of the pointer being dereferenced does not match "
                    "type of the target object");
            break;

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

bool SymProc::checkForInvalidDeref(TValId val, TObjType cltTarget) {
    switch (val) {
        case VAL_NULL:
            CL_ERROR_MSG(lw_, "dereference of NULL value");
            bt_->printBackTrace();
            // fall through!

        case VAL_DEREF_FAILED:
            return true;

        case VAL_INVALID:
            CL_TRAP;

        default:
            break;
    }

    if (isUninitialized(sh_.valOrigin(val))) {
        CL_ERROR_MSG(lw_, "dereference of uninitialized value");
        bt_->printBackTrace();
        return true;
    }

    const EValueTarget code = sh_.valTarget(val);
    if (!isPossibleToDeref(code) && !isGone(code)) {
        CL_ERROR_MSG(lw_, "dereference of unknown value");
        bt_->printBackTrace();
        return true;
    }

    if (!cltTarget)
        // no type-info, check that there is at least _some_ target
        return this->checkForInvalidDeref(sh_.objAt(val));

    else if (isDataPtr(cltTarget))
        // check if there is _any_ data pointer at that address
        return this->checkForInvalidDeref(sh_.ptrAt(val));

    else
        // mach the type of target object
        return this->checkForInvalidDeref(sh_.objAt(val, cltTarget));
}

TValId SymProc::varAt(const struct cl_operand &op) {
    const int uid = varIdFromOperand(&op);
    const int nestLevel = bt_->countOccurrencesOfTopFnc();
    const CVar cVar(uid, nestLevel);

    const TValId at = sh_.addrOfVar(cVar);
    CL_BREAK_IF(at <= 0);
    return at;
}

TOffset offDerefArray(const struct cl_accessor *ac) {
    // resolve index and type
    const int idx = intCstFromOperand(ac->data.array.index);
    const TObjType clt = targetTypeOfArray(ac->type);

    // compute the offset
    const TOffset off = idx * clt->size;
    if (off)
        CL_BREAK_IF("not tested yet");

    return off;
}

TOffset offItem(const struct cl_accessor *ac) {
    const int id = ac->data.item.id;
    const TObjType clt = ac->type;
    CL_BREAK_IF(!clt || clt->item_cnt <= id);

    return clt->items[id].offset;
}

TValId SymProc::targetAt(const struct cl_operand &op) {
    // resolve program variable
    TValId addr = this->varAt(op);
    const struct cl_accessor *ac = op.accessor;
    if (!ac)
        // no accessors, we're done
        return addr;

    // check for dereference first
    const bool isDeref = (CL_ACCESSOR_DEREF == ac->code);
    if (isDeref) {
        // jump to next accessor
        CL_BREAK_IF(ac->next && *ac->next->type != *targetTypeOfPtr(ac->type));
        ac = ac->next;
    }

    // go through the chain of accessors
    TOffset off = 0;
    bool isRef = false;
    for (; ac; ac = ac->next) {
        const enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_REF:
                CL_BREAK_IF(isRef || ac->next);
                isRef = true;
                continue;

            case CL_ACCESSOR_DEREF:
                CL_BREAK_IF("chaining of CL_ACCESSOR_DEREF not supported");
                continue;

            case CL_ACCESSOR_DEREF_ARRAY:
                off += offDerefArray(ac);
                break;

            case CL_ACCESSOR_ITEM:
                off += offItem(ac);
                break;
        }
    }

    // resolve type of the target
    const TObjType cltTarget = (isRef)
        ? /* FIXME: too strict in some cases */ targetTypeOfPtr(op.type)
        : op.type;

    if (isDeref)
        // read the value inside the pointer
        addr = sh_.valueOf(sh_.ptrAt(addr));

    // apply the offset
    addr = sh_.valByOffset(addr, off);

    if (isDeref && this->checkForInvalidDeref(addr, cltTarget))
        return VAL_DEREF_FAILED;

    return addr;
}

TObjId SymProc::objByOperand(const struct cl_operand &op) {
    CL_BREAK_IF(seekRefAccessor(op.accessor));
    const TValId at = this->targetAt(op);
    if (VAL_DEREF_FAILED == at)
        return OBJ_DEREF_FAILED;

    const TObjId obj = sh_.objAt(at, op.type);
    switch (obj) {
        case OBJ_INVALID:
        case OBJ_DELETED:
        case OBJ_LOST:
            CL_BREAK_IF("SymProc::objByOperand() failed to resolve an object");
            return OBJ_INVALID;

        case OBJ_UNKNOWN:
        case OBJ_DEREF_FAILED:
        case OBJ_RETURN:
        default:
            return obj;
    }
}

TValId SymProc::heapValFromObj(const struct cl_operand &op) {
    if (seekRefAccessor(op.accessor))
        return this->targetAt(op);

    const TObjId obj = this->objByOperand(op);
    switch (obj) {
        case OBJ_UNKNOWN:
            return sh_.valCreate(VT_UNKNOWN, VO_REINTERPRET);

        case OBJ_DEREF_FAILED:
            return VAL_DEREF_FAILED;

        default:
            if (obj < 0)
                return VAL_INVALID;
    }

    return sh_.valueOf(obj);
}

TValId SymProc::valFromOperand(const struct cl_operand &op) {
    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
            return this->heapValFromObj(op);

        case CL_OPERAND_CST:
            return this->heapValFromCst(op);

        default:
            CL_BREAK_IF("invalid call of SymProc::valFromOperand()");
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
        const TValId val = this->valFromOperand(op);
        CL_BREAK_IF(VAL_INVALID == val);

        // obtain the inner content of the custom value
        return sh_.valGetCustom(val);
    }
}

void SymProc::heapObjDefineType(TObjId lhs, TValId rhs) {
    const TObjId var = sh_.objAt(rhs);
    CL_BREAK_IF(OBJ_INVALID == var);

    TObjType clt = sh_.objType(lhs);
    if (!clt)
        return;

    // move to next clt
    // --> what are we pointing to actually?
    clt = targetTypeOfPtr(clt);
    if (CL_TYPE_VOID == clt->code)
        return;

    // anonymous objects of zero size are not allowed
    const int cbGot = sh_.valSizeOfTarget(rhs);
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

    sh_.objDefineType(var, clt);
}

void SymProc::heapSetSingleVal(TObjId lhs, TValId rhs) {
    // save the old value, which is going to be overwritten
    const TValId oldValue = sh_.valueOf(lhs);
    CL_BREAK_IF(VAL_INVALID == oldValue);

    if (0 < rhs) {
        const TObjId target = sh_.objAt(rhs);
        if (0 < target && !sh_.objType(target))
            // anonymous object is going to be specified by a type
            this->heapObjDefineType(lhs, rhs);
    }

    sh_.objSetValue(lhs, rhs);
    if (collectJunk(sh_, oldValue, lw_))
        bt_->printBackTrace();
}

class ValueWriter {
    private:
        SymProc             &proc_;
        const TValId        valToWrite_;

    public:
        ValueWriter(SymProc *proc, TValId valToWrite):
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
        SymHeap             &sh_;
        const TValId        root_;
        const TObjType      rootClt_;
        const TObjId        preserverSubtree_;

    public:
        UnionInvalidator(SymProc *proc, TObjId root, TObjId preserverSubtree):
            proc_(*proc),
            sh_(proc->sh_),
            root_(sh_.placedAt(root)),
            rootClt_(sh_.objType(root)),
            preserverSubtree_(preserverSubtree)
        {
            CL_BREAK_IF(!rootClt_ || rootClt_->code != CL_TYPE_UNION);
        }

        bool operator()(const TFieldIdxChain &ic, const struct cl_type_item *it)
        {
            SymHeap &sh = proc_.sh_;
            const TObjType clt = it->type;
            if (isComposite(clt))
                return /* continue */ true;

            const TOffset off = offsetByIdxChain(rootClt_, ic);
            const TValId at = sh.valByOffset(root_, off);
            const TObjId sub = sh.objAt(at, clt);

            // first ensure we don't overwrite something we don't want to
            TObjId obj = sub;
            do {
                if (preserverSubtree_ == obj)
                    return /* continue */ true;
            }
            while (OBJ_INVALID != (obj = /* XXX */ sh.objParent(obj)));

            const TValId val = sh.valCreate(VT_UNKNOWN, VO_REINTERPRET);
            proc_.heapSetSingleVal(sub, val);
            return /* continue */ true;
        }
};

class ValueMirror {
    private:
        SymProc &proc_;

    public:
        ValueMirror(SymProc *proc): proc_(*proc) { }

        bool operator()(const TObjId item[2]) const {
            const TObjId lhs = item[0];
            const TValId rhs = proc_.sh_.valueOf(item[1]);
            proc_.heapSetSingleVal(lhs, rhs);

            return /* continue */ true;
        }
};

void SymProc::objSetValue(TObjId lhs, TValId rhs) {
    // seek all surrounding unions on the way to root (if any)
    TObjId parent, obj = lhs;
    for (; OBJ_INVALID != (parent = sh_.objParent(obj)); obj = parent) {
        const TObjType clt = sh_.objType(parent);
        if (!clt || clt->code != CL_TYPE_UNION)
            continue;

        // invalidate all siblings within the surrounding union
        UnionInvalidator visitor(this, parent, obj);
        traverseTypeIc(clt, visitor, /* digOnlyStructs */ true);
    }

    // FIXME: handle some other special values also this way?
    if (VAL_DEREF_FAILED == rhs) {
        // we're already on an error path
        const TObjType clt = sh_.objType(lhs);
        if (!clt || clt->code != CL_TYPE_STRUCT) {
            sh_.objSetValue(lhs, rhs);
            return;
        }

        // fill values of all sub-objects by 'rhs'
        ValueWriter writer(this, rhs);
        traverseLiveObjs(sh_, sh_.placedAt(lhs), writer);
        return;
    }

    // FIXME: this doesn't work well if we get UV_UNKNOWN of type CL_TYPE_STRUCT
    const TObjId rObj = sh_.valGetCompositeObj(rhs);
    if (OBJ_INVALID == rObj) {
        // non-composite value
        this->heapSetSingleVal(lhs, rhs);
        return;
    }

    // DFS for composite types
    const ValueMirror mirror(this);
    const TValId roots[] = { sh_.placedAt(lhs), sh_.placedAt(rObj) };
    traverseLiveObjs<2>(sh_, roots, mirror);
}

void SymProc::valDestroyTarget(TValId addr) {
    // gather potentialy destroyed pointer sub-values
    std::vector<TValId> ptrs;
    getPtrValues(ptrs, sh_, addr);

    // destroy object recursively
    if (0 < addr)
        sh_.valDestroyTarget(addr);
    else
        CL_BREAK_IF("attempt to destroy an invalid object");

    // now check for JUNK
    bool junk = false;
    BOOST_FOREACH(TValId val, ptrs) {
        if (collectJunk(sh_, val, lw_))
            junk = true;
    }

    if (junk)
        // print backtrace at most once per one call of objDestroy()
        bt_->printBackTrace();
}

void SymProc::killVar(const struct cl_operand &op) {
#if DEBUG_SE_STACK_FRAME
    const int uid = varIdFromOperand(&op);
    const CodeStorage::Storage &stor = sh_.stor();
    const std::string varString = varTostring(stor, uid);
    CL_DEBUG_MSG(lw_, "FFF SymProc::killVar() destroys var " << varString);
#endif
    const TValId addr = this->varAt(op);
    this->valDestroyTarget(addr);
}

void SymProc::killInsn(const CodeStorage::Insn &insn) {
#if !SE_EARLY_VARS_DESTRUCTION
    return;
#endif
    // kill variables
    const CodeStorage::TOperandList &ops = insn.operands;
    for (unsigned i = 0; i < ops.size(); ++i) {
        if (insn.opsToKill[i])
            this->killVar(ops[i]);
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymExecCore implementation
TValId SymExecCore::valFromOperand(const struct cl_operand &op) {
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

        return sh_.valCreate(VT_UNKNOWN, VO_ASSIGNED);
    }

no_nasty_assumptions:
    return SymProc::valFromOperand(op);
}

bool SymExecCore::lhsFromOperand(TObjId *pObj, const struct cl_operand &op) {
    *pObj = this->objByOperand(op);
    switch (*pObj) {
        case OBJ_UNKNOWN:
            CL_ERROR_MSG(lw_, "attempt to use an unknown value as l-value");
            bt_->printBackTrace();
            // fall through!

        case OBJ_DEREF_FAILED:
            return false;

        default:
            CL_BREAK_IF(*pObj <= 0);
            return true;
    }
}

void SymExecCore::execFreeCore(const TValId val) {
    const TObjId obj = sh_.objAt(val);
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

    if (sh_.valOffset(val)) {
        CL_ERROR_MSG(lw_, "attempt to free a non-root object");
        bt_->printBackTrace();
        return;
    }

    if (isProgramVar(sh_.valTarget(val))) {
        CVar cv = sh_.cVarByRoot(val);
        CL_DEBUG("about to free var " << varTostring(sh_.stor(), cv.uid));
        CL_ERROR_MSG(lw_, "attempt to free a non-heap object");
        bt_->printBackTrace();
        return;
    }

    this->valDestroyTarget(val);
}

void SymExecCore::execFree(const CodeStorage::TOperandList &opList) {
    CL_BREAK_IF(/* dst + fnc + ptr */ 3 != opList.size());

    // free() does not usually return a value
    CL_BREAK_IF(CL_OPERAND_VOID != opList[0].code);

    // resolve value to be freed
    TValId val = valFromOperand(opList[/* ptr given to free() */2]);
    switch (val) {
        case VAL_INVALID:
            CL_BREAK_IF("SymExecCore::execFree() got invalid value");
            return;

        case VAL_NULL:
            CL_DEBUG_MSG(lw_, "ignoring free() called with NULL value");
            return;

        case VAL_DEREF_FAILED:
            CL_DEBUG_MSG(lw_, "ignoring free() called on VAL_DEREF_FAILED");
            return;

        default:
            break;
    }

    if (isUninitialized(sh_.valOrigin(val))) {
        CL_ERROR_MSG(lw_, "free() called on uninitialized value");
        bt_->printBackTrace();
        return;
    }

    const EValueTarget code = sh_.valTarget(val);
    if (!isPossibleToDeref(code) && !isGone(code)) {
        CL_ERROR_MSG(lw_, "free() called on unknown value");
        bt_->printBackTrace();
        return;
    }

    CL_DEBUG_MSG(lw_, "executing free()");
    this->execFreeCore(val);
}

void SymExecCore::execMalloc(SymState                           &state,
                             const CodeStorage::Insn            &insn)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    CL_BREAK_IF(/* dst + fnc + size */ 3 != opList.size());

    // resolve lhs
    TObjId lhs = OBJ_INVALID;
    if (!this->lhsFromOperand(&lhs, opList[/* dst */ 0]))
        // error alredy emitted
        return;

    // amount of allocated memory must be a constant
    const struct cl_operand &amount = opList[2];
    const int cbAmount = intCstFromOperand(&amount);
    CL_DEBUG_MSG(lw_, "executing malloc(" << cbAmount << ")");

    // now create a heap object
    const TValId val = sh_.heapAlloc(cbAmount);
    CL_BREAK_IF(val <= 0);

    if (!ep_.fastMode) {
        // OOM state simulation
        this->objSetValue(lhs, VAL_NULL);
        this->killInsn(insn);
        state.insert(sh_);
    }

    // store the result of malloc
    this->objSetValue(lhs, val);
    this->killInsn(insn);
    state.insert(sh_);
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
        this->execMalloc(dst, insn);
        return true;
    }

    if (STREQ(fncName, "free")) {
        this->execFree(opList);
        this->killInsn(insn);
        dst.insert(sh_);
        return true;
    }

    return handleBuiltIn(dst, *this, insn);
}

bool describeCmpOp(
        const enum cl_binop_e       code,
        bool                        *pNegative,
        bool                        *pPreserveEq,
        bool                        *pPreserveNeq)
{
    *pNegative = false;
    *pPreserveEq = false;
    *pPreserveNeq = false;

    switch (code) {
        case CL_BINOP_EQ:
            *pPreserveEq = true;
            // fall through!

        case CL_BINOP_LE:
        case CL_BINOP_GE:
            *pPreserveNeq = true;
            return true;

        case CL_BINOP_NE:
            *pNegative = true;
            *pPreserveEq = true;
            *pPreserveNeq = true;
            // fall through!

        case CL_BINOP_LT:
        case CL_BINOP_GT:
            return true;

        default:
            // unhandled binary operator
            return false;
    }
}

TValId compareValues(
        SymHeap                     &sh,
        const enum cl_binop_e       code,
        const TObjType              clt,
        const TValId                v1,
        const TValId                v2)
{
    if (VAL_DEREF_FAILED == v1 || VAL_DEREF_FAILED == v2)
        return VAL_DEREF_FAILED;

    // resolve binary operator
    bool neg, preserveEq, preserveNeq;
    if (!describeCmpOp(code, &neg, &preserveEq, &preserveNeq)) {
        CL_WARN("binary operator not implemented yet");
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    // inconsistency check
    if ((v1 == v2) && preserveNeq)
        return boolToVal(!neg);
    if (sh.proveNeq(v1, v2) && preserveEq)
        return boolToVal(neg);

    // forward unknown bool values if possible
    if (CL_TYPE_BOOL == clt->code) {
        if (v1 == boolToVal(!neg))
            return v2;
        if (v2 == boolToVal(!neg))
            return v1;
    }

    // propagate UV_UNINITIALIZED
    const EValueOrigin vo1 = sh.valOrigin(v1);
    const EValueOrigin vo2 = sh.valOrigin(v2);

    const bool isUninit1 = isUninitialized(vo1);
    const bool isUninit2 = isUninitialized(vo2);

    EValueOrigin vo = VO_UNKNOWN;
    if (isUninit1 && isUninit2)
        vo = std::min(vo1, vo2);
    else if (isUninit1)
        vo = vo1;
    else if (isUninit2)
        vo = vo2;

    return sh.valCreate(VT_UNKNOWN, vo);
}

TValId SymExecCore::handlePointerPlus(
        const TValId                at,
        const struct cl_operand     &op)
{
    if (CL_OPERAND_CST != op.code) {
        CL_ERROR_MSG(lw_, "pointer plus offset not known in compile-time");
        return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    if (VAL_NULL == at) {
        CL_DEBUG_MSG(lw_, "pointer plus with NULL treated as unknown value");
        return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    const TOffset offRequested = intCstFromOperand(&op);
    CL_DEBUG("handlePointerPlus(): " << offRequested << "b offset requested");
    return sh_.valByOffset(at, offRequested);
}

// template for generic (unary, binary, ...) operator handlers
template <int ARITY>
struct OpHandler {
    static TValId handleOp(
            SymProc                 &proc,
            int                     code,
            const TValId            rhs[ARITY],
            const TObjType          clt[ARITY +/* dst */1]);
};

// unary operator handler
template <>
struct OpHandler</* unary */ 1> {
    static TValId handleOp(
            SymProc                 &proc,
            int                     iCode,
            const TValId            rhs[1],
            const TObjType          clt[1 + /* dst type */ 1])
    {
        SymHeap &sh = proc.sh_;
        const TValId val = rhs[0];

        const enum cl_unop_e code = static_cast<enum cl_unop_e>(iCode);
        switch (code) {
            case CL_UNOP_TRUTH_NOT:
                return compareValues(sh, CL_BINOP_EQ, clt[0], VAL_FALSE, val);

            case CL_UNOP_ASSIGN:
                return val;

            default:
                CL_WARN_MSG(proc.lw_, "unary operator not implemented yet");
                return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        }
    }
};

// binary operator handler
template <>
struct OpHandler</* binary */ 2> {
    static TValId handleOp(
            SymProc                 &proc,
            int                     iCode,
            const TValId            rhs[2],
            const TObjType          clt[2 + /* dst type */ 1])
    {
        CL_BREAK_IF(!clt[0] || !clt[1] || !clt[2]);
        SymHeap &sh = proc.sh_;

        const enum cl_binop_e code = static_cast<enum cl_binop_e>(iCode);
        switch (code) {
            case CL_BINOP_EQ:
            case CL_BINOP_NE:
            case CL_BINOP_LT:
            case CL_BINOP_GT:
            case CL_BINOP_LE:
            case CL_BINOP_GE:
                CL_BREAK_IF(clt[/* src1 */ 0]->code != clt[/* src2 */ 1]->code);
                return compareValues(sh, code, clt[0], rhs[0], rhs[1]);

            default:
                return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
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

    // store type of dst operand
    TObjType clt[ARITY + /* dst type */ 1];
    clt[/* dst type */ ARITY] = dst.type;

    // gather rhs values (and type-info)
    const CodeStorage::TOperandList &opList = insn.operands;
    TValId rhs[ARITY];
    for (int i = 0; i < ARITY; ++i) {
        const struct cl_operand &op = opList[i + /* [+dst] */ 1];
        clt[i] = op.type;

        const TValId val = this->valFromOperand(op);
        CL_BREAK_IF(VAL_INVALID == val);
        if (VAL_DEREF_FAILED == val) {
            // we're already on an error path
            this->objSetValue(varLhs, VAL_DEREF_FAILED);
            return;
        }

        rhs[i] = val;
    }

    TValId valResult = VAL_INVALID;
    if (2 == ARITY && CL_BINOP_POINTER_PLUS
            == static_cast<enum cl_binop_e>(insn.subCode))
    {
        // handle pointer plus
        valResult = this->handlePointerPlus(rhs[0], opList[/* offset */ 2]);
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
    todo.push_back(sh_);
    while (!todo.empty()) {
        SymHeap &sh = todo.front();
        SymExecCore core(sh, bt_, ep_);
        core.setLocation(lw_);

        // TODO: implement full version of the alg (complexity m*n)
        bool hitLocal = false;
        BOOST_FOREACH(unsigned idx, derefs) {
            const struct cl_operand &op = opList.at(idx);

            // we expect a pointer at this point
            const TObjId ptr = sh.objAt(core.varAt(op));
            const TValId val = sh.valueOf(ptr);
            if (VT_ABSTRACT == sh.valTarget(val)) {
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

    // kill variables
    this->killInsn(insn);

    if (feelFreeToOverwrite)
        // aggressive optimization
        dst.insertFast(sh_);
    else
        dst.insert(sh_);

    return true;
}

bool SymExecCore::exec(SymState &dst, const CodeStorage::Insn &insn) {
    lw_ = &insn.loc;
    if (this->concretizeIfNeeded(dst, insn))
        // concretization loop done
        return true;

    return this->execCore(dst, insn, /* aggressive optimization */ true);
}
