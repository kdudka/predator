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
    bt_->printBackTrace(forcePtrace);

    // dump trace graph, or schedule and endpoint for batch trace graph dump
#if 2 == SE_DUMP_TRACE_GRAPHS
    Trace::plotTrace(sh_.traceNode(), "symtrace");
#elif 1 == SE_DUMP_TRACE_GRAPHS
    Trace::GraphProxy *glProxy = Trace::Globals::instance()->glProxy();
    glProxy->insert(sh_.traceNode(), "symtrace");
#endif

    printMemUsage("SymBackTrace::printBackTrace");
    if (ML_ERROR != level)
        // do not panic for now
        return;

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

        case CL_TYPE_REAL:
            // floating-point value
            cv.code = CV_REAL;
            cv.data.fpn = cst.data.cst_real.value;
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
        case VT_RANGE:
            CL_BREAK_IF("please implement");

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
    const TOffset size = sh_.valSizeOfTarget(at);

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

    if (nullify) {
        // initialize to zero
        sh_.writeUniformBlock(at, VAL_NULL, size);
    }
#if SE_TRACK_UNINITIALIZED
    else if (isLcVar) {
        // uninitialized stack variable
        const TValId tpl = sh_.valCreate(VT_UNKNOWN, VO_STACK);
        sh_.writeUniformBlock(at, tpl, size);
    }
#endif

    if (!var.initials.empty())
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

TValId SymProc::heapValFromObj(const struct cl_operand &op) {
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

void SymProc::heapObjDefineType(const ObjHandle &lhs, TValId rhs) {
    const EValueTarget code = sh_.valTarget(rhs);
    if (!isPossibleToDeref(code))
        // no valid target anyway
        return;

    if (sh_.valOffset(rhs))
        // not a pointer to root
        return;

    const TObjType cltTarget = targetTypeOfPtr(lhs.objType());
    if (!cltTarget || CL_TYPE_VOID == cltTarget->code)
        // no type-info given for the target
        return;

    const TObjType cltLast = sh_.valLastKnownTypeOfTarget(rhs);
    if (isComposite(cltLast) && !isComposite(cltTarget))
        // we are accessing a field that is placed at zero offset of a composite
        // type but it yet does not mean that we are changing the root type-info
        return;

    const int rootSize = sh_.valSizeOfTarget(rhs);
    CL_BREAK_IF(rootSize <= 0 && isOnHeap(code));
    if (cltLast && cltLast->size == rootSize && cltTarget->size != rootSize)
        // basically the same rule as above but now we check the size of target
        return;

    // update the last known type-info of the root
    sh_.valSetLastKnownTypeOfTarget(rhs, cltTarget);
}

void SymProc::reportMemLeak(const EValueTarget code, const char *reason) {
    const char *const what = describeRootObj(code);
    CL_WARN_MSG(lw_, "memory leak detected while " << reason << "ing " << what);
    this->printBackTrace(ML_WARN);
}

void SymProc::heapSetSingleVal(const ObjHandle &lhs, TValId rhs) {
    if (!lhs.isValid()) {
        CL_ERROR_MSG(lw_, "invalid L-value");
        this->printBackTrace(ML_ERROR);
        return;
    }

    const TValId lhsAt = lhs.placedAt();
    const EValueTarget code = sh_.valTarget(lhsAt);
    CL_BREAK_IF(!isPossibleToDeref(code));

    if (isPossibleToDeref(sh_.valTarget(rhs))) {
        // we are going to write a pointer, check if we have enough space for it
        const TOffset dstSize = sh_.valSizeOfTarget(lhsAt);
        const TOffset ptrSize = sh_.stor().types.dataPtrSizeof();
        if (dstSize < ptrSize) {
            CL_ERROR_MSG(lw_, "not enough space to store value of a pointer");
            CL_NOTE_MSG(lw_, "dstSize: " << dstSize << " B");
            CL_NOTE_MSG(lw_, "ptrSize: " << ptrSize << " B");
            this->printBackTrace(ML_ERROR);
            rhs = sh_.valCreate(VT_UNKNOWN, VO_REINTERPRET);
        }
    }

    // update type-info
    this->heapObjDefineType(lhs, rhs);

    LeakMonitor lm(sh_);
    lm.enter();

    TValSet killedPtrs;
    lhs.setValue(rhs, &killedPtrs);

    if (lm.collectJunkFrom(killedPtrs))
        this->reportMemLeak(code, "assign");

    lm.leave();
}

void SymProc::objSetValue(const ObjHandle &lhs, TValId rhs) {
    const TValId lhsAt = lhs.placedAt();
    const EValueTarget code = sh_.valTarget(lhsAt);
    CL_BREAK_IF(!isPossibleToDeref(code));

    const TObjType clt = lhs.objType();
    const bool isComp = isComposite(clt, /* includingArray */ false);
    const unsigned size = clt->size;
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
        this->heapSetSingleVal(lhs, rhs);
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
        this->reportMemLeak(code, "assign");

    lm.leave();
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
        const IntRange              &addrRange,
        const IntRange              &sizeRange,
        const IntRange              &totalRange,
        TValSet                     *killedPtrs)
{
    const TValId valUnknown = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    const TValId valBegTotal = sh.valByOffset(root, totalRange.lo);

    // how much memory can we guarantee the content of?
    IntRange safeRange;
    safeRange.lo = addrRange.hi;
    safeRange.hi = addrRange.lo + sizeRange.lo;

    // check whether we are able to write something specific at all
    if (VAL_NULL != valToWrite || safeRange.hi <= safeRange.lo) {
        CL_DEBUG("memset() only invalidates the given range");
        const long totalSize = widthOf(totalRange) - /* closed int */ 1;
        sh.writeUniformBlock(valBegTotal, valUnknown, totalSize, killedPtrs);
        return;
    }

    // compute the size we can write precisely
    const long safeSize = widthOf(safeRange) - /* closed int */ 1;
    CL_BREAK_IF(safeSize <= 0);

    // valToWrite is VAL_NULL (we do not support writing arbitrary values yet)
    const TValId valBegSafe = sh.valByOffset(root, safeRange.lo);
    sh.writeUniformBlock(valBegSafe, valToWrite, safeSize, killedPtrs);

    // compute size of the prefix we _have_ to invalidate
    const long prefixSize = safeRange.lo - totalRange.lo;
    CL_BREAK_IF(prefixSize < 0);
    if (0 < prefixSize) {
        CL_DEBUG("memset() invalidates ambiguous prefix");
        sh.writeUniformBlock(valBegTotal, valUnknown, prefixSize, killedPtrs);
    }

    // compute size of the suffix we _have_ to invalidate
    const long suffixSize = totalRange.hi - safeRange.hi;
    CL_BREAK_IF(suffixSize < 0);
    if (0 < suffixSize) {
        CL_DEBUG("memset() invalidates ambiguous suffix");
        const TValId suffixAddr = sh.valByOffset(root, safeRange.hi);
        sh.writeUniformBlock(suffixAddr, valUnknown, suffixSize, killedPtrs);
    }
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
    IntRange sizeRange;
    if (!rangeFromVal(&sizeRange, sh, valSize) || sizeRange.lo < 0) {
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
    const IntRange addrRange = sh.valOffsetRange(addr);

    // deduce whether the end is fixed or not
    bool isEndFixed = isSingular(addrRange) && isSingular(sizeRange);
    if (!isEndFixed && (widthOf(addrRange) == widthOf(sizeRange))) {
        bool neg;
        if (sh.areBound(&neg, addr, valSize) && neg)
            isEndFixed = true;
    }

    // how much memory are we going to touch in the worst case?
    IntRange totalRange;
    totalRange.lo = addrRange.lo;
    totalRange.hi = addrRange.hi + ((isEndFixed)
        ? sizeRange.lo
        : sizeRange.hi);

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
        const unsigned                  size,
        const bool                      nullified)
{
    // resolve lhs
    const ObjHandle lhs = this->objByOperand(insn.operands[/* dst */ 0]);
    if (OBJ_DEREF_FAILED == lhs.objId())
        // error alredy emitted
        return;

    if (!size) {
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
    if (nullified)
        // initilize to zero as we are doing calloc()
        sh_.writeUniformBlock(val, VAL_NULL, size);

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

bool compareIntCsts(
        bool                        *pDst,
        const enum cl_binop_e       code,
        const long                  num1,
        const long                  num2)
{
    switch (code) {
        case CL_BINOP_NE:
            // TODO: improve SymHeapCore to handle it actually :-)
            // fall through!

        case CL_BINOP_EQ:
            // this should have been already handled by the caller
            return false;

        case CL_BINOP_LE:
            *pDst = (num1 <= num2);
            return true;

        case CL_BINOP_GE:
            *pDst = (num1 >= num2);
            return true;

        case CL_BINOP_LT:
            *pDst = (num1 <  num2);
            return true;

        case CL_BINOP_GT:
            *pDst = (num1 >  num2);
            return true;

        default:
            CL_BREAK_IF("unhandled binary operator in compareIntCsts()");
            return false;
    }
}

bool compareIntRanges(
        bool                        *pDst,
        const enum cl_binop_e       code,
        const IntRange              &range1,
        const IntRange              &range2)
{
    bool loResult;
    if (!compareIntCsts(&loResult, code, range1.lo, range2.lo))
        return false;

    bool hiResult;
    if (!compareIntCsts(&hiResult, code, range1.hi, range2.hi))
        return false;

    if (loResult == hiResult) {
        // we got the same results for both boundaries --> pick any
        *pDst = loResult;
        return true;
    }

#if !SE_ALLOW_OFF_RANGES
    CL_BREAK_IF("please implement");
#endif
    return false;
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
    const IntRange range1 = sh.valOffsetRange(v1);
    const IntRange range2 = sh.valOffsetRange(v2);
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
        const TObjType              /* clt */,
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

    long num1, num2;
    if (numFromVal(&num1, sh, v1) && numFromVal(&num2, sh, v2)) {
        // both values are integral constants
        bool result;
        if (compareIntCsts(&result, code, num1, num2))
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
    const bool ltr = cTraits.leftToRight;
    const bool rtl = cTraits.rightToLeft;
    CL_BREAK_IF(ltr && rtl);

    if (!ltr && !rtl)
        // not a suitable binary operator (modulo some corner cases)
        return false;

    const bool isRange1 = (VT_RANGE == sh.valTarget(v1));
    const bool isRange2 = (VT_RANGE == sh.valTarget(v2));
    if (isRange1 == isRange2)
        // not a suitable value pair for trimming
        return false;

    const TValId root = sh.valRoot(v1);
    if (root != sh.valRoot(v2))
        // root mismatch
        return false;

    // FIXME: the following code must be full of bugs
    CL_DEBUG("trimRangesIfPossible() is taking place...");

    // should we include the boundary to the result?
    const bool isOpen = (branch == cTraits.negative);

    // which boundary are we going to trim?
    const bool neg = (branch == isRange2);
    const bool trimLo = (neg == ltr);

    // pick the values in the appropriate order
    const TValId valRange = (isRange1) ? v1 : v2;
    const TValId valTrim  = (isRange2) ? v1 : v2;

    // extract the current offset range and trimming offset
    IntRange offRange = sh.valOffsetRange(valRange);
    const TOffset offTrim = sh.valOffset(valTrim);

    if (trimLo)
        // shift the lower bound up
        offRange.lo = offTrim + isOpen;
    else
        // shift the upper bound down
        offRange.hi = offTrim - isOpen;

    // trim the designated VT_RANGE value
    sh.valRestrictOffsetRange(valRange, offRange);
    return true;
}

bool reflectCmpResult(
        SymHeap                     &sh,
        const enum cl_binop_e       code,
        const bool                  branch,
        const TValId                v1,
        const TValId                v2)
{
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
        sh.valMerge(v1, v2);
    }

    return true;
}

// FIXME: avoid using a macro definition for this
#define INT_RAGNE_BINOP(dst, src1, op, src2) do {   \
    (dst).lo = (src1).lo op (src2).lo;              \
    (dst).hi = (src1).hi op (src2).hi;              \
} while (0)

bool computeIntRngResult(
        TValId                      *pDst,
        SymHeap                     &sh,
        const enum cl_binop_e       code,
        const IntRange              rng1,
        const IntRange              rng2)
{
    // compute the result of an integral binary operation
    IntRange result;
    switch (code) {
#if SE_ALLOW_CST_INT_PLUS_MINUS
        case CL_BINOP_PLUS:
            INT_RAGNE_BINOP(result, rng1, +, rng2);
            break;

        case CL_BINOP_MINUS:
            INT_RAGNE_BINOP(result, rng1, -, rng2);
            break;
#endif
        case CL_BINOP_BIT_AND:
            INT_RAGNE_BINOP(result, rng1, &, rng2);
            break;

        case CL_BINOP_MULT:
            INT_RAGNE_BINOP(result, rng1, *, rng2);
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
#if SE_ALLOW_CST_INT_PLUS_MINUS
            CL_BREAK_IF("unhandled binary integral operation");
#endif
            return false;
    }

    // wrap the result as a heap value
    CustomValue cv(CV_INT_RANGE);
    cv.data.rng = result;
    *pDst = sh.valWrapCustom(cv);
    return true;
}

TValId SymProc::handleIntegralOp(TValId v1, TValId v2, enum cl_binop_e code) {
    if (CL_BINOP_MINUS == code) {
        // chances are this could be a pointer difference
        const EValueTarget code1 = sh_.valTarget(v1);
        const EValueTarget code2 = sh_.valTarget(v2);

        if (isAnyDataArea(code1) && isAnyDataArea(code2))
            // ... and it indeed is a pointer difference
            return sh_.diffPointers(v1, v2);
    }

    // check whether we both values are integral constant
    IntRange rng1, rng2;
    if (rangeFromVal(&rng1, sh_, v1) && rangeFromVal(&rng2, sh_, v2)) {

        // first try to preserve range coincidence if we can
        switch (code) {
            case CL_BINOP_PLUS:
                if (!isSingular(rng1) && isSingular(rng2))
                    return sh_.valByOffset(v1, rng2.lo);
                else if (isSingular(rng1) && !isSingular(rng2))
                    return sh_.valByOffset(v2, rng1.lo);
                break;

            case CL_BINOP_MINUS:
                if (!isSingular(rng1) && isSingular(rng2))
                    return sh_.valByOffset(v1, -rng2.lo);
                break;

            default:
                break;
        }

        TValId result;
        if (computeIntRngResult(&result, sh_, code, rng1, rng2))
            return result;
    }

    return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);
}

TValId handleBitNot(SymHeap &sh, const TValId val) {
    // check whether the value is an integral constant
    long num;
    if (!numFromVal(&num, sh, val))
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);

    // compute the integral result
    const long result = ~num;

    // wrap the result as a heap value expressing a constant integer
    CustomValue cv(CV_INT);
    cv.data.num = result;
    return sh.valWrapCustom(cv);
}

TValId SymProc::handleIntegralOp(TValId val, enum cl_unop_e code) {
    switch (code) {
        case CL_UNOP_MINUS:
            return this->handleIntegralOp(VAL_NULL, val, CL_BINOP_MINUS);

        case CL_UNOP_BIT_NOT:
            return handleBitNot(sh_, val);

        default:
            CL_BREAK_IF("unhandled unary integral operation");
            return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }
}

TValId SymProc::handlePointerPlus(TValId at, TValId off, bool negOffset) {
    long num;
    if (!numFromVal(&num, sh_, off)) {
        CL_DEBUG_MSG(lw_, "pointer plus offset not a known integer");
        return sh_.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    TOffset offRequested = static_cast<TOffset>(num);
    if (negOffset)
        offRequested = -offRequested;

    CL_DEBUG("handlePointerPlus(): " << offRequested << "b offset requested");
    return sh_.valByOffset(at, offRequested);
}

// TODO: move this to symutil?
bool isIntCst(const SymHeap &sh, const TValId val) {
    switch (val) {
        case VAL_NULL:
        case VAL_TRUE:
            return true;

        default:
            if (VT_CUSTOM == sh.valTarget(val))
                break;

            return false;
    }

    const CustomValue &cv = sh.valUnwrapCustom(val);
    return (CV_INT == cv.code);
}

TValId handlePtrBitAnd(
        SymProc                    &proc,
        const TValId                vPtr,
        const TValId                vInt)
{
    SymHeap &sh = proc.sh();

    long mask;
    if (!numFromVal(&mask, sh, vInt) || 0 < mask)
        // giving up
        return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);

    if (!mask)
        // the whole pointer has been masked
        return VAL_NULL;

    // include all possible scenarios into consideration
    IntRange range;
    range.lo = 1L + mask;
    range.hi = 0L;

    // create the appropriate VT_RANGE value
    return sh.valByRange(vPtr, range);
}

TValId handlePtrOperator(
        SymProc                    &proc,
        const TValId                vPtr,
        const TValId                vInt,
        const enum cl_binop_e       code)
{
    SymHeap &sh = proc.sh();

    switch (code) {
        case CL_BINOP_PLUS:
            return proc.handlePointerPlus(vPtr, vInt, /* negOffset */ false);

        case CL_BINOP_MINUS:
            return proc.handlePointerPlus(vPtr, vInt, /* negOffset */ true);

        case CL_BINOP_BIT_AND:
            return handlePtrBitAnd(proc, vPtr, vInt);

        default:
            CL_BREAK_IF("unhandled binary operator in handlePtrOperator()");
            return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }
}

/// ptr arithmetic is sometimes (with CIL always) masked as integral arithmetic
bool reconstructPtrArithmetic(
        TValId                     *pResult,
        SymProc                    &proc,
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

    const SymHeap &sh = proc.sh();

    if (isAnyDataArea(sh.valTarget(v1)) && isIntCst(sh, v2)) {
        CL_DEBUG("integral operator applied on ptr handled as ptr operator...");
        *pResult = handlePtrOperator(proc, /* vPtr */ v1, /* vInt */ v2, code);
        return true;
    }

    if (isAnyDataArea(sh.valTarget(v2)) && isIntCst(sh, v1)) {
        if (CL_BINOP_MINUS == code)
            // CL_BINOP_MINUS makes no sense here, it would mean e.g. (4 - &foo)
            return false;

        CL_DEBUG("integral operator applied on ptr handled as ptr operator...");
        *pResult = handlePtrOperator(proc, /* vPtr */ v2, /* vInt */ v1, code);
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
        SymHeap &sh = proc.sh_;
        const TValId val = rhs[0];

        const enum cl_unop_e code = static_cast<enum cl_unop_e>(iCode);
        switch (code) {
            case CL_UNOP_BIT_NOT:
                if (!clt[0] || CL_TYPE_BOOL != clt[0]->code
                        || !clt[1] || CL_TYPE_BOOL != clt[1]->code)
                    return proc.handleIntegralOp(val, code);
                // gcc 4.7.x uses CL_UNOP_BIT_NOT for bools with truth semantics
                // fall through!

            case CL_UNOP_TRUTH_NOT:
                return compareValues(sh, CL_BINOP_EQ, clt[0], VAL_FALSE, val);

            case CL_UNOP_MINUS:
                return proc.handleIntegralOp(rhs[0], code);

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
        CL_BREAK_IF(!clt[0] || !clt[1] || !clt[2]);
        SymHeap &sh = proc.sh_;
        TValId vRes;

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

            case CL_BINOP_MULT:
                if (VAL_NULL == rhs[0] || VAL_NULL == rhs[1])
                    // whatever we got as the second operand, the result is zero
                    return VAL_NULL;

                // fall through!

            case CL_BINOP_MIN:
            case CL_BINOP_MAX:
                goto handle_int;

            case CL_BINOP_BIT_AND:
                if (VAL_NULL == rhs[0] || VAL_NULL == rhs[1])
                    // whatever we got as the second operand, the result is zero
                    return VAL_NULL;

                // fall through!

            case CL_BINOP_PLUS:
            case CL_BINOP_MINUS:
                if (reconstructPtrArithmetic(&vRes, proc, rhs[0], rhs[1], code))
                    return vRes;
                else
                    goto handle_int;

            case CL_BINOP_POINTER_PLUS:
                return proc.handlePointerPlus(rhs[0], rhs[1]);

            default:
                // over-approximate anything else
                return sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        }

handle_int:
        return proc.handleIntegralOp(rhs[0], rhs[1], code);
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

            // we expect a pointer at this point
            const TValId val = valOfPtrAt(sh, slave.varAt(op));
            if (VT_ABSTRACT == sh.valTarget(val)) {
#ifndef NDEBUG
                CL_BREAK_IF(hitLocal);
                hitLocal = true;
                hit = true;
#endif
                concretizeObj(sh, val, todo);
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
        derefs = opsWithDerefSemanticsInCallInsn(insn);

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
