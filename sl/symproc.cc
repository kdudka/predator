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

#include "memdebug.hh"
#include "symabstract.hh"
#include "symbin.hh"
#include "symbt.hh"
#include "symgc.hh"
#include "symheap.hh"
#include "symplot.hh"
#include "symseg.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "util.hh"

#include <stack>
#include <stdexcept>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymProc implementation
void SymProc::printBackTrace(EMsgLevel level, bool forcePtrace) {
    // update trace graph
    Trace::MsgNode *trMsg = new Trace::MsgNode(sh_.traceNode(), level, lw_);
    sh_.traceUpdate(trMsg);
    CL_BREAK_IF(!chkTraceGraphConsistency(trMsg));

    // print the backtrace
    if (bt_->printBackTrace(forcePtrace))
        printMemUsage("SymBackTrace::printBackTrace");

    // dump trace graph, or schedule and endpoint for batch trace graph dump
#if 2 == SE_DUMP_TRACE_GRAPHS
    Trace::plotTrace(sh_.traceNode(), "symtrace");
#elif 1 == SE_DUMP_TRACE_GRAPHS
    Trace::GraphProxy *glProxy = Trace::Globals::instance()->glProxy();
    glProxy->insert(sh_.traceNode(), "symtrace");
#endif

    if (ML_ERROR != level)
        // do not panic for now
        return;

#if SE_PLOT_ERROR_STATES
    plotHeap(sh_, "error-state", lw_);
#endif

#if SE_ERROR_RECOVERY_MODE
    errorDetected_ = true;
#else
    throw std::runtime_error("support for error recovery not compiled in");
#endif
}

bool SymProc::hasFatalError() const {
#if 1 < SE_ERROR_RECOVERY_MODE
    // full error recovery mode
    return false;
#else
    return errorDetected_;
#endif
}

TValId SymProc::valFromCst(const struct cl_operand &op) {
    const struct cl_cst &cst = op.data.cst;

    CustomValue cv;

    const cl_type_e code = cst.code;
    switch (code) {
        case CL_TYPE_ENUM:
        case CL_TYPE_INT:
            // integral value
            cv = CustomValue(IR::rngFromNum(cst.data.cst_int.value));
            break;

        case CL_TYPE_REAL:
            // floating-point value
            cv = CustomValue(cst.data.cst_real.value);
            break;

        case CL_TYPE_FNC:
            // code pointer
            cv = CustomValue(cst.data.cst_fnc.uid);
            break;

        case CL_TYPE_STRING:
            // string literal
            cv = CustomValue(cst.data.cst_string.value);
            break;

        default:
            CL_BREAK_IF("valFromCst() got something special");
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
            CL_BREAK_IF("invalid call of describeUnknownVal()");
            // fall through!

        case VO_INVALID:
        case VO_ASSIGNED:
        case VO_UNKNOWN:
            return;

        case VO_REINTERPRET:
            what = "a result of an unsupported data reinterpretation";
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
        const TSizeOf                   sizeOfTarget)
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
    const TSizeRange rootSizeRange = sh.valSizeOfTarget(root);
    const TSizeOf minRootSize = rootSizeRange.lo;
    CL_BREAK_IF(minRootSize <= 0);

    const TOffset off = sh.valOffset(val);
    if (off < 0) {
        CL_NOTE_MSG(loc, "the pointer being dereferenced points "
                << (-off) << "B above " << what
                << " of size " << minRootSize << "B");

        return;
    }

    TOffset beyond = off - minRootSize;
    if (0 <= beyond) {
        CL_NOTE_MSG(loc, "the pointer being dereferenced points "
                << beyond << "B beyond " << what
                << " of size " << minRootSize << "B");

        return;
    }

    const TSizeRange dstSizeRange = sh.valSizeOfTarget(val);
    const TSizeOf minDstSize = dstSizeRange.lo;
    beyond = sizeOfTarget - minDstSize;
    if (0 < beyond) {
        CL_NOTE_MSG(loc, "the target object ends "
                << beyond << "B beyond " << what
                << " of size " << minRootSize << "B");
    }
}

bool SymProc::checkForInvalidDeref(TValId val, const TSizeOf sizeOfTarget) {
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
        case VT_RANGE:
            CL_BREAK_IF("please implement");
            // fall through!

        case VT_INVALID:
        case VT_COMPOSITE:
        case VT_ABSTRACT:
            CL_BREAK_IF("attempt to dereference something special");
            // fall through!

        case VT_CUSTOM:
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

    const TSizeRange dstSizeRange = sh_.valSizeOfTarget(val);
    const TSizeOf minDstSize = dstSizeRange.lo;
    if (sh_.valOffset(val) < 0 || minDstSize < sizeOfTarget) {
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
    if (var.initials.empty())
        // nothing to do at this level (already handled by SymExecCore)
        return;

    SymExecCoreParams ep;
    ep.skipVarInit = /* avoid an infinite recursion */ true;
    SymExecCore core(sh_, bt_, ep);
    BOOST_FOREACH(const CodeStorage::Insn *insn, var.initials) {
        const struct cl_loc *loc = &insn->loc;
        core.setLocation(loc);
        CL_DEBUG_MSG(loc,
                "(I) executing an explicit var initializer: " << *insn);
        SymHeapList dst;

        if (!core.exec(dst, *insn))
            CL_BREAK_IF("varInit() malfunction");

        CL_BREAK_IF(1 != dst.size());
        SymHeap &result = const_cast<SymHeap &>(dst[/* the only result */ 0]);
        sh_.swap(result);
    }
}

TValId SymProc::varAt(const CVar &cv) {
    TValId at = sh_.addrOfVar(cv, /* createIfNeeded */ false);
    if (0 < at)
        // var already alive
        return at;

    // lazy var creation
    at = sh_.addrOfVar(cv, /* createIfNeeded */ true);

    // resolve Var
    const CodeStorage::Storage &stor = sh_.stor();
    const CodeStorage::Var &var = stor.vars[cv.uid];
    const bool isLcVar = isOnStack(var);

    // initialize to zero?
    bool nullify = var.initialized;
    if (!isLcVar)
#if SE_ASSUME_FRESH_STATIC_DATA
        nullify = true;
#else
        // do not initialize static variables
        return at;
#endif

    bool needInit = !var.initials.empty();
    if (nullify) {
        // initialize to zero
        const TSizeRange size = sh_.valSizeOfTarget(at);
        CL_BREAK_IF(!isSingular(size));
        sh_.writeUniformBlock(at, VAL_NULL, size.lo);
    }
    else if (isLcVar)
        needInit = true;

    if (needInit)
        // go through explicit initializers
        this->varInit(at);

    return at;
}

TValId SymProc::varAt(const struct cl_operand &op) {
    // resolve CVar
    const int uid = varIdFromOperand(&op);
    const int nestLevel = bt_->countOccurrencesOfTopFnc();
    const CVar cv(uid, nestLevel);
    return this->varAt(cv);
}

bool addOffDerefArray(SymProc &proc, TOffset &off, const struct cl_accessor *ac)
{
    // read value of the operand that is used as an array index
    const struct cl_operand *opIdx = ac->data.array.index;
    const TValId valIdx = proc.valFromOperand(*opIdx);

    // unwrap the integral value inside the heap value (if available)
    IR::TInt idx;
    if (!numFromVal(&idx, proc.sh(), valIdx))
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
        // FIXME: This assertion is known to fail on test-0208.c using gcc-4.6.2
        // as GCC_HOST, yet it works fine with gcc-4.5.3; for some reason, 4.6.2
        // optimizes out the cast from (struct dm_list *) to (struct str_list *)
#if 0
        CL_BREAK_IF(ac->next && *ac->next->type != *targetTypeOfPtr(ac->type));
#endif
        // jump to next accessor
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
                if (addOffDerefArray(*this, off, ac))
                    continue;
                else
                    // no clue how to compute the resulting offset
                    return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);

            case CL_ACCESSOR_ITEM:
                off += offItem(ac);
                continue;

            case CL_ACCESSOR_OFFSET:
                off += ac->data.offset.off;
                continue;
        }
    }

    if (isDeref) {
        // read the value inside the pointer
        const PtrHandle ptr(sh_, addr);
        addr = ptr.value();
    }

    // apply the offset
    return sh_.valByOffset(addr, off);
}

ObjHandle SymProc::objByOperand(const struct cl_operand &op) {
    CL_BREAK_IF(seekRefAccessor(op.accessor));

    // resolve address of the target object
    const TValId at = this->targetAt(op);
    const EValueOrigin origin = sh_.valOrigin(at);
    if (VO_DEREF_FAILED == origin)
        // we are already on the error path
        return ObjHandle(OBJ_DEREF_FAILED);

    // check for invalid dereference
    const TObjType cltTarget = op.type;
    if (this->checkForInvalidDeref(at, cltTarget->size)) {
        this->printBackTrace(ML_ERROR);
        return ObjHandle(OBJ_DEREF_FAILED);
    }

    // resolve the target object
    const ObjHandle obj(sh_, at, op.type);
    if (!obj.isValid())
        CL_BREAK_IF("SymProc::objByOperand() failed to resolve an object");

    // all OK
    return obj;
}

TValId SymProc::valFromObj(const struct cl_operand &op) {
    if (seekRefAccessor(op.accessor))
        return this->targetAt(op);

    const ObjHandle handle = this->objByOperand(op);
    if (handle.isValid())
        return handle.value();

    // failed to resolve object handle
    const TObjId obj = handle.objId();
    switch (obj) {
        case OBJ_UNKNOWN:
            return sh_.valCreate(VT_UNKNOWN, VO_REINTERPRET);

        case OBJ_DEREF_FAILED:
            return sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

        default:
            return VAL_INVALID;
    }
}

TValId SymProc::valFromOperand(const struct cl_operand &op) {
    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_VAR:
            return this->valFromObj(op);

        case CL_OPERAND_CST:
            return this->valFromCst(op);

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
    if (CV_FNC != cv.code())
        // not a pointer to function
        return false;

    *pUid = cv.uid();
    return true;
}

void digRootTypeInfo(SymHeap &sh, const ObjHandle &lhs, TValId rhs) {
    const EValueTarget code = sh.valTarget(rhs);
    if (!isPossibleToDeref(code))
        // no valid target anyway
        return;

    if (sh.valOffset(rhs))
        // not a pointer to root
        return;

    const TObjType cltTarget = targetTypeOfPtr(lhs.objType());
    if (!cltTarget || CL_TYPE_VOID == cltTarget->code)
        // no type-info given for the target
        return;

    const TObjType cltLast = sh.valLastKnownTypeOfTarget(rhs);
    if (isComposite(cltLast) && !isComposite(cltTarget))
        // we are accessing a field that is placed at zero offset of a composite
        // type but it yet does not mean that we are changing the root type-info
        return;

    const TSizeRange rootSizeRange = sh.valSizeOfTarget(rhs);
    const TSizeOf rootSize = rootSizeRange.lo;
    CL_BREAK_IF(rootSize <= 0 && isOnHeap(code));

    if (cltLast && cltLast->size == rootSize && cltTarget->size != rootSize)
        // basically the same rule as above but now we check the size of target
        return;

    // update the last known type-info of the root
    sh.valSetLastKnownTypeOfTarget(rhs, cltTarget);
}

void reportMemLeak(SymProc &proc, const EValueTarget code, const char *reason) {
    const struct cl_loc *loc = proc.lw();
    const char *const what = describeRootObj(code);
    CL_WARN_MSG(loc, "memory leak detected while " << reason << "ing " << what);
    proc.printBackTrace(ML_WARN);
}

/// pointer kind classification
enum EPointerKind {
    PK_DATA,                        ///< data pointer
    PK_CODE                         ///< code pointer
};

// if we are going to write a pointer, check whether we have enough space for it
TValId ptrObjectEncoderCore(
        SymProc                    &proc,
        const ObjHandle            &dst,
        const TValId                val,
        const EPointerKind          code)
{
    SymHeap &sh = proc.sh();
    TStorRef &stor = sh.stor();

    // read pointer's sizeof from Code Storage
    TSizeOf ptrSize = 0;
    switch (code) {
        case PK_DATA:
            ptrSize = stor.types.dataPtrSizeof();
            break;

        case PK_CODE:
            ptrSize = stor.types.codePtrSizeof();
            break;
    }

    const TSizeOf dstSize = dst.objType()->size;
    CL_BREAK_IF((sh.valSizeOfTarget(dst.placedAt()).lo < dstSize));
    if (ptrSize <= dstSize)
        return val;

    const struct cl_loc *loc = proc.lw();
    CL_ERROR_MSG(loc, "not enough space to store value of a pointer");
    CL_NOTE_MSG(loc, "dstSize: " << dstSize << " B");
    CL_NOTE_MSG(loc, "ptrSize: " << ptrSize << " B");
    proc.printBackTrace(ML_ERROR);
    return sh.valCreate(VT_UNKNOWN, VO_REINTERPRET);
}

TValId ptrObjectEncoder(SymProc &proc, const ObjHandle &dst, TValId val) {
    return ptrObjectEncoderCore(proc, dst, val, PK_DATA);
}

// FIXME: the following code is full of off-by-one errors and integral overflows
// TODO: allow overflow detection for target types >= sizeof(long) on host
TValId integralEncoder(
        SymProc                    &proc,
        const ObjHandle            &dst,
        const TValId                val,
        const IR::Range            &rngOrig)
{
    const struct cl_loc *loc = proc.lw();

    // read type-info of the target object
    const TObjType clt = dst.objType();
    const TSizeOf size = clt->size;
    CL_BREAK_IF(isComposite(clt) || !size);

    if (static_cast<TSizeOf>(sizeof(IR::TInt)) <= size)
        // the program being analyzed uses wider integral type than we do, there
        // is no chance to catch anything because of poor representation of ints
        return val;

    // start with the given range as a candidate for result
    IR::Range rng(rngOrig);
    const bool isUnsigned = clt->is_unsigned;
    const char *const sig = (isUnsigned) ? "an unsigned" : "a signed";

    // compute the count of bits we have available to store the integral value
    const TSizeOf limitWidth = (size << 3) /* sign bit */ - !isUnsigned;

    if (IR::IntMin != rng.lo && rng.lo < IR::Int0) {
        const IR::TInt loLimit = IR::Int1 << limitWidth;
        if (rng.lo < -loLimit) {
            CL_WARN_MSG(loc, "possible underflow of " << sig << " integer");
            rng = IR::FullRange;
        }
    }

    if (IR::IntMax != rng.hi) {
        const IR::TInt hiLimit = IR::Int1 << limitWidth;
        if (hiLimit <= rng.hi) {
            CL_WARN_MSG(loc, "possible overflow of " << sig << " integer");
            rng = IR::FullRange;
        }
    }

    if (rngOrig == rng)
        // do not create a fresh value to prevent unnecessary information lost
        return val;

    // something has changed, print the backtrace!
    proc.printBackTrace(ML_WARN);

    // wrap the resulting range as a fresh heap value
    const CustomValue cv(rng);
    return proc.sh().valWrapCustom(cv);
}

TValId customValueEncoder(SymProc &proc, const ObjHandle &dst, TValId val) {
    SymHeap &sh = proc.sh();
    const CustomValue cv = sh.valUnwrapCustom(val);
    const ECustomValue code = cv.code();

    switch (code) {
        case CV_FNC:
            return ptrObjectEncoderCore(proc, dst, val, PK_CODE);

        case CV_INT_RANGE:
            return integralEncoder(proc, dst, val, rngFromCustom(cv));

        case CV_REAL:
            CL_DEBUG_MSG(proc.lw(), "floating point numbers are not supported");
            // fall through!

        case CV_STRING:
            return val;

        case CV_INVALID:
            break;
    }

    CL_BREAK_IF("customValueEncoder() got invalid custom value");
    return VAL_INVALID;
}

void objSetAtomicVal(SymProc &proc, const ObjHandle &lhs, TValId rhs) {
    if (!lhs.isValid()) {
        CL_ERROR_MSG(proc.lw(), "invalid L-value");
        proc.printBackTrace(ML_ERROR);
        return;
    }

    SymHeap &sh = proc.sh();
    const EValueTarget codeLhs = sh.valTarget(lhs.placedAt());
    CL_BREAK_IF(!isPossibleToDeref(codeLhs));

    // generic prototype for a value encoder
    TValId (*encode)(SymProc &, const ObjHandle &obj, const TValId val) = 0;

    const EValueTarget codeRhs = sh.valTarget(rhs);
    if (isPossibleToDeref(codeRhs))
        // pointer write validator
        encode = ptrObjectEncoder;

    else if (VT_CUSTOM == codeRhs)
        // custom value write validator
        encode = customValueEncoder;

    if (encode)
        // translate the value using the selected encoder
        rhs = encode(proc, lhs, rhs);

    // update type-info of the root
    digRootTypeInfo(sh, lhs, rhs);

    LeakMonitor lm(sh);
    lm.enter();

    TValSet killedPtrs;
    lhs.setValue(rhs, &killedPtrs);

    if (lm.collectJunkFrom(killedPtrs))
        reportMemLeak(proc, codeLhs, "assign");

    lm.leave();
}

void SymProc::objSetValue(const ObjHandle &lhs, TValId rhs) {
    const TValId lhsAt = lhs.placedAt();
    const EValueTarget code = sh_.valTarget(lhsAt);
    CL_BREAK_IF(!isPossibleToDeref(code));

    const TObjType clt = lhs.objType();
    const bool isComp = isComposite(clt, /* includingArray */ false);
    const TSizeOf size = clt->size;
    CL_BREAK_IF(!size);

    if (VO_DEREF_FAILED == sh_.valOrigin(rhs)) {
        // we're already on an error path
        const TValId tplValue = sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);
        if (isComp)
            sh_.writeUniformBlock(lhsAt, tplValue, size);
        else
            lhs.setValue(tplValue);

        return;
    }

    CL_BREAK_IF(isComp != (VT_COMPOSITE == sh_.valTarget(rhs)));
    if (!isComp) {
        // not a composite object
        objSetAtomicVal(*this, lhs, rhs);
        return;
    }

    // FIXME: Should we check for overlapping?  What does the C standard say??
    const ObjHandle compObj(sh_, sh_.valGetComposite(rhs));
    const TValId rhsAt = compObj.placedAt();

    LeakMonitor lm(sh_);
    lm.enter();

    TValSet killedPtrs;
    sh_.copyBlockOfRawMemory(lhsAt, rhsAt, size, &killedPtrs);

    if (lm.collectJunkFrom(killedPtrs))
        reportMemLeak(*this, code, "assign");

    lm.leave();
}

void SymProc::valDestroyTarget(TValId addr) {
    const EValueTarget code = sh_.valTarget(addr);
    if (VAL_ADDR_OF_RET == addr && isGone(code))
        return;

    LeakMonitor lm(sh_);
    lm.enter();

    if (/* leaking */ lm.destroyRoot(addr))
        reportMemLeak(*this, code, "destroy");

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
    const TValId addr = sh_.addrOfVar(cVar, /* createIfNeeded */ false);
    if (VAL_INVALID == addr)
        // the var is dead already
        return;

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

void execMemsetCore(
        SymHeap                     &sh,
        const TValId                 root,
        const TValId                 valToWrite,
        const IR::Range             &addrRange,
        const IR::Range             &sizeRange,
        const IR::Range             &totalRange,
        TValSet                     *killedPtrs)
{
    const TValId valUnknown = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    const TValId valBegTotal = sh.valByOffset(root, totalRange.lo);

    // how much memory can we guarantee the content of?
    IR::Range safeRange;
    safeRange.alignment = IR::Int1;
    safeRange.lo = addrRange.hi;
    safeRange.hi = addrRange.lo + sizeRange.lo;

    // check whether we are able to write something specific at all
    if (VAL_NULL != valToWrite || safeRange.hi <= safeRange.lo) {
        CL_DEBUG("memset() only invalidates the given range");
        const IR::TInt totalSize = widthOf(totalRange) - /* closed int */ 1;
        sh.writeUniformBlock(valBegTotal, valUnknown, totalSize, killedPtrs);
        return;
    }

    // compute the size we can write precisely
    const IR::TInt safeSize = widthOf(safeRange) - /* closed int */ 1;
    CL_BREAK_IF(safeSize <= 0);

    // valToWrite is VAL_NULL (we do not support writing arbitrary values yet)
    const TValId valBegSafe = sh.valByOffset(root, safeRange.lo);
    sh.writeUniformBlock(valBegSafe, valToWrite, safeSize, killedPtrs);

    // compute size of the prefix we _have_ to invalidate
    const IR::TInt prefixSize = safeRange.lo - totalRange.lo;
    CL_BREAK_IF(prefixSize < 0);
    if (0 < prefixSize) {
        CL_DEBUG("memset() invalidates ambiguous prefix");
        sh.writeUniformBlock(valBegTotal, valUnknown, prefixSize, killedPtrs);
    }

    // compute size of the suffix we _have_ to invalidate
    const IR::TInt suffixSize = totalRange.hi - safeRange.hi;
    CL_BREAK_IF(suffixSize < 0);
    if (0 < suffixSize) {
        CL_DEBUG("memset() invalidates ambiguous suffix");
        const TValId suffixAddr = sh.valByOffset(root, safeRange.hi);
        sh.writeUniformBlock(suffixAddr, valUnknown, suffixSize, killedPtrs);
    }
}

inline void wipeAlignment(IR::Range &rng) {
    CL_BREAK_IF(isAligned(rng));
    rng.alignment = IR::Int1;
}

bool checkForOverlap(
        SymHeap                                     &sh,
        const TValId                                 valDst,
        const TValId                                 valSrc,
        const TSizeOf                                size)
{
    const TValId rootDst = sh.valRoot(valDst);
    const TValId rootSrc = sh.valRoot(valSrc);
    if (sh.proveNeq(rootDst, rootSrc))
        // the roots are proven to be two distinct roots
        return false;

    // TODO
    CL_BREAK_IF("please implement");
    (void) size;
    return true;
}

void executeMemset(
        SymProc                     &proc,
        const TValId                 addr,
        const TValId                 valToWrite,
        const TValId                 valSize)
{
    SymHeap &sh = proc.sh();
    const struct cl_loc *lw = proc.lw();

    // how much we are going to write?
    IR::Range sizeRange;
    if (!rngFromVal(&sizeRange, sh, valSize) || sizeRange.lo < 0) {
        CL_ERROR_MSG(lw, "size arg of memset() is not a known integer");
        proc.printBackTrace(ML_ERROR);
        return;
    }
    if (!sizeRange.hi) {
        CL_WARN_MSG(lw, "ignoring call of memset() with size == 0");
        proc.printBackTrace(ML_WARN);
        return;
    }

    // resolve address range
    IR::Range addrRange = sh.valOffsetRange(addr);

    // how much memory are we going to touch in the worst case?
    const TValId valLimit = sh.valShift(addr, valSize);
    IR::Range totalRange = sh.valOffsetRange(valLimit);
    totalRange.lo = addrRange.lo;

    // check the pointer - is it valid? do we have enough allocated memory?
    const TValId root = sh.valRoot(addr);
    const TValId valBegTotal = sh.valByOffset(root, totalRange.lo);
    if (proc.checkForInvalidDeref(valBegTotal, widthOf(totalRange) - 1)) {
        // error message already printed out
        proc.printBackTrace(ML_ERROR);
        return;
    }

    // enter leak monitor
    LeakMonitor lm(sh);
    lm.enter();
    TValSet killedPtrs;

    wipeAlignment(addrRange);
    wipeAlignment(sizeRange);
    wipeAlignment(totalRange);

    // write the data
    execMemsetCore(sh, root, valToWrite,
            addrRange, sizeRange, totalRange,
            &killedPtrs);

    // check for memory leaks
    if (lm.collectJunkFrom(killedPtrs)) {
        CL_WARN_MSG(lw, "memory leak detected while executing memset()");
        proc.printBackTrace(ML_WARN);
    }

    // leave leak monitor
    lm.leave();
}

void executeMemmove(
        SymProc                     &proc,
        const TValId                 valDst,
        const TValId                 valSrc,
        const TValId                 valSize,
        const bool                   allowOverlap)
{
    SymHeap &sh = proc.sh();
    const struct cl_loc *loc = proc.lw();

    IR::Range size;
    if (!rngFromVal(&size, sh, valSize) || size.lo < 0) {
        CL_ERROR_MSG(loc, "size arg of memmove() is not a known integer");
        proc.printBackTrace(ML_ERROR);
        return;
    }
    if (!size.hi) {
        CL_WARN_MSG(loc, "ignoring call of memcpy()/memmove() with size == 0");
        proc.printBackTrace(ML_WARN);
        return;
    }

    if (proc.checkForInvalidDeref(valDst, size.hi)
            || proc.checkForInvalidDeref(valSrc, size.hi))
    {
        // error message already printed out
        proc.printBackTrace(ML_ERROR);
        return;
    }

    if (!allowOverlap && checkForOverlap(sh, valDst, valSrc, size.hi)) {
        CL_ERROR_MSG(loc, "source and destination overlap in call of memcpy()");
        proc.printBackTrace(ML_ERROR);
        return;
    }

    LeakMonitor lm(sh);
    lm.enter();

    TValSet killedPtrs;
    sh.copyBlockOfRawMemory(valDst, valSrc, size.lo, &killedPtrs);

    if (!isSingular(size)) {
        CL_DEBUG_MSG(loc, "memcpy()/memmove() invalidates ambiguous suffix");
        const TValId suffixAddr = sh.valByOffset(valDst, size.lo);
        const TValId valUnknown = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        const TSizeOf suffWidth = widthOf(size) - /* closed int */ 1;
        sh.writeUniformBlock(suffixAddr, valUnknown, suffWidth, &killedPtrs);
    }

    if (lm.collectJunkFrom(killedPtrs)) {
        CL_WARN_MSG(loc, "memory leak detected while executing memmove()");
        proc.printBackTrace(ML_WARN);
    }

    lm.leave();
}


// /////////////////////////////////////////////////////////////////////////////
// SymExecCore implementation
void SymExecCore::varInit(TValId at) {
    if (ep_.skipVarInit)
        // we are explicitly asked to not initialize any vars
        return;

    if (ep_.trackUninit && VT_ON_STACK == sh_.valTarget(at)) {
        // uninitialized stack variable
        const TValId tpl = sh_.valCreate(VT_UNKNOWN, VO_STACK);
        const TSizeRange size = sh_.valSizeOfTarget(at);
        CL_BREAK_IF(!isSingular(size));
        sh_.writeUniformBlock(at, tpl, size.lo);
    }

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
            this->printBackTrace(ML_ERROR);
            return;

        case VT_LOST:
            CL_ERROR_MSG(lw_, "attempt to free a non-existing non-heap object");
            this->printBackTrace(ML_ERROR);
            return;

        case VT_STATIC:
        case VT_ON_STACK:
            CL_ERROR_MSG(lw_, "attempt to free a non-heap object");
            this->printBackTrace(ML_ERROR);
            return;

        case VT_CUSTOM:
            CL_ERROR_MSG(lw_, "free() called on non-pointer value");
            this->printBackTrace(ML_ERROR);
            return;

        case VT_RANGE:
            CL_BREAK_IF("please implement");
            // fall through!

        case VT_INVALID:
        case VT_ABSTRACT:
        case VT_COMPOSITE:
            CL_BREAK_IF("invalid call of SymExecCore::execFree()");
            // fall through!

        case VT_UNKNOWN:
            if (VO_DEREF_FAILED == sh_.valOrigin(val))
                return;

            CL_ERROR_MSG(lw_, "invalid free()");
            describeUnknownVal(*this, val, "free");
            this->printBackTrace(ML_ERROR);
            return;

        case VT_ON_HEAP:
            break;
    }

    const TOffset off = sh_.valOffset(val);
    if (off) {
        CL_ERROR_MSG(lw_, "free() called with offset " << off << "B");
        this->printBackTrace(ML_ERROR);
        return;
    }

    CL_DEBUG_MSG(lw_, "executing free()");
    this->valDestroyTarget(val);
}

void SymExecCore::execHeapAlloc(
        SymState                        &dst,
        const CodeStorage::Insn         &insn,
        const TSizeRange                size,
        const bool                      nullified)
{
    // resolve lhs
    const ObjHandle lhs = this->objByOperand(insn.operands[/* dst */ 0]);
    if (OBJ_DEREF_FAILED == lhs.objId())
        // error alredy emitted
        return;

    if (!size.hi) {
        CL_WARN_MSG(lw_, "POSIX says that, given zero size, the behavior of \
malloc/calloc is implementation-defined");
        CL_NOTE_MSG(lw_, "assuming NULL as the result");
        this->printBackTrace(ML_WARN);
        this->objSetValue(lhs, VAL_NULL);
        this->killInsn(insn);
        dst.insert(sh_);
        return;
    }

    if (ep_.oomSimulation) {
        // clone the heap and core
        SymHeap oomHeap(sh_);
        SymExecCore oomCore(oomHeap, bt_, ep_);
        oomCore.setLocation(lw_);
        Trace::waiveCloneOperation(oomHeap);

        // OOM state simulation
        const ObjHandle oomLhs(oomHeap, lhs);
        oomCore.objSetValue(oomLhs, VAL_NULL);
        oomCore.killInsn(insn);
        dst.insert(oomHeap);
    }

    // now create a heap object
    const TValId val = sh_.heapAlloc(size);

    if (nullified) {
        // initialize to zero as we are doing calloc()
        sh_.writeUniformBlock(val, VAL_NULL, size.lo);
    }
    else if (ep_.trackUninit) {
        // uninitialized heap block
        const TValId tplValue = sh_.valCreate(VT_UNKNOWN, VO_HEAP);
        sh_.writeUniformBlock(val, tplValue, size.lo);
    }

    // store the result of malloc
    this->objSetValue(lhs, val);
    this->killInsn(insn);
    dst.insert(sh_);
}

bool describeCmpOp(CmpOpTraits *pTraits, const enum cl_binop_e code) {
    memset(pTraits, 0, sizeof *pTraits);

    switch (code) {
        case CL_BINOP_LE:
        case CL_BINOP_LT:
            pTraits->leftToRight = true;
            break;

        case CL_BINOP_GE:
        case CL_BINOP_GT:
            pTraits->rightToLeft = true;
            break;

        default:
            // no ordering traits here...
            break;
    }

    switch (code) {
        case CL_BINOP_EQ:
            pTraits->preserveEq = true;
            // fall through!

        case CL_BINOP_LE:
        case CL_BINOP_GE:
            pTraits->preserveNeq = true;
            return true;

        case CL_BINOP_NE:
            pTraits->preserveEq = true;
            // fall through!

        case CL_BINOP_LT:
        case CL_BINOP_GT:
            pTraits->negative = true;
            pTraits->preserveNeq = true;
            return true;

        default:
            // unhandled binary operator
            return false;
    }
}

TValId comparePointers(
        SymHeap                     &sh,
        const enum cl_binop_e       code,
        const TValId                v1,
        const TValId                v2)
{
    const TValId root1 = sh.valRoot(v1);
    const TValId root2 = sh.valRoot(v2);
    if (root1 != root2)
        // TODO: not much info, but we can still deduce at least something
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);

    bool result;
    const IR::Range range1 = sh.valOffsetRange(v1);
    const IR::Range range2 = sh.valOffsetRange(v2);
    if (compareIntRanges(&result, code, range1, range2))
        return boolToVal(result);

#if !SE_ALLOW_OFF_RANGES
    CL_BREAK_IF("please implement");
#endif
    return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
}

TValId compareValues(
        SymHeap                     &sh,
        const enum cl_binop_e       code,
        const TValId                v1,
        const TValId                v2)
{
    const EValueOrigin vo1 = sh.valOrigin(v1);
    const EValueOrigin vo2 = sh.valOrigin(v2);
    if (VO_DEREF_FAILED == vo1 || VO_DEREF_FAILED == vo2)
        return sh.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

    // resolve binary operator
    CmpOpTraits cTraits;
    if (!describeCmpOp(&cTraits, code)) {
        CL_WARN("binary operator not implemented yet");
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    // inconsistency check
    const bool neg = cTraits.negative;
    if ((v1 == v2) && cTraits.preserveNeq)
        return boolToVal(!neg);
    if (sh.proveNeq(v1, v2) && cTraits.preserveEq)
        return boolToVal(neg);

    const EValueTarget code1 = sh.valTarget(v1);
    const EValueTarget code2 = sh.valTarget(v2);
    if (isAnyDataArea(code1) && isAnyDataArea(code2))
        // both values are pointers
        return comparePointers(sh, code, v1, v2);

    IR::Range rng1, rng2;
    if (rngFromVal(&rng1, sh, v1) && rngFromVal(&rng2, sh, v2)) {
        // both values are integral constants
        bool result;
        if (compareIntRanges(&result, code, rng1, rng2))
            return boolToVal(result);
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

bool trimRangesIfPossible(
        SymHeap                     &sh,
        const CmpOpTraits           &cTraits,
        const bool                  branch,
        const TValId                v1,
        const TValId                v2)
{
    IR::Range rng1;
    if (!anyRangeFromVal(&rng1, sh, v1))
        return false;

    IR::Range rng2;
    if (!anyRangeFromVal(&rng2, sh, v2))
        return false;

    const bool isRange1 = !isSingular(rng1);
    const bool isRange2 = !isSingular(rng2);
    if (isRange1 == isRange2)
        // not a suitable value pair for trimming
        return false;

    const bool ltr = cTraits.leftToRight;
    const bool rtl = cTraits.rightToLeft;
    CL_BREAK_IF(ltr && rtl);

    // use the offsets in the appropriate order
    IR::Range win        = (isRange1) ? rng1    : rng2;
    const IR::TInt limit = (isRange2) ? rng1.lo : rng2.lo;

    // which boundary are we going to trim?
    bool trimLo;
    if (ltr || rtl) {
        const bool neg = (branch == isRange2);
        trimLo = (neg == ltr);
    }
    else {
        if (!cTraits.preserveEq || !cTraits.preserveNeq)
            // not a suitable binary operator
            return false;

        if (limit == win.lo)
            // cut off a single integer from the lower bound
            trimLo = true;
        else if (limit == win.hi)
            // cut off a single integer from the upper bound
            trimLo = false;
        else
            // no luck...
            return false;
    }

    // one of the values holds a range inside
    const TValId valRange = (isRange1) ? v1 : v2;

    const EValueTarget code = sh.valTarget(valRange);
    if (VT_RANGE == code) {
        // comparing the offsets makes sense only if both values shares the root
        const TValId root1 = sh.valRoot(v1);
        const TValId root2 = sh.valRoot(v2);
        if (root1 != root2)
            // root mismatch
            return false;
    }

    CL_DEBUG("trimRangesIfPossible() is taking place...");

    // should we include the boundary to the result?
    const bool isOpen = (branch == cTraits.negative);

    if (trimLo)
        // shift the lower bound up
        win.lo = limit + isOpen;
    else
        // shift the upper bound down
        win.hi = limit - isOpen;

    // trim the designated VT_RANGE value
    sh.valRestrictRange(valRange, win);
    return true;
}

bool reflectCmpResult(
        SymProc                    &proc,
        const enum cl_binop_e       code,
        const bool                  branch,
        const TValId                v1,
        const TValId                v2)
{
    SymHeap &sh = proc.sh();
    CL_BREAK_IF(!protoCheckConsistency(sh));

    // resolve binary operator
    CmpOpTraits cTraits;
    if (!describeCmpOp(&cTraits, code))
        return false;

    if (trimRangesIfPossible(sh, cTraits, branch, v1, v2))
        return true;

    if (branch == cTraits.negative) {
        if (!cTraits.preserveNeq)
            return false;

        // introduce a Neq predicate over v1 and v2
        sh.neqOp(SymHeap::NEQ_ADD, v1, v2);
    }
    else {
        if (!cTraits.preserveEq)
            return false;

        // we have deduced that v1 and v2 is actually the same value

        LeakMonitor lm(sh);
        lm.enter();

        TValList leakList;
        sh.valMerge(v1, v2, &leakList);

        if (lm.importLeakList(&leakList)) {
            const struct cl_loc *loc = proc.lw();
            CL_WARN_MSG(loc, "memory leak detected while removing a segment");
            proc.printBackTrace(ML_WARN);
        }

        lm.leave();
    }

    CL_BREAK_IF(!protoCheckConsistency(sh));
    return true;
}

bool computeIntRngResult(
        TValId                      *pDst,
        SymHeapCore                 &sh,
        const enum cl_binop_e       code,
        const IR::Range             rng1,
        const IR::Range             rng2)
{
    // compute the result of an integral binary operation
    IR::Range result;
    result.alignment = IR::Int1;

    switch (code) {
        case CL_BINOP_PLUS:
            result = rng1 + rng2;
            break;

        case CL_BINOP_MINUS:
            result = rng1 - rng2;
            break;

        case CL_BINOP_BIT_AND:
            if (!isSingular(rng1) || !isSingular(rng2))
                return false;

            result = IR::rngFromNum(rng1.lo & rng2.lo);
            break;

        case CL_BINOP_LSHIFT:
            if (!isSingular(rng2))
                return false;

            result = rng1 << rng2.lo;
            break;

        case CL_BINOP_RSHIFT:
            if (!isSingular(rng2))
                return false;

            result = rng1 >> rng2.lo;
            break;

        case CL_BINOP_MULT:
            result = rng1 * rng2;
            break;

        case CL_BINOP_MIN:
            result.lo = std::min(rng1.lo, rng2.lo);
            result.hi = std::min(rng1.hi, rng2.hi);
            break;

        case CL_BINOP_MAX:
            result.lo = std::max(rng1.lo, rng2.lo);
            result.hi = std::max(rng1.hi, rng2.hi);
            break;

        default:
            CL_BREAK_IF("unhandled binary integral operation");
            return false;
    }

    // wrap the result as a heap value
    const CustomValue cv(result);
    *pDst = sh.valWrapCustom(cv);
    return true;
}

/// if v1 is scalar and v2 is range (or vice versa), call valShift() on them
inline bool handleRangeByScalarPlus(
        TValId                     *pResult,
        SymHeapCore                &sh,
        const TValId                v1,
        const TValId                v2,
        const IR::Range            &rng1,
        const IR::Range            &rng2)
{
    bool isRange1;
    if (!isRangeByNum(&isRange1, rng1, rng2))
        return false;

    if (isRange1)
        *pResult = sh.valShift(v1, v2);
    else
        *pResult = sh.valShift(v2, v1);

    return true;
}

bool handleRangeBitMask(
        TValId                     *pResult,
        SymHeapCore                &sh,
        const TValId                v1,
        const TValId                v2,
        const IR::Range            &rng1,
        const IR::Range            &rng2)
{
    bool isRange1;
    if (!isRangeByNum(&isRange1, rng1, rng2))
        return false;

    IR::TInt mask;
    const TValId valMask  = (isRange1) ?   v2 :   v1;
    if (!numFromVal(&mask, sh, valMask))
        CL_BREAK_IF("handleRangeBitMask() malfunction");

    IR::Range rng = (isRange1) ? rng1 : rng2;
    rng &= mask;

    const CustomValue cv(rng);
    *pResult = sh.valWrapCustom(cv);
    return true;
}

TValId handleIntegralOp(
        SymHeapCore                &sh,
        const TValId                v1,
        const TValId                v2,
        const enum cl_binop_e       code)
{
    if (CL_BINOP_MINUS == code) {
        // chances are this could be a pointer difference
        const EValueTarget code1 = sh.valTarget(v1);
        const EValueTarget code2 = sh.valTarget(v2);

        if (isAnyDataArea(code1) && isAnyDataArea(code2))
            // ... and it indeed is a pointer difference
            return sh.diffPointers(v1, v2);
    }

    TValId result;

    // check whether both values are integral constant
    IR::Range rng1, rng2;
    if (rngFromVal(&rng1, sh, v1) && rngFromVal(&rng2, sh, v2)) {

        // first try to preserve range coincidence if we can
        switch (code) {
            case CL_BINOP_BIT_AND:
                if (handleRangeBitMask(&result, sh, v1, v2, rng1, rng2))
                    return result;
                break;

            case CL_BINOP_PLUS:
                if (handleRangeByScalarPlus(&result, sh, v1, v2, rng1, rng2))
                    return result;
                break;

            case CL_BINOP_MINUS:
                if (!isSingular(rng1) && isSingular(rng2))
                    return sh.valByOffset(v1, -rng2.lo);
                break;

            default:
                break;
        }

        if (computeIntRngResult(&result, sh, code, rng1, rng2))
            return result;
    }

    return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
}

TValId handleBitNot(SymHeapCore &sh, const TValId val) {
    // check whether the value is an integral constant
    IR::TInt num;
    if (!numFromVal(&num, sh, val))
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);

    // compute the integral result
    const IR::TInt result = ~num;

    // wrap the result as a heap value expressing a constant integer
    CustomValue cv(IR::rngFromNum(result));
    return sh.valWrapCustom(cv);
}

TValId handleIntegralOp(
        SymHeapCore                 &sh,
        const TValId                val,
        const enum cl_unop_e        code)
{
    switch (code) {
        case CL_UNOP_MINUS:
            return handleIntegralOp(sh, VAL_NULL, val, CL_BINOP_MINUS);

        case CL_UNOP_BIT_NOT:
            return handleBitNot(sh, val);

        default:
            CL_BREAK_IF("unhandled unary integral operation");
            return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }
}

bool isAnyIntValue(const SymHeapCore &sh, const TValId val) {
    const TValId root = sh.valRoot(val);
    switch (root) {
        case VAL_NULL:
        case VAL_TRUE:
            return true;

        default:
            if (VT_CUSTOM == sh.valTarget(val))
                break;

            return false;
    }

    const CustomValue &cv = sh.valUnwrapCustom(val);
    const ECustomValue code = cv.code();
    switch (code) {
        case CV_INT_RANGE:
            return true;

        default:
            return false;
    }
}

TValId handlePtrBitAnd(
        SymHeapCore                &sh,
        const TValId                vPtr,
        const TValId                vInt)
{
    IR::TInt mask;
    if (!numFromVal(&mask, sh, vInt) || 0 < mask)
        // giving up
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);

    if (!mask)
        // the whole pointer has been masked
        return VAL_NULL;

    // include all possible scenarios into consideration
    IR::Range range;
    range.lo        = IR::Int1 + mask;
    range.hi        = IR::Int0;
    range.alignment = IR::Int1;

    // create the appropriate VT_RANGE value
    return sh.valByRange(vPtr, range);
}

TValId handlePtrOperator(
        SymHeapCore                &sh,
        const TValId                vPtr,
        TValId                      vInt,
        const enum cl_binop_e       code)
{
    switch (code) {
        case CL_BINOP_MINUS:
            vInt = handleIntegralOp(sh, vInt, CL_UNOP_MINUS);
            // fall through!

        case CL_BINOP_PLUS:
            return sh.valShift(vPtr, vInt);

        case CL_BINOP_BIT_AND:
            return handlePtrBitAnd(sh, vPtr, vInt);

        default:
            CL_BREAK_IF("unhandled binary operator in handlePtrOperator()");
    }

    return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
}

/// ptr arithmetic is sometimes (with CIL always) masked as integral arithmetic
bool reconstructPtrArithmetic(
        TValId                     *pResult,
        SymHeapCore                &sh,
        const TValId                v1,
        const TValId                v2,
        const enum cl_binop_e       code)
{
    // these are no-ops (and I would bet they come from CIL anyway)
    if (VAL_NULL == v1) {
        *pResult =  v2;
        return true;
    }
    if (VAL_NULL == v2) {
        *pResult =  v1;
        return true;
    }

    if (isAnyDataArea(sh.valTarget(v1)) && isAnyIntValue(sh, v2)) {
        CL_DEBUG("integral operator applied on ptr handled as ptr operator...");
        *pResult = handlePtrOperator(sh, /* vPtr */ v1, /* vInt */ v2, code);
        return true;
    }

    if (isAnyDataArea(sh.valTarget(v2)) && isAnyIntValue(sh, v1)) {
        if (CL_BINOP_MINUS == code)
            // CL_BINOP_MINUS makes no sense here, it would mean e.g. (4 - &foo)
            return false;

        CL_DEBUG("integral operator applied on ptr handled as ptr operator...");
        *pResult = handlePtrOperator(sh, /* vPtr */ v2, /* vInt */ v1, code);
        return true;
    }

    // looks like a real integral operation
    return false;
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
        SymHeap &sh = proc.sh();
        const TValId val = rhs[0];

        const enum cl_unop_e code = static_cast<enum cl_unop_e>(iCode);
        switch (code) {
            case CL_UNOP_BIT_NOT:
                if (!clt[0] || CL_TYPE_BOOL != clt[0]->code
                        || !clt[1] || CL_TYPE_BOOL != clt[1]->code)
                    return handleIntegralOp(sh, val, code);
                // gcc 4.7.x uses CL_UNOP_BIT_NOT for bools with truth semantics
                // fall through!

            case CL_UNOP_TRUTH_NOT:
                return compareValues(sh, CL_BINOP_EQ, VAL_FALSE, val);

            case CL_UNOP_MINUS:
                return handleIntegralOp(sh, rhs[0], code);

            case CL_UNOP_ASSIGN:
                return val;

            default:
                // over-approximate anything else
                goto unknown_result;
        }

unknown_result:
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
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
        SymHeap &sh = proc.sh();
        TValId vRes;

        CL_BREAK_IF(!clt[0] || !clt[1] || !clt[2]);
        (void) clt;

        const enum cl_binop_e code = static_cast<enum cl_binop_e>(iCode);
        switch (code) {
            case CL_BINOP_EQ:
            case CL_BINOP_NE:
            case CL_BINOP_LT:
            case CL_BINOP_GT:
            case CL_BINOP_LE:
            case CL_BINOP_GE:
                CL_BREAK_IF(!areComparableTypes(clt[0], clt[1]));
                return compareValues(sh, code, rhs[0], rhs[1]);

            case CL_BINOP_MULT:
                if (VAL_NULL == rhs[0] || VAL_NULL == rhs[1])
                    // whatever we got as the second operand, the result is zero
                    return VAL_NULL;

                // fall through!

            case CL_BINOP_MIN:
            case CL_BINOP_MAX:
            case CL_BINOP_LSHIFT:
            case CL_BINOP_RSHIFT:
                goto handle_int;

            case CL_BINOP_BIT_AND:
                if (VAL_NULL == rhs[0] || VAL_NULL == rhs[1])
                    // whatever we got as the second operand, the result is zero
                    return VAL_NULL;

                // fall through!

            case CL_BINOP_PLUS:
            case CL_BINOP_MINUS:
                if (reconstructPtrArithmetic(&vRes, sh, rhs[0], rhs[1], code))
                    return vRes;
                else
                    goto handle_int;

            case CL_BINOP_POINTER_PLUS:
                return sh.valShift(rhs[0], rhs[1]);

            default:
                // over-approximate anything else
                return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        }

handle_int:
        return handleIntegralOp(sh, rhs[0], rhs[1], code);
    }
};

template <int ARITY>
void SymExecCore::execOp(const CodeStorage::Insn &insn) {
    // resolve lhs
    const struct cl_operand &dst = insn.operands[/* dst */ 0];
    const ObjHandle lhs = this->objByOperand(dst);
    if (OBJ_DEREF_FAILED == lhs.objId())
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
            lhs.setValue(vFail);
            return;
        }

        rhs[i] = val;
    }

    const TValId valResult =
        OpHandler<ARITY>::handleOp(*this, insn.subCode, rhs, clt);

#if SE_TRACK_NON_POINTER_VALUES < 2
    // avoid creation of live object in case we are not interested in its value
    if (!isDataPtr(dst.type) && VO_UNKNOWN == sh_.valOrigin(valResult)) {
        const TValId root = sh_.valRoot(lhs.placedAt());

        ObjList liveObjs;
        sh_.gatherLiveObjects(liveObjs, root);
        BOOST_FOREACH(const ObjHandle &obj, liveObjs)
            if (obj == lhs)
                goto already_alive;

        return;
    }
already_alive:
#endif
    // store the result
    this->objSetValue(lhs, valResult);
}

void SymExecCore::handleLabel(const CodeStorage::Insn &insn) {
    const struct cl_operand &op = insn.operands[/* name */ 0];
    if (CL_OPERAND_VOID == op.code)
        // anonymous label
        return;

    const std::string &errLabel = ep_.errLabel;
    if (errLabel.empty())
        // we are not looking for error labels, keep going...
        return;

    // resolve name
    CL_BREAK_IF(CL_OPERAND_CST != op.code);
    const struct cl_cst &cst = op.data.cst;
    CL_BREAK_IF(CL_TYPE_STRING != cst.code);
    const char *name = cst.data.cst_string.value;
    CL_BREAK_IF(!name);

    if (ep_.errLabel.compare(name))
        // not an error label
        return;

    CL_ERROR_MSG(lw_, "error label \"" << name << "\" has been reached");

    // print the backtrace and leave
    this->printBackTrace(ML_ERROR, /* forcePtrace */ true);
    printMemUsage("SymBackTrace::printBackTrace");
    throw std::runtime_error("an error label has been reached");
}

bool SymExecCore::execCore(
        SymState                    &dst,
        const CodeStorage::Insn     &insn)
{
    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            this->execOp<1>(insn);
            break;

        case CL_INSN_BINOP:
            this->execOp<2>(insn);
            break;

        case CL_INSN_LABEL:
            this->handleLabel(insn);
            break;

        case CL_INSN_CALL:
            // the symbin module is now fully responsible for handling built-ins
            return handleBuiltIn(dst, *this, insn);

        default:
            CL_BREAK_IF("SymExecCore::execCore() got an unexpected insn");
            return false;
    }

    if (this->hasFatalError())
        // do not insert anything into dst
        return true;

    // kill variables
    this->killInsn(insn);

    Trace::Node *trOrig = sh_.traceNode();
    Trace::Node *trInsn = new Trace::InsnNode(trOrig, &insn, /* bin */ false);
    sh_.traceUpdate(trInsn);
    dst.insert(sh_);
    return true;
}

template <class TDerefs>
bool SymExecCore::concretizeLoop(
        SymState                    &dst,
        const CodeStorage::Insn     &insn,
        const TDerefs               &derefs)
{
#ifndef NDEBUG
    bool hit = false;
#endif
    TSymHeapList todo;
    todo.push_back(sh_);
    Trace::waiveCloneOperation(todo.back());

    while (!todo.empty()) {
        SymHeap &sh = todo.front();
        SymExecCore slave(sh, bt_, ep_);
        slave.setLocation(lw_);

#ifndef NDEBUG
        bool hitLocal = false;
#endif
        BOOST_FOREACH(unsigned idx, derefs) {
            const struct cl_operand &op = insn.operands.at(idx);
            if (CL_OPERAND_VAR != op.code)
                // literals cannot be abstract
                continue;

            const TValId at = slave.varAt(op);
            if (!canWriteDataPtrAt(sh, at))
                continue;

            // we expect a pointer at this point
            const TValId val = valOfPtrAt(sh, at);
            if (VT_ABSTRACT == sh.valTarget(val)) {
#ifndef NDEBUG
                CL_BREAK_IF(hitLocal);
                hitLocal = true;
                hit = true;
#endif
                LeakMonitor lm(sh);
                lm.enter();

                TValList leakList;
                concretizeObj(sh, val, todo, &leakList);

                if (lm.importLeakList(&leakList)) {
                    CL_WARN_MSG(lw_, "memory leak detected while unfolding");
                    this->printBackTrace(ML_WARN);
                }

                lm.leave();
            }
        }

        // process the current heap and move to the next one (if any)
        if (!slave.execCore(dst, insn)) {
            // hit a real function call, this has to be handled by SymExec
            CL_BREAK_IF(CL_INSN_CALL != insn.code);

            // FIXME: are we ready for this?
            CL_BREAK_IF(hit);
            return false;
        }

        if (slave.errorDetected_)
            // propagate the 'error detected' flag
            errorDetected_ = true;

        todo.pop_front();
    }

    return true;
}

bool SymExecCore::exec(SymState &dst, const CodeStorage::Insn &insn) {
    TOpIdxList derefs;

    const cl_insn_e code = insn.code;
    if (CL_INSN_CALL == code)
        // certain built-ins dereference certain operands (free, memset, ...)
        derefs = opsWithDerefSemanticsInCallInsn(*this, insn);

    // look for explicit dereferences in operands of the instruction
    const CodeStorage::TOperandList &opList = insn.operands;
    for (unsigned idx = 0; idx < opList.size(); ++idx) {
        const struct cl_accessor *ac = opList[idx].accessor;
        if (!ac)
            continue;

        // we expect the dereference only as the first accessor
        const enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_DEREF:
            case CL_ACCESSOR_DEREF_ARRAY:
                break;

            default:
                // no dereference in this operand
                continue;
        }

        if (seekRefAccessor(ac))
            // not a dereference, only an address is being computed
            continue;

        derefs.push_back(idx);
    }

    if (derefs.empty())
        return this->execCore(dst, insn);

    // handle dereferences
    return this->concretizeLoop(dst, insn, derefs);
}
