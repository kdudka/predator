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
#include <cl/memdebug.hh>
#include <cl/storage.hh>

#include "glconf.hh"
#include "prototype.hh"
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
void SymProc::printBackTrace(EMsgLevel level, bool forcePtrace)
{
    // update trace graph
    Trace::MsgNode *trMsg = new Trace::MsgNode(sh_.traceNode(), level, lw_);
    sh_.traceUpdate(trMsg);
    CL_BREAK_IF(!chkTraceGraphConsistency(trMsg));

    // print the backtrace
    if (forcePtrace) {
        Trace::printTrace(trMsg);
        printMemUsage("Trace::printTrace");
    }
    else if (bt_->printBackTrace())
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

    if (GlConf::data.errorRecoveryMode)
        errorDetected_ = true;
    else
        throw std::runtime_error("error recovery is disabled");
}

bool SymProc::hasFatalError() const
{
    return (GlConf::data.errorRecoveryMode < 2)
        && errorDetected_;
}

TValId SymProc::valFromCst(const struct cl_operand &op)
{
    const struct cl_cst &cst = op.data.cst;

    CustomValue cv;

    const cl_type_e code = cst.code;
    switch (code) {
        case CL_TYPE_ENUM:
        case CL_TYPE_INT:
            // integral value
            cv = CustomValue(IR::rngFromNum(
                        /* FIXME: deal better with integer literals */
                        static_cast<int>(cst.data.cst_int.value)));
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

const char* describeObj(const EStorageClass code)
{
    switch (code) {
        case SC_STATIC:
            return "a static variable";

        case SC_ON_STACK:
            return "a variable on stack";

        case SC_ON_HEAP:
            return "a heap object";

        default:
            CL_BREAK_IF("unhandled storage class in describeObj()");
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
    const TObjId obj = sh.objByAddr(val);
    const EStorageClass code = sh.objStorClass(obj);
    const char *const what = describeObj(code);

    const TSizeRange rootSizeRange = sh.objSize(obj);
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

    const TSizeRange dstSizeRange = valSizeOfTarget(sh, val);
    const TSizeOf minDstSize = dstSizeRange.lo;
    beyond = sizeOfTarget - minDstSize;
    if (0 < beyond) {
        CL_NOTE_MSG(loc, "the target object ends "
                << beyond << "B beyond " << what
                << " of size " << minRootSize << "B");
    }
}

bool SymProc::checkForInvalidDeref(TValId val, const TSizeOf sizeOfTarget)
{
    if (VAL_NULL == val) {
        CL_ERROR_MSG(lw_, "dereference of NULL value");
        return true;
    }

    const TObjId obj = sh_.objByAddr(val);
    if (OBJ_NULL == obj) {
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
            CL_BREAK_IF("attempt to dereference something special");
            // fall through!

        case VT_CUSTOM:
        case VT_UNKNOWN:
            CL_ERROR_MSG(lw_, "invalid dereference");
            describeUnknownVal(*this, val, "dereference");
            return true;

        case VT_OBJECT:
            break;
    }

    if (!sh_.isValid(obj)) {
        const EStorageClass sc = sh_.objStorClass(obj);
        switch (sc) {
            default:
                CL_BREAK_IF("checkForInvalidDeref() got something special");
                // fall through!

            case SC_ON_STACK:
                CL_ERROR_MSG(lw_,"dereference of non-existing non-heap object");
                break;

            case SC_ON_HEAP:
                CL_ERROR_MSG(lw_, "dereference of already deleted heap object");
                break;
        }

        return true;
    }

    const TSizeRange dstSizeRange = valSizeOfTarget(sh_, val);
    const TSizeOf minDstSize = dstSizeRange.lo;
    if (sh_.valOffset(val) < 0 || minDstSize < sizeOfTarget) {
        // out of bounds
        reportDerefOutOfBounds(*this, val, sizeOfTarget);
        return true;
    }

    // all OK
    return false;
}

void SymProc::varInit(TObjId obj)
{
    const CVar cv = sh_.cVarByObject(obj);
    const CodeStorage::Storage &stor = sh_.stor();
    const CodeStorage::Var &var = stor.vars[cv.uid];
    if (var.initials.empty())
        // nothing to do at this level (already handled by SymExecCore)
        return;

    SymExecCore core(sh_, bt_);
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

TObjId SymProc::objByVar(const CVar &cv)
{
    TObjId reg = sh_.regionByVar(cv, /* createIfNeeded */ false);
    if (OBJ_INVALID != reg)
        // var already alive
        return reg;

    // lazy var creation
    reg = sh_.regionByVar(cv, /* createIfNeeded */ true);

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
        return reg;
#endif

    bool needInit = !var.initials.empty();
    if (nullify) {
        // initialize to zero
        const TSizeRange size = sh_.objSize(reg);
        CL_BREAK_IF(!isSingular(size));

        const UniformBlock ub = {
            /* off      */ 0,
            /* size     */ size.lo,
            /* tplValue */ VAL_NULL
        };

        sh_.writeUniformBlock(reg, ub);
    }
    else if (isLcVar)
        needInit = true;

    if (needInit)
        // go through explicit initializers
        this->varInit(reg);

    return reg;
}

TObjId SymProc::objByVar(const struct cl_operand &op)
{
    // resolve CVar
    const int uid = varIdFromOperand(&op);
    const int nestLevel = bt_->countOccurrencesOfTopFnc();
    const CVar cv(uid, nestLevel);
    return this->objByVar(cv);
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

TOffset offItem(const struct cl_accessor *ac)
{
    const int id = ac->data.item.id;
    const TObjType clt = ac->type;
    CL_BREAK_IF(!clt || clt->item_cnt <= id);

    return clt->items[id].offset;
}

TValId SymProc::targetAt(const struct cl_operand &op)
{
    // resolve program variable
    const TObjId obj = this->objByVar(op);
    TValId addr = sh_.addrOfTarget(obj, TS_REGION);

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
        const PtrHandle ptr(sh_, obj);
        addr = ptr.value();
    }

    // apply the offset
    return sh_.valByOffset(addr, off);
}

FldHandle SymProc::fldByOperand(const struct cl_operand &op)
{
    CL_BREAK_IF(seekRefAccessor(op.accessor));

    // resolve address of the target object
    const TValId at = this->targetAt(op);
    const EValueOrigin origin = sh_.valOrigin(at);
    if (VO_DEREF_FAILED == origin)
        // we are already on the error path
        return FldHandle(FLD_DEREF_FAILED);

    // check for invalid dereference
    const TObjType cltTarget = op.type;
    if (this->checkForInvalidDeref(at, cltTarget->size)) {
        this->printBackTrace(ML_ERROR);
        return FldHandle(FLD_DEREF_FAILED);
    }

    // resolve the target object
    const TObjId obj = sh_.objByAddr(at);
    const TOffset off = sh_.valOffset(at);
    const FldHandle fld(sh_, obj, op.type, off);
    if (!fld.isValidHandle())
        CL_BREAK_IF("SymProc::fldByOperand() failed to resolve an object");

    // all OK
    return fld;
}

TValId SymProc::valFromObj(const struct cl_operand &op)
{
    if (seekRefAccessor(op.accessor))
        return this->targetAt(op);

    const FldHandle handle = this->fldByOperand(op);
    if (handle.isValidHandle())
        return handle.value();

    // failed to resolve object handle
    const TFldId fld = handle.fieldId();
    switch (fld) {
        case FLD_UNKNOWN:
            return sh_.valCreate(VT_UNKNOWN, VO_REINTERPRET);

        case FLD_DEREF_FAILED:
            return sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

        default:
            return VAL_INVALID;
    }
}

TValId SymProc::valFromOperand(const struct cl_operand &op)
{
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

bool SymProc::fncFromOperand(int *pUid, const struct cl_operand &op)
{
    if (fncUidFromOperand(pUid, &op))
        return true;

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

void digRootTypeInfo(SymHeap &sh, const FldHandle &lhs, TValId rhs)
{
    if (!isPossibleToDeref(sh, rhs))
        // no valid target anyway
        return;

    if (sh.valOffset(rhs))
        // not a pointer to root
        return;

    const TObjType cltTarget = targetTypeOfPtr(lhs.type());
    if (!cltTarget || CL_TYPE_VOID == cltTarget->code)
        // no type-info given for the target
        return;

    const TObjId rhsTarget = sh.objByAddr(rhs);
    const TObjType cltLast = sh.objEstimatedType(rhsTarget);
    if (isComposite(cltLast) && !isComposite(cltTarget))
        // we are accessing a field that is placed at zero offset of a composite
        // type but it yet does not mean that we are changing the root type-info
        return;

    const TSizeRange rootSizeRange = sh.objSize(rhsTarget);
    const TSizeOf rootSize = rootSizeRange.lo;
    CL_BREAK_IF(rootSize <= 0 && isOnHeap(sh.objStorClass(rhsTarget)));

    if (cltLast && cltLast->size == rootSize && cltTarget->size != rootSize)
        // basically the same rule as above but now we check the size of target
        return;

    // update the last known type-info of the root
    sh.objSetEstimatedType(rhsTarget, cltTarget);
}

void reportMemLeak(SymProc &proc, const EStorageClass code, const char *reason)
{
    const struct cl_loc *loc = proc.lw();
    const char *const what = describeObj(code);
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
        const FldHandle            &dst,
        const TValId                val,
        const EPointerKind          code)
{
    SymHeap &sh = proc.sh();
    TStorRef stor = sh.stor();

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

    const TSizeOf dstSize = dst.type()->size;
    if (ptrSize <= dstSize)
        return val;

    const struct cl_loc *loc = proc.lw();
    CL_ERROR_MSG(loc, "not enough space to store value of a pointer");
    CL_NOTE_MSG(loc, "dstSize: " << dstSize << " B");
    CL_NOTE_MSG(loc, "ptrSize: " << ptrSize << " B");
    proc.printBackTrace(ML_ERROR);
    return sh.valCreate(VT_UNKNOWN, VO_REINTERPRET);
}

TValId ptrObjectEncoder(SymProc &proc, const FldHandle &dst, TValId val)
{
    return ptrObjectEncoderCore(proc, dst, val, PK_DATA);
}

// FIXME: the following code is full of off-by-one errors and integral overflows
// TODO: allow overflow detection for target types >= sizeof(long) on host
TValId integralEncoder(
        SymProc                    &proc,
        const FldHandle            &dst,
        const TValId                val,
        const IR::Range            &rngOrig)
{
    const struct cl_loc *loc = proc.lw();

    // read type-info of the target object
    const TObjType clt = dst.type();
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

TValId customValueEncoder(SymProc &proc, const FldHandle &dst, TValId val)
{
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

void objSetAtomicVal(SymProc &proc, const FldHandle &lhs, TValId rhs)
{
    if (!lhs.isValidHandle()) {
        CL_ERROR_MSG(proc.lw(), "invalid L-value");
        proc.printBackTrace(ML_ERROR);
        return;
    }

    SymHeap &sh = proc.sh();

    // generic prototype for a value encoder
    TValId (*encode)(SymProc &, const FldHandle &fld, const TValId val) = 0;

    const EValueTarget codeRhs = sh.valTarget(rhs);
    if (isPossibleToDeref(sh, rhs))
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

    if (lm.collectJunkFrom(killedPtrs)) {
        const TObjId objLhs = lhs.obj();
        const EStorageClass codeLhs = sh.objStorClass(objLhs);
        reportMemLeak(proc, codeLhs, "assign");
    }

    lm.leave();
}

void SymProc::setValueOf(const FldHandle &lhs, TValId rhs)
{
    const TValId lhsAt = lhs.placedAt();
    CL_BREAK_IF(!isPossibleToDeref(sh_, lhsAt));

    const TObjType clt = lhs.type();
    const TSizeOf size = clt->size;
    CL_BREAK_IF(!size);

    if (VO_DEREF_FAILED == sh_.valOrigin(rhs)) {
        // we're already on an error path
        const TValId tplValue = sh_.valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

        const UniformBlock ub = {
            /* off      */  lhs.offset(),
            /* size     */  size,
            /* tplValue */  tplValue
        };

        if (isComposite(clt, /* includingArray */ false))
            sh_.writeUniformBlock(lhs.obj(), ub);
        else
            lhs.setValue(tplValue);

        return;
    }

    if (VT_COMPOSITE != sh_.valTarget(rhs)) {
        // not a composite object
        objSetAtomicVal(*this, lhs, rhs);
        return;
    }

    // resolve rhs
    const FldHandle compObj(sh_, sh_.valGetComposite(rhs));
    const TValId rhsAt = compObj.placedAt();

    // wrap the size of the object being assigned as a heap value
    const CustomValue cv(IR::rngFromNum(size));
    const TValId valSize = sh_.valWrapCustom(cv);

    // C99 says that lhs and rhs cannot _partially_ overlap
    // TODO: avoid reporting errors for full overlaps
    // TODO: check for overlaps when assigning atomic variables, too
    executeMemmove(*this, lhsAt, rhsAt, valSize, /* allowOverlap */ false);
}

void SymProc::objDestroy(TObjId obj)
{
    if (OBJ_RETURN == obj && !sh_.isValid(obj))
        return;

    LeakMonitor lm(sh_);
    lm.enter();

    const EStorageClass code = sh_.objStorClass(obj);
    if (/* leaking */ lm.destroyObject(obj))
        reportMemLeak(*this, code, "destroy");

    lm.leave();
}

void SymProc::killVar(const CodeStorage::KillVar &kv)
{
    const int nestLevel = bt_->countOccurrencesOfTopFnc();
    const CVar cVar(kv.uid, nestLevel);
    const TObjId obj = sh_.regionByVar(cVar, /* createIfNeeded */ false);
    if (OBJ_INVALID == obj)
        // the var is dead already
        return;

    const std::string varString = varToString(sh_.stor(), kv.uid);

    if (!sh_.pointedByCount(obj)) {
        // just destroy the variable
#if DEBUG_SE_STACK_FRAME
        CL_DEBUG_MSG(lw_, "FFF SymProc::killVar() destroys var " << varString);
#endif
        this->objDestroy(obj);
        return;
    }

    // somebody points at the var

    if (kv.onlyIfNotPointed)
        // killer suggests to wait with its destruction
        return;

    // if it cannot be safely removed, then at least invalidate its contents
    CL_DEBUG_MSG(lw_, "FFF SymProc::killVar() invalidates var " << varString);
    const TValId valUnknown = sh_.valCreate(VT_UNKNOWN, VO_ASSIGNED);
    const TSizeRange size = sh_.objSize(obj);
    CL_BREAK_IF(!isSingular(size));

    // enter leak monitor
    LeakMonitor lm(sh_);
    lm.enter();

    // invalidate the contents
    TValSet killedPtrs;
    const UniformBlock ub = {
        /* off      */  0,
        /* size     */ size.lo,
        /* tplValue */ valUnknown
    };
    sh_.writeUniformBlock(obj, ub, &killedPtrs);

    // check for memory leaks
    if (lm.collectJunkFrom(killedPtrs)) {
        CL_WARN_MSG(lw_,
                "memory leak detected while invalidating a dead variable");
        this->printBackTrace(ML_WARN);
    }

    // leave leak monitor
    lm.leave();
}

bool headingToAbort(const CodeStorage::Block *bb)
{
    const CodeStorage::Insn *term = bb->back();
    const cl_insn_e code = term->code;
    return (CL_INSN_ABORT == code);
}

void SymProc::killInsn(const CodeStorage::Insn &insn)
{
    using namespace CodeStorage;
#if !SE_EARLY_VARS_DESTRUCTION
    return;
#endif
    // kill variables
    BOOST_FOREACH(const KillVar &kv, insn.varsToKill)
        this->killVar(kv);
}

void SymProc::killPerTarget(const CodeStorage::Insn &insn, unsigned target)
{
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
    const TObjId obj = sh.objByAddr(root);
    const EValueOrigin code = sh.valOrigin(valToWrite);
    const TValId valUnknown = sh.valCreate(VT_UNKNOWN, code);

    // how much memory can we guarantee the content of?
    IR::Range safeRange;
    safeRange.alignment = IR::Int1;
    safeRange.lo = addrRange.hi;
    safeRange.hi = addrRange.lo + sizeRange.lo;

    // check whether we are able to write something specific at all
    if (VAL_NULL != valToWrite || safeRange.hi <= safeRange.lo) {
        CL_DEBUG("memset() only invalidates the given range");
        const IR::TInt totalSize = widthOf(totalRange) - /* closed int */ 1;
        const UniformBlock ubAll = {
            /* off      */  totalRange.lo,
            /* size     */  totalSize,
            /* tplValue */  valUnknown
        };
        sh.writeUniformBlock(obj, ubAll, killedPtrs);
        return;
    }

    // compute the size we can write precisely
    const IR::TInt safeSize = widthOf(safeRange) - /* closed int */ 1;
    CL_BREAK_IF(safeSize <= 0);

    // valToWrite is VAL_NULL (we do not support writing arbitrary values yet)
    const UniformBlock ubSafe = {
        /* off      */  safeRange.lo,
        /* size     */  safeSize,
        /* tplValue */  valToWrite,
    };
    sh.writeUniformBlock(obj, ubSafe, killedPtrs);

    // compute size of the prefix we _have_ to invalidate
    const IR::TInt prefixSize = safeRange.lo - totalRange.lo;
    CL_BREAK_IF(prefixSize < 0);
    if (0 < prefixSize) {
        CL_DEBUG("memset() invalidates ambiguous prefix");
        const UniformBlock ubPrefix = {
            /* off      */  totalRange.lo,
            /* size     */  prefixSize,
            /* tplValue */  valUnknown
        };
        sh.writeUniformBlock(obj, ubPrefix, killedPtrs);
    }

    // compute size of the suffix we _have_ to invalidate
    const IR::TInt suffixSize = totalRange.hi - safeRange.hi;
    CL_BREAK_IF(suffixSize < 0);
    if (0 < suffixSize) {
        CL_DEBUG("memset() invalidates ambiguous suffix");
        const UniformBlock ubSuffix = {
            /* off      */  safeRange.hi,
            /* size     */  suffixSize,
            /* tplValue */  valUnknown
        };
        sh.writeUniformBlock(obj, ubSuffix, killedPtrs);
    }
}

inline void wipeAlignment(IR::Range &rng)
{
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
    if (segProveNeq(sh, rootDst, rootSrc))
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
    const char *fnc = (allowOverlap)
        ? "memmove()"
        : "memcpy()";

    const TObjId objDst = sh.objByAddr(valDst);

    IR::Range size;
    if (!rngFromVal(&size, sh, valSize) || size.lo < 0) {
        CL_ERROR_MSG(loc, "size arg of " << fnc << " is not a known integer");
        proc.printBackTrace(ML_ERROR);
        return;
    }
    if (!size.hi) {
        CL_DEBUG_MSG(loc, "ignoring call of " << fnc << " with size == 0");
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
        CL_ERROR_MSG(loc, "source and destination overlap in call of " << fnc);
        proc.printBackTrace(ML_ERROR);
        return;
    }

    LeakMonitor lm(sh);
    lm.enter();

    TValSet killedPtrs;
    sh.copyBlockOfRawMemory(valDst, valSrc, size.lo, &killedPtrs);

    if (!isSingular(size)) {
        CL_DEBUG_MSG(loc, fnc << " invalidates ambiguous suffix");
        const TValId valUnknown = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        const TSizeOf suffWidth = widthOf(size) - /* closed int */ 1;
        const UniformBlock ubSuffix = {
            /* off      */  size.lo,
            /* size     */  suffWidth,
            /* tplValue */  valUnknown
        };
        sh.writeUniformBlock(objDst, ubSuffix, &killedPtrs);
    }

    if (lm.collectJunkFrom(killedPtrs)) {
        CL_WARN_MSG(loc, "memory leak detected while executing " << fnc);
        proc.printBackTrace(ML_WARN);
    }

    lm.leave();
}


// /////////////////////////////////////////////////////////////////////////////
// SymExecCore implementation
void SymExecCore::varInit(TObjId obj)
{
    if (!ep_.trackUninit)
        return;

    const EStorageClass code = sh_.objStorClass(obj);
    if (SC_ON_STACK == code) {
        // uninitialized stack variable
        const TSizeRange size = sh_.objSize(obj);
        const TValId tpl = sh_.valCreate(VT_UNKNOWN, VO_STACK);
        CL_BREAK_IF(!isSingular(size));

        const UniformBlock ub = {
            /* off      */  0,
            /* size     */  size.lo,
            /* tplValue */  tpl
        };

        sh_.writeUniformBlock(obj, ub);
    }

    SymProc::varInit(obj);
}

void SymExecCore::execFree(TValId val)
{
    if (VAL_NULL == val) {
        CL_DEBUG_MSG(lw_, "ignoring free() called with NULL value");
        return;
    }

    const EValueTarget code = sh_.valTarget(val);
    switch (code) {
        case VT_CUSTOM:
            CL_ERROR_MSG(lw_, "free() called on non-pointer value");
            this->printBackTrace(ML_ERROR);
            return;

        case VT_RANGE:
            CL_BREAK_IF("please implement");
            // fall through!

        case VT_INVALID:
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

        case VT_OBJECT:
            break;
    }

    const TObjId obj = sh_.objByAddr(val);
    const bool hasValidTarget = sh_.isValid(obj);

    const EStorageClass sc = sh_.objStorClass(obj);
    switch (sc) {
        case SC_ON_HEAP:
            break;

        default:
            CL_BREAK_IF("execFree() got something special");
            // fall through

        case SC_ON_STACK:
            if (!hasValidTarget) {
                CL_ERROR_MSG(lw_, "attempt to free a non-existing non-heap object");
                this->printBackTrace(ML_ERROR);
                return;
            }
            // fall through!

        case SC_STATIC:
            CL_ERROR_MSG(lw_, "attempt to free a non-heap object");
            this->printBackTrace(ML_ERROR);
            return;
    }

    if (!hasValidTarget) {
        CL_ERROR_MSG(lw_, "double free()");
        this->printBackTrace(ML_ERROR);
        return;
    }

    const TOffset off = sh_.valOffset(val);
    if (off) {
        CL_ERROR_MSG(lw_, "free() called with offset " << off << "B");
        this->printBackTrace(ML_ERROR);
        return;
    }

    CL_DEBUG_MSG(lw_, "executing free()");
    this->objDestroy(obj);
}

void SymExecCore::execStackRestore()
{
    TObjList anonStackObjs;
    const CallInst callInst(this->bt_);
    sh_.clearAnonStackObjects(anonStackObjs, callInst);

    BOOST_FOREACH(const TObjId obj, anonStackObjs) {
        CL_DEBUG_MSG(lw_, "releasing an anonymous stack object");
        this->objDestroy(obj);
    }
}

bool lhsFromOperand(FldHandle *pLhs, SymProc &proc, const struct cl_operand &op)
{
    if (seekRefAccessor(op.accessor))
        CL_BREAK_IF("lhs not an l-value");

    *pLhs = proc.fldByOperand(op);
    if (FLD_DEREF_FAILED == pLhs->fieldId())
        return false;

    CL_BREAK_IF(!pLhs->isValidHandle());
    return true;
}

void SymExecCore::execStackAlloc(
        const struct cl_operand         &opLhs,
        const TSizeRange                &size)
{
    // resolve lhs
    FldHandle lhs;
    if (!lhsFromOperand(&lhs, *this, opLhs))
        // error alredy emitted
        return;

    if (!size.hi) {
        // object of zero size could hardly be properly allocated
        const TValId valUnknown = sh_.valCreate(VT_UNKNOWN, VO_STACK);
        this->setValueOf(lhs, valUnknown);
        return;
    }

    // now create an annonymous stack object
    const CallInst callInst(this->bt_);
    const TObjId obj = sh_.stackAlloc(size, callInst);

    if (ep_.trackUninit) {
        // uninitialized heap block
        const TValId tplValue = sh_.valCreate(VT_UNKNOWN, VO_STACK);
        const UniformBlock ub = {
            /* off      */  0,
            /* size     */  size.lo,
            /* tplValue */  tplValue
        };
        sh_.writeUniformBlock(obj, ub);
    }

    // store the address returned by alloca()
    const TValId addr = sh_.addrOfTarget(obj, TS_REGION);
    this->setValueOf(lhs, addr);
}

void SymExecCore::execHeapAlloc(
        SymState                        &dst,
        const CodeStorage::Insn         &insn,
        const TSizeRange                size,
        const bool                      nullified)
{
    // resolve lhs
    FldHandle lhs;
    if (!lhsFromOperand(&lhs, *this, insn.operands[/* dst */ 0]))
        // error alredy emitted
        return;

    if (!size.hi) {
        CL_WARN_MSG(lw_, "POSIX says that, given zero size, the behavior of \
malloc/calloc is implementation-defined");
        CL_NOTE_MSG(lw_, "assuming NULL as the result");
        this->printBackTrace(ML_WARN);
        this->setValueOf(lhs, VAL_NULL);
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
        const FldHandle oomLhs(oomHeap, lhs);
        oomCore.setValueOf(oomLhs, VAL_NULL);
        oomCore.killInsn(insn);
        dst.insert(oomHeap);
    }

    // now create a heap object
    const TObjId reg = sh_.heapAlloc(size);

    UniformBlock ub = {
        /* off      */  0,
        /* size     */  size.lo,
        /* tplValue */  VAL_NULL
    };

    if (nullified) {
        // initialize to zero as we are doing calloc()
        sh_.writeUniformBlock(reg, ub);
    }
    else if (ep_.trackUninit) {
        // uninitialized heap block
        ub.tplValue = sh_.valCreate(VT_UNKNOWN, VO_HEAP);
        sh_.writeUniformBlock(reg, ub);
    }

    // store the result of malloc
    const TValId val = sh_.addrOfTarget(reg, TS_REGION);
    this->setValueOf(lhs, val);
    this->killInsn(insn);
    dst.insert(sh_);
}

bool describeCmpOp(CmpOpTraits *pTraits, const enum cl_binop_e code)
{
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
    if (segProveNeq(sh, v1, v2) && cTraits.preserveEq)
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

bool spliceOutAbstractPathCore(
        SymProc                &proc,
        const TValId            addrFirst,
        const TValId            endPoint,
        const bool              readOnlyMode = false)
{
    SymHeap &sh = proc.sh();

    TObjSet leakObjs;
    LeakMonitor lm(sh);
    lm.enter();

    // NOTE: If there is a cycle consisting of empty list segments only, we will
    // loop indefinitely.  However, the basic list segment axiom guarantees that
    // there is no such cycle.

    TValId addr = addrFirst;
    int len = 1;

    for (;;) {
        const TObjId seg = sh.objByAddr(addr);
        if (OK_REGION == sh.objKind(seg) || objMinLength(sh, seg)) {
            // we are on a wrong way already...
            CL_BREAK_IF(!readOnlyMode);
            return false;
        }

        const TValId valNext = nextValFromSegAddr(sh, addr);

        if (!readOnlyMode)
            spliceOutListSegment(sh, seg, &leakObjs);

        if (valNext == endPoint)
            // we have the chain we are looking for
            break;

        addr = valNext;
        ++len;
    }

    if (readOnlyMode)
        return true;

    // read-write pass done
    const struct cl_loc *loc = proc.lw();
    CL_DEBUG_MSG(loc, "spliceOutAbstractPathCore() removed "
            << len << " possibly empty abstract objects");

    // append a trace node for this operation
    sh.traceUpdate(new Trace::SpliceOutNode(sh.traceNode(), len));

    if (lm.importLeakObjs(&leakObjs)) {
        // [L0] leakage during splice-out
        CL_WARN_MSG(loc, "memory leak detected while removing a segment");
        proc.printBackTrace(ML_WARN);
        lm.leave();
    }

    return true;
}

bool valMerge(SymState &dst, SymProc &proc, TValId v1, TValId v2);

bool dlSegMergeAddressesOfEmpty(
        SymState                    &dst,
        SymProc                     &procTpl,
        const TValId                 addr1,
        const TValId                 addr2)
{
    // we need to clone the SymHeap and SymProc objects
    SymHeap sh(procTpl.sh());
    Trace::waiveCloneOperation(sh);
    SymProc proc(sh, procTpl.bt());
    proc.setLocation(procTpl.lw());

    const TValId valNext1 = prevValFromSegAddr(sh, addr1);
    const TValId valNext2 = prevValFromSegAddr(sh, addr2);

    if (!spliceOutAbstractPathCore(proc, addr1, valNext2))
        CL_BREAK_IF("dlSegMergeAddressesOfEmpty() failed to remove a DLS");

    if (valNext1 == valNext2) {
        dst.insert(sh);
        return true;
    }

    CL_DEBUG_MSG(proc.lw(),
            "dlSegMergeAddressesIfNeeded() calls valMerge() recursively");

    return valMerge(dst, proc, valNext1, valNext2);
}

bool dlSegMergeAddressesIfNeeded(
        SymState                    &dst,
        SymProc                     &proc,
        const TValId                 v1,
        const TValId                 v2)
{
    SymHeap &sh = proc.sh();
    const TObjId obj1 = sh.objByAddr(v1);
    const TObjId obj2 = sh.objByAddr(v2);

    if (!isAbstractObject(sh, obj1) || !isAbstractObject(sh, obj2))
        // not a pair of abstract values
        return false;

    const TOffset off1 = sh.valOffset(v1);
    const TOffset off2 = sh.valOffset(v2);
    if (off1 != off2)
        // the given value differ in target offset
        return false;

    const ETargetSpecifier ts1 = sh.targetSpec(v1);
    const ETargetSpecifier ts2 = sh.targetSpec(v2);
    if (ts1 == ts2 || obj1 != obj2)
        // apparently not the case we are looking for
        return false;

    if (!sh.segMinLength(obj1))
        // 0+ DLS --> we have to look through!
        dlSegMergeAddressesOfEmpty(dst, proc, v1, v2);

    dlSegReplaceByConcrete(sh, obj1 /* = obj2 */);
    sh.traceUpdate(new Trace::SpliceOutNode(sh.traceNode(), /* len */ 1));
    dst.insert(sh);
    return true;
}

bool spliceOutAbstractPath(
        SymProc                     &proc,
        const TValId                 atAddr,
        const TValId                 pointingTo,
        const bool                   readOnlyMode = false)
{
    SymHeap &sh = proc.sh();

    TValId endPoint = pointingTo;

    const TObjId seg = sh.objByAddr(atAddr);

    const EObjKind kind = sh.objKind(seg);
    if (OK_OBJ_OR_NULL != kind) {
        // if atAddr is above/bellow head, we need to shift endPoint accordingly
        const TOffset off = sh.segBinding(seg).head - sh.valOffset(atAddr);
        endPoint = sh.valByOffset(pointingTo, off);
    }

    if (!spliceOutAbstractPathCore(proc, atAddr, endPoint, /* RO */ true))
        // read-only attempt failed
        return false;

    if (readOnlyMode || spliceOutAbstractPathCore(proc, atAddr, endPoint))
        return true;

    CL_BREAK_IF("failed to splice-out a single list segment");
    return false;
}

bool valMerge(SymState &dst, SymProc &proc, TValId v1, TValId v2)
{
    SymHeap &sh = proc.sh();

    moveKnownValueToLeft(sh, v1, v2);

    // check that at least one value is unknown
    const bool isAbstract1 = isAbstractObject(sh, sh.objByAddr(v1));
    const bool isAbstract2 = isAbstractObject(sh, sh.objByAddr(v2));
    if (!isAbstract1 && !isAbstract2) {
        // no abstract objects involved
        sh.valReplace(v2, v1);
        dst.insert(sh);
        return true;
    }

    // where did we get here from?
    Trace::Node *trNode = sh.traceNode();

    if (isAbstract1 && spliceOutAbstractPath(proc, v1, v2)) {
        // splice-out succeeded ... ls(v1, v2)
        dst.insert(sh);
        return true;
    }

    if (isAbstract2 && spliceOutAbstractPath(proc, v2, v1)) {
        // splice-out succeeded ... ls(v2, v1)
        dst.insert(sh);
        return true;
    }

    if (dlSegMergeAddressesIfNeeded(dst, proc, v1, v2))
        // splice-out succeeded ... v1 -> dls <- v2
        return true;

    // collect the sets of values we get by jumping over 0+ abstract objects
    TValSet seen1, seen2;
    lookThrough(sh, v1, &seen1);
    lookThrough(sh, v2, &seen2);

    const struct cl_loc *loc = proc.lw();

    // try to look through possibly empty objects
    const TValId v1Tail = lookThrough(sh, v1, &seen2);
    const TValId v2Tail = lookThrough(sh, v2, &seen1);
    if (v1Tail == v2Tail && v1 != v1Tail && v2 != v2Tail) {
        CL_DEBUG_MSG(loc, "valMerge() removes two abstract paths at a time");

        if (!spliceOutAbstractPath(proc, v1, v1Tail, /* read-only */ true))
            goto fail;
        if (!spliceOutAbstractPath(proc, v2, v2Tail, /* read-only */ true))
            goto fail;

        // go ahead, try it read-write!

        if (spliceOutAbstractPath(proc, v1, v1Tail)
                && spliceOutAbstractPath(proc, v2, v2Tail))
        {
            dst.insert(sh);
            return true;
        }

        CL_ERROR_MSG(loc, "internal error in valMerge(), heap inconsistent!");
        CL_BREAK_IF("this will need some debugging...");
    }

fail:
    CL_DEBUG_MSG(loc, "failed to splice-out list segment!");
    trNode = new Trace::SpliceOutNode(trNode, /* failed */ 0);
    sh.traceUpdate(trNode);
    dst.insert(sh);
    return false;
}

bool reflectCmpResult(
        SymState                   &dst,
        SymProc                    &procSrc,
        const enum cl_binop_e       code,
        const bool                  branch,
        const TValId                v1,
        const TValId                v2)
{
    SymHeap &sh = procSrc.sh();
    CL_BREAK_IF(!protoCheckConsistency(sh));

    // create a slave SymProc instance
    SymProc proc(sh, procSrc.bt());
    proc.setLocation(procSrc.lw());

    // resolve binary operator
    CmpOpTraits cTraits;
    if (!describeCmpOp(&cTraits, code))
        goto fail;

    if (trimRangesIfPossible(sh, cTraits, branch, v1, v2))
        goto done;

    if (branch == cTraits.negative) {
        if (!cTraits.preserveNeq)
            goto fail;

        if (!segApplyNeq(sh, v1, v2))
            // introduce an explicit Neq predicate over v1 and v2
            sh.addNeq(v1, v2);
    }
    else {
        if (!cTraits.preserveEq)
            goto fail;

        // we have deduced that v1 and v2 is actually the same value
        return valMerge(dst, proc, v1, v2);
    }

done:
    CL_BREAK_IF(!protoCheckConsistency(sh));
    dst.insert(sh);
    return true;

fail:
    dst.insert(sh);
    return false;
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

            default:
                break;
        }

        if (computeIntRngResult(&result, sh, code, rng1, rng2))
            return result;
    }

    return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
}

TValId handleBitNot(SymHeapCore &sh, const TValId val)
{
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

bool isAnyIntValue(const SymHeapCore &sh, const TValId val)
{
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
void SymExecCore::execOp(const CodeStorage::Insn &insn)
{
    // resolve lhs
    FldHandle lhs;
    const struct cl_operand &dst = insn.operands[/* dst */ 0];
    if (!lhsFromOperand(&lhs, *this, dst))
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
        const TObjId obj = lhs.obj();

        FldList liveObjs;
        sh_.gatherLiveFields(liveObjs, obj);
        BOOST_FOREACH(const FldHandle &fld, liveObjs)
            if (fld == lhs)
                goto already_alive;

        return;
    }
already_alive:
#endif
    // store the result
    this->setValueOf(lhs, valResult);
}

void SymExecCore::handleLabel(const CodeStorage::Insn &insn)
{
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

            // we expect a pointer at this point
            const TObjId ptr = slave.objByVar(op);
            CL_BREAK_IF(!sh.isValid(ptr));

            // read the value inside the pointer
            const TValId addr = valOfPtr(sh, ptr, /* off */ 0);
            if (!canWriteDataPtrAt(sh, addr))
                continue;

            // resolve the target object of the address
            const TObjId obj = sh.objByAddr(addr);
            CL_BREAK_IF(!sh.isValid(obj));

            // check whether the target is an abstract object
            const EObjKind kind = sh.objKind(obj);
            if (OK_REGION == kind)
                continue;

            const ETargetSpecifier ts = sh.targetSpec(addr);
#ifndef NDEBUG
            CL_BREAK_IF(hitLocal);
            hitLocal = true;
            hit = true;
#endif
            LeakMonitor lm(sh);
            lm.enter();

            TObjSet leakObjs;
            concretizeObj(sh, todo, obj, ts, &leakObjs);

            if (lm.importLeakObjs(&leakObjs)) {
                CL_WARN_MSG(lw_, "memory leak detected while unfolding");
                this->printBackTrace(ML_WARN);
            }

            lm.leave();
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

bool SymExecCore::exec(SymState &dst, const CodeStorage::Insn &insn)
{
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
