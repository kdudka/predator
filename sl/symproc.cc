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
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

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
    const struct cl_cst &cst = op.data.cst;

    CustomValue cv(CV_INVALID);

    const cl_type_e code = cst.code;
    switch (code) {
        case CL_TYPE_ENUM:
        case CL_TYPE_INT:
            // integral value
            cv.code = CV_INT;
            cv.data.num = cst.data.cst_int.value;
            break;

        case CL_TYPE_FNC:
            // code pointer
            cv.code = CV_FNC;
            cv.data.uid = cst.data.cst_fnc.uid;
            break;

        case CL_TYPE_STRING:
            // string literal
            cv.code = CV_STRING;
            cv.data.str = cst.data.cst_string.value;
            break;

        default:
            CL_BREAK_IF("heapValFromCst() got something special");
            break;
    }

    return sh_.valWrapCustom(cv);
}

void describeUnknownVal(
        SymProc                         &proc,
        const TValId                     val,
        const char                      *action)
{
    const struct cl_loc *loc = proc.lw();
    SymHeap &sh = proc.sh();

    const char *what = 0;

    const EValueOrigin code = sh.valOrigin(val);
    switch (code) {
        case VO_DEREF_FAILED:
            CL_BREAK_IF("invalid call of reportDerefOfUnknownVal()");
            // fall through!

        case VO_INVALID:
        case VO_ASSIGNED:
        case VO_UNKNOWN:
            return;

        case VO_REINTERPRET:
            what = "a result of an unsupported data reinterpretation";
            break;

        case VO_STATIC:
            what = "an untouched contents of static data";
            break;

        case VO_STACK:
            what = "an untouched contents of stack";
            break;

        case VO_HEAP:
            what = "an untouched contents of heap";
            break;
    }

    if (what)
        CL_NOTE_MSG(loc, "the value being " << action << "d is " << what);
    else
        CL_BREAK_IF("valOrigin out of range?");
}

const char* describeRootObj(const EValueTarget code) {
    switch (code) {
        case VT_STATIC:
            return "a static variable";

        case VT_ON_STACK:
            return "a variable on stack";

        case VT_ON_HEAP:
            return "a heap object";

        default:
            CL_BREAK_IF("invalid call of describeRootObj()");
            return "a nonsense";
    }
}

void reportDerefOutOfBounds(
        SymProc                         &proc,
        const TValId                    val,
        const TOffset                   sizeOfTarget)
{
    const struct cl_loc *loc = proc.lw();
    SymHeap &sh = proc.sh();

    CL_BREAK_IF(sizeOfTarget <= 0);
    CL_ERROR_MSG(loc, "dereferencing object of size " << sizeOfTarget
            << "B out of bounds");

    // what is the root actually?
    const EValueTarget code = sh.valTarget(val);
    const char *const what = describeRootObj(code);

    const TValId root = sh.valRoot(val);
    const int rootSize = sh.valSizeOfTarget(root);
    CL_BREAK_IF(rootSize <= 0);

    const TOffset off = sh.valOffset(val);
    if (off < 0) {
        CL_NOTE_MSG(loc, "the pointer being dereferenced points "
                << (-off) << "B above " << what
                << " of size " << rootSize << "B");

        return;
    }

    int beyond = off - rootSize;
    if (0 <= beyond) {
        CL_NOTE_MSG(loc, "the pointer being dereferenced points "
                << beyond << "B beyond " << what
                << " of size " << rootSize << "B");

        return;
    }

    beyond = sizeOfTarget - sh.valSizeOfTarget(val);
    if (0 < beyond) {
        CL_NOTE_MSG(loc, "the target object ends "
                << beyond << "B beyond " << what
                << " of size " << rootSize << "B");
    }
}

bool SymProc::checkForInvalidDeref(TValId val, const TOffset sizeOfTarget) {
    if (VAL_NULL == val) {
        CL_ERROR_MSG(lw_, "dereference of NULL value");
        return true;
    }

    if (VAL_NULL == sh_.valRoot(val)) {
        const TOffset off = sh_.valOffset(val);
        CL_ERROR_MSG(lw_, "dereference of NULL value with offset "
                << off << "B");

        return true;
    }

    const EValueTarget code = sh_.valTarget(val);
    switch (code) {
        case VT_INVALID:
        case VT_COMPOSITE:
        case VT_CUSTOM:
        case VT_ABSTRACT:
            CL_BREAK_IF("attempt to dereference something special");
            // fall through!

        case VT_UNKNOWN:
            CL_ERROR_MSG(lw_, "invalid dereference");
            describeUnknownVal(*this, val, "dereference");
            return true;

        case VT_LOST:
            CL_ERROR_MSG(lw_, "dereference of non-existing non-heap object");
            return true;

        case VT_DELETED:
            CL_ERROR_MSG(lw_, "dereference of already deleted heap object");
            return true;

        case VT_STATIC:
        case VT_ON_STACK:
        case VT_ON_HEAP:
            break;
    }

    if (sh_.valOffset(val) < 0 || sh_.valSizeOfTarget(val) < sizeOfTarget) {
        // out of bounds
        reportDerefOutOfBounds(*this, val, sizeOfTarget);
        return true;
    }

    // all OK
    return false;
}

void SymProc::varInit(TValId at) {
    const CVar cv = sh_.cVarByRoot(at);
    const CodeStorage::Storage &stor = sh_.stor();
    const CodeStorage::Var &var = stor.vars[cv.uid];

    SymExecCoreParams ep;
    ep.skipVarInit = /* avoid an infinite recursion */ true;
    SymExecCore core(sh_, bt_, ep);
    BOOST_FOREACH(const CodeStorage::Insn *insn, var.initials) {
        core.setLocation(&insn->loc);
        CL_DEBUG_MSG(&insn->loc,
                "(I) executing an explicit var initializer: " << *insn);
        SymHeapList dst;

        if (!core.exec(dst, *insn))
            CL_BREAK_IF("initVariable() malfunction");

        CL_BREAK_IF(1 != dst.size());
        SymHeap &result = const_cast<SymHeap &>(dst[/* the only result */ 0]);
        sh_.swap(result);
    }
}

TValId SymProc::varAt(const struct cl_operand &op) {
    // resolve CVar
    const int uid = varIdFromOperand(&op);
    const int nestLevel = bt_->countOccurrencesOfTopFnc();
    const CVar cVar(uid, nestLevel);

    // get the address (SymHeapCore is responsible for lazy creation)
    const TValId at = sh_.addrOfVar(cVar);
    CL_BREAK_IF(at <= 0);

    // resolve Var
    const CodeStorage::Storage &stor = sh_.stor();
    const CodeStorage::Var &var = stor.vars[uid];
    bool needInit = !var.initials.empty();
#if !SE_ASSUME_FRESH_STATIC_DATA
    needInit &= isOnStack(var);
#endif
    if (!needInit)
        // in this case, we do not care if the var is initialized or not
        return at;

    TObjList liveObjs;
    sh_.gatherLiveObjects(liveObjs, at);
    if (!liveObjs.empty())
        // not a fresh variable --> preserve its contents
        return at;

    // delayed initialization
    this->varInit(at);
    return at;
}

bool SymProc::addOffDerefArray(TOffset &off, const struct cl_accessor *ac) {
    // read value of the operand that is used as an array index
    const struct cl_operand *opIdx = ac->data.array.index;
    const TValId valIdx = this->valFromOperand(*opIdx);

    // unwrap the integral value inside the heap value (if available)
    long idx;
    if (!numFromVal(&idx, sh_, valIdx))
        return false;

    // compute the resulting offset
    const TObjType clt = targetTypeOfArray(ac->type);
    off += idx * clt->size;
    return true;
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
    for (; ac; ac = ac->next) {
        const enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_REF:
                CL_BREAK_IF(ac->next);
                continue;

            case CL_ACCESSOR_DEREF:
                CL_BREAK_IF("chaining of CL_ACCESSOR_DEREF not supported");
                continue;

            case CL_ACCESSOR_DEREF_ARRAY:
                if (this->addOffDerefArray(off, ac))
                    continue;
                else
                    // no clue how to compute the resulting offset
                    return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);

            case CL_ACCESSOR_ITEM:
                off += offItem(ac);
                continue;
        }
    }

    if (isDeref) {
        // read the value inside the pointer
        bool exclusive;
        const TObjId ptr = sh_.ptrAt(addr, &exclusive);
        addr = sh_.valueOf(ptr);
        if (exclusive)
            sh_.objReleaseId(ptr);
    }

    // apply the offset
    return sh_.valByOffset(addr, off);
}

TObjId SymProc::objByOperand(const struct cl_operand &op, bool *exclusive) {
    CL_BREAK_IF(seekRefAccessor(op.accessor));

    // resolve address of the target object
    const TValId at = this->targetAt(op);
    const EValueOrigin origin = sh_.valOrigin(at);
    if (VO_DEREF_FAILED == origin)
        // we are already on the error path
        return OBJ_DEREF_FAILED;

    // check for invalid dereference
    const TObjType cltTarget = op.type;
    if (this->checkForInvalidDeref(at, cltTarget->size)) {
        bt_->printBackTrace();
        return OBJ_DEREF_FAILED;
    }

    // resolve the target object
    const TObjId obj = sh_.objAt(at, op.type, exclusive);
    if (obj <= 0)
        CL_BREAK_IF("SymProc::objByOperand() failed to resolve an object");

    // all OK
    return obj;
}

TValId SymProc::heapValFromObj(const struct cl_operand &op) {
    if (seekRefAccessor(op.accessor))
        return this->targetAt(op);

    bool exclusive;
    const TObjId obj = this->objByOperand(op, &exclusive);
    switch (obj) {
        case OBJ_UNKNOWN:
            return sh_.valCreate(VT_UNKNOWN, VO_REINTERPRET);

        case OBJ_DEREF_FAILED:
            return sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

        default:
            if (obj < 0)
                return VAL_INVALID;
    }

    const TValId val = sh_.valueOf(obj);
    if (exclusive)
        sh_.objReleaseId(obj);

    return val;
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

bool SymProc::fncFromOperand(int *pUid, const struct cl_operand &op) {
    if (CL_OPERAND_CST == op.code) {
        // direct call
        const struct cl_cst &cst = op.data.cst;
        if (CL_TYPE_FNC != cst.code)
            return false;

        *pUid = cst.data.cst_fnc.uid;
        return true;
    }

    // assume indirect call
    const TValId val = this->valFromOperand(op);
    if (VT_CUSTOM != sh_.valTarget(val))
        // not a custom value
        return false;

    CustomValue cv = sh_.valUnwrapCustom(val);
    if (CV_FNC != cv.code)
        // not a pointer to function
        return false;

    *pUid = cv.data.uid;
    return true;
}

void SymProc::heapObjDefineType(TObjId lhs, TValId rhs) {
    if (!isPossibleToDeref(sh_.valTarget(rhs)))
        // no valid target anyway
        return;

    if (sh_.valOffset(rhs))
        // not a pointer to root
        return;

    const TObjType cltTarget = targetTypeOfPtr(sh_.objType(lhs));
    if (!cltTarget || CL_TYPE_VOID == cltTarget->code)
        // no type-info given for the target
        return;

    const TObjType cltLast = sh_.valLastKnownTypeOfTarget(rhs);
    if (isComposite(cltLast) && !isComposite(cltTarget))
        // we are accessing a field that is placed at zero offset of a composite
        // type but it yet does not mean that we are changing the root type-info
        return;

    const int rootSize = sh_.valSizeOfTarget(rhs);
    CL_BREAK_IF(rootSize <= 0);
    if (cltLast && cltLast->size == rootSize && cltTarget->size != rootSize)
        // basically the same rule as above but now we check the size of target
        return;

    // update the last known type-info of the root
    sh_.valSetLastKnownTypeOfTarget(rhs, cltTarget);
}

void SymProc::reportMemLeak(const EValueTarget code, const char *reason) {
    const char *const what = describeRootObj(code);
    CL_WARN_MSG(lw_, "memory leak detected while " << reason << "ing " << what);
    bt_->printBackTrace();
}

void SymProc::heapSetSingleVal(TObjId lhs, TValId rhs) {
    if (lhs < 0) {
        CL_ERROR_MSG(lw_, "invalid L-value");
        bt_->printBackTrace();
        return;
    }

    const TValId lhsAt = sh_.placedAt(lhs);
    const EValueTarget code = sh_.valTarget(lhsAt);
    CL_BREAK_IF(!isPossibleToDeref(code));

    // update type-info
    this->heapObjDefineType(lhs, rhs);

    LeakMonitor lm(sh_);
    lm.enter();

    TValSet killedPtrs;
    sh_.objSetValue(lhs, rhs, &killedPtrs);

    if (lm.collectJunkFrom(killedPtrs))
        this->reportMemLeak(code, "assign");

    lm.leave();
}

class ValueMirror {
    private:
        SymProc             &proc_;
        SymHeap             &sh_;
        int                 end_;

    public:
        ValueMirror(SymProc *proc, const TObjId rObj):
            proc_(*proc),
            sh_(proc->sh())
        {
            const TOffset off = sh_.valOffset(sh_.placedAt(rObj));
            const TObjType clt = sh_.objType(rObj);
            end_ = off + clt->size;
        }

        bool operator()(const TObjId item[2]) const {
            const TObjId lhs = item[0];
            const TObjId rhs = item[1];
            if (0 < rhs) {
                const TValId rhsAt = sh_.placedAt(rhs);
                const TOffset off = sh_.valOffset(rhsAt);
                if (end_ <= off)
                    // we are above the end_ of rhs
                    return /* continue */ true;
            }

            const TValId val = sh_.valueOf(rhs);
            proc_.heapSetSingleVal(lhs, val);

            return /* continue */ true;
        }
};

void SymProc::objSetValue(TObjId lhs, TValId rhs) {
    CL_BREAK_IF(!isPossibleToDeref(sh_.valTarget(sh_.placedAt(lhs))));
    CL_BREAK_IF(!sh_.objType(lhs));

    if (VO_DEREF_FAILED == sh_.valOrigin(rhs)) {
        // we're already on an error path
        const TValId lhsAt = sh_.placedAt(lhs);
        const TValId tplValue = sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);
        const unsigned size = sh_.objType(lhs)->size;
        sh_.writeUniformBlock(lhsAt, tplValue, size);
        return;
    }

    if (VT_COMPOSITE != sh_.valTarget(rhs)) {
        this->heapSetSingleVal(lhs, rhs);
        return;
    }

    // object-wise copy of a composite type
    const TObjId rObj = sh_.valGetComposite(rhs);
    CL_BREAK_IF(rObj < 0);

    const TValId roots[] = {
        sh_.placedAt(lhs),
        sh_.placedAt(rObj)
    };

    const ValueMirror mirror(this, rObj);
    traverseLiveObjs<2>(sh_, roots, mirror);
}

void SymProc::valDestroyTarget(TValId addr) {
    const EValueTarget code = sh_.valTarget(addr);
    if (VAL_ADDR_OF_RET == addr && isGone(code))
        return;

    LeakMonitor lm(sh_);
    lm.enter();

    TValList killedPtrs;
    destroyRootAndCollectPtrs(sh_, addr, &killedPtrs);

    if (lm.collectJunkFrom(killedPtrs))
        this->reportMemLeak(code, "destroy");

    lm.leave();
}

void SymProc::killVar(const CodeStorage::KillVar &kv) {
    const int uid = kv.uid;
#if DEBUG_SE_STACK_FRAME
    const CodeStorage::Storage &stor = sh_.stor();
    const std::string varString = varToString(stor, uid);
    CL_DEBUG_MSG(lw_, "FFF SymProc::killVar() destroys var " << varString);
#endif

    // get the address (SymHeapCore is responsible for lazy creation)
    const int nestLevel = bt_->countOccurrencesOfTopFnc();
    const CVar cVar(uid, nestLevel);
    const TValId addr = sh_.addrOfVar(cVar);

    if (kv.onlyIfNotPointed && sh_.pointedByCount(addr))
        // somebody points at the var, please wait with its destruction
        return;

    this->valDestroyTarget(addr);
}

bool headingToAbort(const CodeStorage::Block *bb) {
    const CodeStorage::Insn *term = bb->back();
    const cl_insn_e code = term->code;
    return (CL_INSN_ABORT == code);
}

void SymProc::killInsn(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;
#if !SE_EARLY_VARS_DESTRUCTION
    return;
#endif
    // kill variables
    BOOST_FOREACH(const KillVar &kv, insn.varsToKill)
        this->killVar(kv);
}

void SymProc::killPerTarget(const CodeStorage::Insn &insn, unsigned target) {
    using namespace CodeStorage;
#if SE_EARLY_VARS_DESTRUCTION
    if (headingToAbort(insn.targets[target]))
#endif
        // There is no point in killing local variables if we are heading to
        // CL_INSN_ABORT.  It can only cause unnecessary reporting of memleak
        // in the OOM handling abort branch.
        return;

    const TKillVarList &kList = insn.killPerTarget[target];
    BOOST_FOREACH(const KillVar &kv, kList)
        this->killVar(kv);
}

// /////////////////////////////////////////////////////////////////////////////
// SymExecCore implementation
void SymExecCore::varInit(TValId at) {
    if (ep_.skipVarInit)
        // we are explicitly asked to not initialize any vars
        return;

    SymProc::varInit(at);
}

void SymExecCore::execFree(TValId val) {
    if (VAL_NULL == val) {
        CL_DEBUG_MSG(lw_, "ignoring free() called with NULL value");
        return;
    }

    const EValueTarget code = sh_.valTarget(val);
    switch (code) {
        case VT_DELETED:
            CL_ERROR_MSG(lw_, "double free()");
            bt_->printBackTrace();
            return;

        case VT_LOST:
            CL_ERROR_MSG(lw_, "attempt to free a non-existing non-heap object");
            bt_->printBackTrace();
            return;

        case VT_STATIC:
        case VT_ON_STACK:
            CL_ERROR_MSG(lw_, "attempt to free a non-heap object");
            bt_->printBackTrace();
            return;

        case VT_CUSTOM:
            CL_ERROR_MSG(lw_, "free() called on non-pointer value");
            bt_->printBackTrace();
            return;

        case VT_INVALID:
        case VT_ABSTRACT:
        case VT_COMPOSITE:
            CL_BREAK_IF("invalid call of SymExecCore::execFreeCore()");
            // fall through!

        case VT_UNKNOWN:
            if (VO_DEREF_FAILED == sh_.valOrigin(val))
                return;

            CL_ERROR_MSG(lw_, "invalid free()");
            describeUnknownVal(*this, val, "free");
            bt_->printBackTrace();
            return;

        case VT_ON_HEAP:
            break;
    }

    const TOffset off = sh_.valOffset(val);
    if (off) {
        CL_ERROR_MSG(lw_, "free() called with offset " << off << "B");
        bt_->printBackTrace();
        return;
    }

    CL_DEBUG_MSG(lw_, "executing free()");
    this->valDestroyTarget(val);
}

void SymExecCore::execHeapAlloc(
        SymState                        &dst,
        const CodeStorage::Insn         &insn,
        const unsigned                  size,
        const bool                      nullified)
{
    // resolve lhs
    const TObjId lhs = this->objByOperand(insn.operands[/* dst */ 0]);
    if (OBJ_DEREF_FAILED == lhs)
        // error alredy emitted
        return;

    if (!size) {
        CL_WARN_MSG(lw_, "POSIX says that, given zero size, the behavior of \
malloc/calloc is implementation-defined");
        CL_NOTE_MSG(lw_, "assuming NULL as the result");
        bt_->printBackTrace();
        this->objSetValue(lhs, VAL_NULL);
        this->killInsn(insn);
        sh_.objReleaseId(lhs);
        dst.insert(sh_);
        return;
    }

    if (!ep_.fastMode) {
        // OOM state simulation
        this->objSetValue(lhs, VAL_NULL);
        this->killInsn(insn);

        // FIXME: Ooops, sh_.objReleaseId(lhs) would affect both results
        dst.insert(sh_);
    }

    // now create a heap object
    const TValId val = sh_.heapAlloc(size);
    if (nullified)
        // initilize to zero as we are doing calloc()
        sh_.writeUniformBlock(val, VAL_NULL, size);

    // store the result of malloc
    this->objSetValue(lhs, val);
    this->killInsn(insn);
    sh_.objReleaseId(lhs);
    dst.insert(sh_);
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
            *pPreserveEq = true;
            // fall through!

        case CL_BINOP_LT:
        case CL_BINOP_GT:
            *pNegative = true;
            *pPreserveNeq = true;
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
    const EValueOrigin vo1 = sh.valOrigin(v1);
    const EValueOrigin vo2 = sh.valOrigin(v2);
    if (VO_DEREF_FAILED == vo1 || VO_DEREF_FAILED == vo2)
        return sh.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

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

TValId SymProc::handlePointerPlus(TValId at, TValId off) {
    long num;
    if (!numFromVal(&num, sh_, off)) {
        CL_DEBUG_MSG(lw_, "pointer plus offset not a known integer");
        return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    const TOffset offRequested = static_cast<TOffset>(num);
    CL_DEBUG("handlePointerPlus(): " << offRequested << "b offset requested");
    return sh_.valByOffset(at, offRequested);
}

void printBackTrace(SymProc &proc) {
    const SymBackTrace *bt = proc.bt();
    bt->printBackTrace();
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

        long num1, num2;

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

            case CL_BINOP_POINTER_PLUS:
                return proc.handlePointerPlus(rhs[0], rhs[1]);

            case CL_BINOP_MULT:
                if (numFromVal(&num1, sh, rhs[0])
                        && numFromVal(&num2, sh, rhs[1]))
                {
                    CL_DEBUG_MSG(proc.lw(), "executing of CL_BINOP_MULT");

                    // FIXME: this will likely break the fix-point in some cases
                    CustomValue cVal(CV_INT);
                    cVal.data.num = num1 * num2;
                    return sh.valWrapCustom(cVal);
                }
                // fall through!

            default:
                return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        }
    }
};

template <int ARITY>
void SymExecCore::execOp(const CodeStorage::Insn &insn) {
    // resolve lhs
    const struct cl_operand &dst = insn.operands[/* dst */ 0];
    const TObjId varLhs = this->objByOperand(dst);
    if (OBJ_DEREF_FAILED == varLhs)
        // error alredy emitted
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
        if (VO_DEREF_FAILED == sh_.valOrigin(val)) {
            // we're already on an error path
            const TValId vFail = sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);
            this->objSetValue(varLhs, vFail);
            sh_.objReleaseId(varLhs);
            return;
        }

        rhs[i] = val;
    }

    const TValId valResult =
        OpHandler<ARITY>::handleOp(*this, insn.subCode, rhs, clt);

#if !SE_TRACK_NON_POINTER_VALUES
    // avoid creation of live object in case we are not interested in its value
    if (!isDataPtr(dst.type) && VO_UNKNOWN == sh_.valOrigin(valResult)) {
        const TValId root = sh_.valRoot(sh_.placedAt(varLhs));

        TObjList liveObjs;
        sh_.gatherLiveObjects(liveObjs, root);
        BOOST_FOREACH(const TObjId obj, liveObjs)
            if (obj == varLhs)
                goto already_alive;

        sh_.objReleaseId(varLhs);
        return;
    }
already_alive:
#endif
    // store the result
    this->objSetValue(varLhs, valResult);
    sh_.objReleaseId(varLhs);
}

template <class TDerefs>
bool SymExecCore::concretizeLoop(SymState                       &dst,
                                 const CodeStorage::Insn        &insn,
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
            const struct cl_operand &op = insn.operands.at(idx);

            // we expect a pointer at this point
            const TValId val = valOfPtrAt(sh, core.varAt(op));
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

bool SymExecCore::concretizeIfNeeded(
        SymState                    &results,
        const CodeStorage::Insn     &insn)
{
    TOpIdxList derefs;

    const cl_insn_e code = insn.code;
    if (CL_INSN_CALL == code)
        derefs = opsWithDerefSemanticsInCallInsn(insn);
    else {
        const CodeStorage::TOperandList &opList = insn.operands;

        // look for dereferences
        for (unsigned idx = 0; idx < opList.size(); ++idx) {
            const struct cl_accessor *ac = opList[idx].accessor;

            if (!ac || CL_ACCESSOR_DEREF != ac->code)
                // we expect the dereference only as the first accessor
                continue;

            if (seekRefAccessor(ac))
                // not a dereference, only an address is being computed
                continue;

            CL_BREAK_IF(CL_INSN_UNOP != insn.code);
            CL_BREAK_IF(CL_UNOP_ASSIGN != static_cast<cl_unop_e>(insn.subCode));
            derefs.push_back(idx);
        }
    }

    if (derefs.empty())
        return false;

    // handle dereferences
    this->concretizeLoop(results, insn, derefs);
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
            // the symbin module is now fully responsible for handling built-ins
            return handleBuiltIn(dst, *this, insn);

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
