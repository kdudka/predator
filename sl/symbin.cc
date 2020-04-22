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
#include "symbin.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

// we are mainly interested in the declaration of enum ___sl_module_id
#define PREDATOR
#include "../include/predator-builtins/verifier-builtins.h"
#undef PREDATOR

#include "glconf.hh"
#include "symabstract.hh"
#include "symdump.hh"
#include "symgc.hh"
#include "symjoin.hh"
#include "symplot.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "util.hh"

#include <cstring>
#include <libgen.h>
#include <map>

typedef const struct cl_loc     *TLoc;
typedef const struct cl_operand &TOp;

/// temporarily create a function call on top of the backtrace
class TempBackTraceTop {
    public:
        TempBackTraceTop(SymExecCore &core, TOp opFnc);
        ~TempBackTraceTop();

    private:
        SymBackTrace       *bt_;
};

TempBackTraceTop::TempBackTraceTop(SymExecCore &core, TOp opFnc):
    bt_(0)
{
    cl_uid_t uid;
    if (!core.fncFromOperand(&uid, opFnc))
        // failed to resolve opFnc
        return;

    bt_ = /* FIXME */ const_cast<SymBackTrace *>(core.bt());
    bt_->pushCall(uid, core.lw());
}

TempBackTraceTop::~TempBackTraceTop()
{
    if (bt_)
        bt_->popCall();
}

bool readPlotName(
        std::string                                 *dst,
        const CodeStorage::TOperandList             &opList,
        const struct cl_loc                         *loc)
{
    const cl_operand &op = opList[/* dst + fnc */ 2];
    if (CL_OPERAND_CST != op.code)
        return false;

    const cl_cst &cst = op.data.cst;
    if (CL_TYPE_STRING == cst.code) {
        // plot name given as string literal
        *dst = cst.data.cst_string.value;
        return true;
    }

    if (CL_TYPE_INT != cst.code || cst.data.cst_int.value)
        // no match
        return false;

    // NULL given as plot name, we're asked to generate the name automagically
    if (!loc || !loc->file) {
        // sorry, no location info here
        *dst = "anonplot";
        return true;
    }

    char *dup = strdup(loc->file);
    const char *fname = basename(dup);

    std::ostringstream str;
    str << fname << "-" << loc->line;
    *dst = str.str();

    free(dup);
    return true;
}

void emitPrototypeError(const struct cl_loc *lw, const char *name)
{
    CL_WARN_MSG(lw, "incorrectly called " << name
            << "() not recognized as built-in");
}

/// insert the given heap to dst if sane (after killing the given instruction)
void insertCoreHeap(
        SymState                                    &dst,
        SymProc                                     &core,
        const CodeStorage::Insn                     &insn)
{
    if (core.hasFatalError())
        return;

    core.killInsn(insn);

    const SymHeap &sh = core.sh();
    dst.insert(sh);
}

bool resolveCallocSize(
        TSizeRange                                  *pDst,
        SymExecCore                                 &core,
        const CodeStorage::TOperandList             &opList)
{
    SymHeap &sh = core.sh();
    const struct cl_loc *lw = core.lw();

    const TValId valNelem = core.valFromOperand(opList[/* nelem */ 2]);
    IR::Range nelem;
    if (!rngFromVal(&nelem, sh, valNelem) || nelem.lo < IR::Int0) {
        CL_ERROR_MSG(lw, "'nelem' arg of calloc() is not a known integer");
        return false;
    }

    const TValId valElsize = core.valFromOperand(opList[/* elsize */ 3]);
    IR::Range elsize;
    if (!rngFromVal(&elsize, sh, valElsize) || elsize.lo < IR::Int0) {
        CL_ERROR_MSG(lw, "'elsize' arg of calloc() is not a known integer");
        return false;
    }

    *pDst = nelem * elsize;
    return true;
}

void printUserMessage(SymProc &proc, const struct cl_operand &opMsg)
{
    const TValId valMsg = proc.valFromOperand(opMsg);

    std::string msg;
    const SymHeap &sh = proc.sh();
    if (!stringFromVal(&msg, sh, valMsg))
        // no user message available
        return;

    const struct cl_loc *loc = proc.lw();
    CL_NOTE_MSG(loc, "user message: " << msg);
}

bool validateStringOp(SymProc &proc, TOp op, TSizeRange *pSize = 0)
{
    SymHeap &sh = proc.sh();
    const struct cl_loc *loc = proc.lw();

    const TValId val = proc.valFromOperand(op);
    const TSizeRange strSize = sh.valSizeOfString(val);
    if (IR::Int0 < strSize.lo) {
        if (pSize)
            *pSize = strSize;

        return true;
    }

    if (!proc.checkForInvalidDeref(val, sizeof(char)))
        CL_ERROR_MSG(loc, "failed to imply a zero-terminated string");

    return false;
}

bool handlePlotTraceGeneric(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name,
        const bool                                   now)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_loc *lw = &insn.loc;

    if (/* dst + fnc + plot_name + node_name */ 4 != opList.size()) {
        emitPrototypeError(lw, name);
        // insufficient count of arguments
        return false;
    }

    if (CL_OPERAND_VOID != opList[/* dst */ 0].code) {
        // not a function returning void
        emitPrototypeError(lw, name);
        return false;
    }

    std::string plotName;
    if (!readPlotName(&plotName, opList, core.lw())) {
        // failed to read plot name
        emitPrototypeError(lw, name);
        return false;
    }

    // resolve name of the user node
    std::string nodeName;
    const SymHeap &sh = core.sh();
    const TValId valNodeName = core.valFromOperand(opList[/* node_name */ 3]);
    if (!stringFromVal(&nodeName, sh, valNodeName)) {
        CL_WARN_MSG(lw, name << "() failed to read node_name");
        nodeName = "NULL";
    }

    const SymExecCoreParams &ep = core.params();
    if (ep.skipPlot) {
        // we are explicitly asked to plot nothing
        CL_DEBUG_MSG(lw, name << "() skipped per user's request");
        insertCoreHeap(dst, core, insn);
        return true;
    }

    // create a user node with the specified label
    Trace::Node *trOrig = sh.traceNode();
    Trace::NodeHandle trHandle(
	    new Trace::UserNode(trOrig, &insn, nodeName.c_str()));
    CL_BREAK_IF(!chkTraceGraphConsistency(trOrig));

    if (now) {
        // dump the plot now!
        if (!Trace::plotTrace(trHandle.node(), plotName))
            CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");
    }
    else {
        // just schedule the user node as a new end-point, will be plot later...
        Trace::GraphProxy *glProxy = Trace::Globals::instance()->glProxy();
        glProxy->insert(trHandle.node(), plotName);
    }

    // built-in processed, we do not care if successfully at this point
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleNoOp(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;

    // this allows functions with 0-4 parameters
    if (opList.size() < 2 || 6 < opList.size()) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    CL_DEBUG_MSG(core.lw(), "ignoring call to " << name << "()");

    // do not change anything in the program configuration
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleExit(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;

    // this allows both abort() and exit()
    if (opList.size() < 2 || 3 < opList.size()) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    if (GlConf::data.exitLeaks) {
        // make the built-in eventually appear in the backtrace
        TempBackTraceTop tempTop(core, opList[/* fnc */ 1]);

        // report immediately visible memory leaks
        destroyProgVars(core);

        // propagate the exit point back to the caller to catch leaks in context
        core.sh().setExitPoint(core.bt());
    }

    // continue to CL_INSN_ABORT if exit() is annotated as no-return fnc
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleBreak(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 3 || opList[0].code != CL_OPERAND_VOID) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    // print what happened
    CL_WARN_MSG(&insn.loc, name << "() reached, stopping per user's request");
    printUserMessage(core, opList[/* msg */ 2]);
    core.printBackTrace(ML_WARN);

    // trap to debugger
    CL_TRAP;

    // continue with the given heap as the result
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleCalloc(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (4 != opList.size()) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    TSizeRange size;
    if (!resolveCallocSize(&size, core, opList)) {
        core.printBackTrace(ML_ERROR);
        return true;
    }

    const struct cl_loc *lw = &insn.loc;
    if (isSingular(size))
        CL_DEBUG_MSG(lw, "executing calloc(/* total size */ "<< size.lo << ")");
    else
        CL_DEBUG_MSG(lw, "executing calloc(/* size given as int range */)");

    core.execHeapAlloc(dst, insn, size, /* nullified */ true);
    return true;
}

bool handleFree(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (/* dst + fnc + ptr */ 3 != opList.size()
        || CL_OPERAND_VOID != opList[0].code)
    {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    // resolve value to be freed
    TValId val = core.valFromOperand(opList[/* ptr given to free() */2]);
    core.execFree(val);
    core.killInsn(insn);

    if (!core.hasFatalError())
        dst.insert(core.sh());

    return true;
}

bool handleKzalloc(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (4 != opList.size()) {
        emitPrototypeError(lw, name);
        return false;
    }

    // amount of allocated memory must be known (TODO: relax this?)
    const TValId valSize = core.valFromOperand(opList[/* size */ 2]);
    IR::Range size;
    if (!rngFromVal(&size, core.sh(), valSize)) {
        CL_ERROR_MSG(lw, "size arg of " << name << "() is not a known integer");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    CL_DEBUG("FIXME: flags given to " << name << "() are ignored for now");
    CL_DEBUG_MSG(lw, "modelling call of kzalloc() as call of calloc()");
    core.execHeapAlloc(dst, insn, size, /* nullified */ true);
    return true;
}

bool handleStackSave(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const TLoc loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (2 != opList.size()) {
        emitPrototypeError(loc, name);
        return false;
    }

    CL_DEBUG_MSG(loc, "ignoring call of " << name << "()");
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleStackRestore(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const TLoc loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (3 != opList.size()) {
        emitPrototypeError(loc, name);
        return false;
    }

    CL_DEBUG_MSG(loc, "executing " << name << "()");
    core.execStackRestore();
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleAlloca(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const TLoc loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;

    // this allows both __builtin_alloca(x) and __builtin_alloca_with_align(x,y)
    if (opList.size() < 3 || 4 < opList.size()) {
        emitPrototypeError(loc, name);
        return false;
    }

    // amount of allocated memory must be known (TODO: relax this?)
    const TValId valSize = core.valFromOperand(opList[/* size */ 2]);
    IR::Range size;
    if (rngFromVal(&size, core.sh(), valSize) && IR::Int0 <= size.lo) {
        CL_DEBUG_MSG(loc, "executing " << name << "()");
        core.execStackAlloc(insn.operands[/* dst */ 0], size);
    }
    else {
        CL_ERROR_MSG(loc, "size arg of " << name<< "() is not a known integer");
        core.printBackTrace(ML_ERROR);
    }

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleMalloc(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (3 != opList.size()) {
        emitPrototypeError(lw, name);
        return false;
    }

    // amount of allocated memory must be known (TODO: relax this?)
    const TValId valSize = core.valFromOperand(opList[/* size */ 2]);
    IR::Range size;
    if (!rngFromVal(&size, core.sh(), valSize) || size.lo < IR::Int0) {
        CL_ERROR_MSG(lw, "size arg of malloc() is not a known integer");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    if (isSingular(size))
        CL_DEBUG_MSG(lw, "executing malloc(" << size.lo << ")");
    else
        CL_DEBUG_MSG(lw, "executing malloc(/* size given as int range */)");

    core.execHeapAlloc(dst, insn, size, /* nullified */ false);
    return true;
}

bool handleRealloc(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (/* dst + fnc + ptr + new_size */4 != opList.size()) {
        emitPrototypeError(lw, name);
        return false;
    }

    // amount of allocated memory must be known (TODO: relax this?)
    const TValId valSize = core.valFromOperand(opList[/* new_size */ 3]);
    IR::Range size;
    if (!rngFromVal(&size, core.sh(), valSize) || size.lo < IR::Int0) {
        CL_ERROR_MSG(lw, "new_size arg of realloc() is not a known integer");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    if (isSingular(size))
        CL_DEBUG_MSG(lw, "executing realloc(ptr, " << size.lo << ")");
    else
        CL_DEBUG_MSG(lw, "executing realloc(ptr, /* size given as int range */)");

    core.execHeapRealloc(dst, insn, size);

    return true;
}

/// common code-base for memcpy() and memmove() built-in handlers
bool handleMemmoveCore(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name,
        const bool                                   allowOverlap)
{
    const struct cl_loc *loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (5 != opList.size()
            && /* in GCC + check */ 6 != opList.size()
            && /* in LLVM + align + volatile */ 7 != opList.size())
    {
        emitPrototypeError(loc, name);
        return false;
    }

    // read the values of memmove parameters
    const TValId valDst     = core.valFromOperand(opList[/* dst  */ 2]);
    const TValId valSrc     = core.valFromOperand(opList[/* src  */ 3]);
    const TValId valSize    = core.valFromOperand(opList[/* size */ 4]);

    CL_DEBUG_MSG(loc, "executing memcpy() or memmove() as a built-in function");
    executeMemmove(core, valDst, valSrc, valSize, allowOverlap);

    const struct cl_operand &opDst = opList[/* ret */ 0];
    if (CL_OPERAND_VOID != opDst.code) {
        // POSIX says that memmove() returns the value of the first argument
        const FldHandle fldDst = core.fldByOperand(opDst);
        core.setValueOf(fldDst, valDst);
    }

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleMemcpy(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    return handleMemmoveCore(dst, core, insn, name, /* allowOverlap */ false);
}

bool handleMemmove(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    return handleMemmoveCore(dst, core, insn, name, /* allowOverlap */ true);
}

bool handleMemset(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (5 != opList.size()
            && /* in GCC + check */ 6 != opList.size()
            && /* in LLVM + align + volatile */ 7 != opList.size())
    {
        emitPrototypeError(lw, name);
        return false;
    }

    // read the values of memset parameters
    const TValId addr       = core.valFromOperand(opList[/* addr */ 2]);
    const TValId valToWrite = core.valFromOperand(opList[/* char */ 3]);
    const TValId valSize    = core.valFromOperand(opList[/* size */ 4]);

    CL_DEBUG_MSG(lw, "executing memset() as a built-in function");
    executeMemset(core, addr, valToWrite, valSize);

    const struct cl_operand &opDst = opList[/* dst */ 0];
    if (CL_OPERAND_VOID != opDst.code) {
        // POSIX says that memset() returns the value of the first argument
        const FldHandle fldDst = core.fldByOperand(opDst);
        core.setValueOf(fldDst, addr);
    }

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handlePrintf(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    SymHeap &sh = core.sh();
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() < 3) {
        emitPrototypeError(lw, name);
        return false;
    }

    const TValId valFmt = core.valFromOperand(opList[/* fmt */ 2]);
    std::string fmtStr;
    if (!stringFromVal(&fmtStr, sh, valFmt)) {
        CL_ERROR_MSG(lw, "fmt arg of printf() is not a string literal");
        core.printBackTrace(ML_ERROR);
        insertCoreHeap(dst, core, insn);
        return true;
    }

    unsigned opIdx = /* 1st vararg */ 3;

    char *fmtAlloc = strdup(fmtStr.c_str());
    const char *fmt = fmtAlloc;
    while (*fmt) {
        if ('%' != *(fmt++))
            continue;

        char c = *fmt;
        if ('%' == c) {
            // %% -> keep going...
            ++fmt;
            continue;
        }

        if (opList.size() <= opIdx) {
            CL_ERROR_MSG(lw, "insufficient count of arguments given to printf()");
            goto fail;
        }

        // skip [0-9.lz]+
        while (isdigit(*fmt) || '.' == *fmt || 'l' == *fmt || 'z' == *fmt)
            ++fmt;
        c = *fmt;
        switch (c) {
            case 'A': case 'E': case 'F': case 'G':
            case 'a': case 'c': case 'd': case 'e': case 'f': case 'g':
            case 'i': case 'o': case 'p': case 'u': case 'x': case 'X':
                // we are not interested in numbers when checking memory safety
                break;

            case 's':
                // %s
                if (validateStringOp(core, opList[opIdx]))
                    break;
                else
                    goto fail;

            default:
                CL_ERROR_MSG(lw, "unhandled conversion given to printf()");
                goto fail;
        }

        // next conversion -> next operand
        ++opIdx;
    }

    if (opIdx < opList.size()) {
        // this is quite suspicious, but would not crash the program
        CL_WARN_MSG(lw, "too many arguments given to printf()");
        core.printBackTrace(ML_WARN);
    }

    free(fmtAlloc);
    insertCoreHeap(dst, core, insn);
    return true;

fail:
    free(fmtAlloc);
    core.printBackTrace(ML_ERROR);
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handlePuts(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 3) {
        emitPrototypeError(lw, name);
        return false;
    }

    if (!validateStringOp(core, opList[/* s */ 2]))
        core.printBackTrace(ML_ERROR);

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleStrlen(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 3) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    TSizeRange len;
    if (validateStringOp(core, opList[/* s */ 2], &len)) {
        const struct cl_operand &opDst = opList[/* ret */ 0];
        if (CL_OPERAND_VOID != opDst.code) {
            // store the return value of strlen()
            const CustomValue cv(len - IR::rngFromNum(IR::Int1));
            const TValId valResult = core.sh().valWrapCustom(cv);
            const FldHandle fldDst = core.fldByOperand(opDst);
            core.setValueOf(fldDst, valResult);
        }
    }
    else
        core.printBackTrace(ML_ERROR);

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleStrcmp(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const TLoc loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 4) {
        emitPrototypeError(loc, name);
        return false;
    }

    const struct cl_operand &opDst = opList[/* ret */ 0];

    // read the values of strcmp parameters
    const TValId valStr1  = core.valFromOperand(opList[/* str1 */ 2]);
    const TValId valStr2  = core.valFromOperand(opList[/* str2 */ 3]);

    SymHeap &sh = core.sh();
    std::string str1;
    std::string str2;
    if (!sh.valString(valStr1, str1) || !sh.valString(valStr2, str2)) {
        CL_ERROR_MSG(loc, "args of " << name << "() are not string literals");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    const int code = std::strcmp(str1.c_str(), str2.c_str());

    if (CL_OPERAND_VOID != opDst.code) {
        // store the return value of strcmp()
        const CustomValue cv(IR::rngFromNum(code));
        const TValId valResult = sh.valWrapCustom(cv);
        const FldHandle fldDst = core.fldByOperand(opDst);
        core.setValueOf(fldDst, valResult);
    }

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleStrncpy(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const TLoc loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 5
            && /* in GCC + check */ 6 != opList.size()) {
        emitPrototypeError(loc, name);
        return false;
    }

    // read the values of strncpy parameters
    const TValId valDst  = core.valFromOperand(opList[/* dst */ 2]);
    const TValId valSrc  = core.valFromOperand(opList[/* src */ 3]);
    const TValId valSize = core.valFromOperand(opList[/* n   */ 4]);

    SymHeap &sh = core.sh();

    IR::Range size;
    if (!rngFromVal(&size, sh, valSize) || size.lo < IR::Int0) {
        CL_ERROR_MSG(loc, "n arg of " << name << "() is not a known integer");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    const TSizeRange strLen = sh.valSizeOfString(valSrc);
    const bool isValidString = (IR::Int0 < strLen.lo);
    const TSizeOf srcLimit = (isValidString)
        ? strLen.hi
        : size.hi;

    if (core.checkForInvalidDeref(valSrc, srcLimit)) {
        // error message already printed out
        core.printBackTrace(ML_ERROR);
        insertCoreHeap(dst, core, insn);
        return true;
    }

    if (isValidString) {
        CL_DEBUG("strncpy() writes zeros");
        executeMemset(core, valDst, VAL_NULL, valSize);

        CL_DEBUG("strncpy() transfers the data");
        const CustomValue cVal(strLen);
        const TValId valLen = sh.valWrapCustom(cVal);
        executeMemmove(core, valDst, valSrc, valLen, /* allowOverlap */ false);
    }
    else {
        CL_DEBUG("strncpy() only invalidates the given range");
        const TValId valUnknown = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        executeMemset(core, valDst, valUnknown, valSize);
    }

    const struct cl_operand &opDst = opList[/* ret */ 0];
    if (CL_OPERAND_VOID != opDst.code) {
        // strncpy() returns the value of the first argument
        const FldHandle fldDst = core.fldByOperand(opDst);
        core.setValueOf(fldDst, valDst);
    }

    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleExpect(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    // exp = __builtin_expect (exp, c); test condition exp, expected value c
    const CodeStorage::TOperandList &opList = insn.operands;
    if (/* dst + fnc + exp + c */ 4 != opList.size())
    {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    const TValId val = core.valFromOperand(opList[/* exp */2]);
    CL_DEBUG_MSG(&insn.loc, "executing " << name << "()");

    // set the returned value to a condition
    const struct cl_operand &opDst = opList[0];
    const FldHandle fldDst = core.fldByOperand(opDst);
    core.setValueOf(fldDst, val);

    // insert the resulting heap
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleAssume(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    SymHeap &sh = core.sh();
    const TLoc loc = core.lw();
    const CodeStorage::TOperandList &opList = insn.operands;

    if (3 != opList.size() || opList[0].code != CL_OPERAND_VOID) {
        emitPrototypeError(loc, name);
        return false;
    }

    const TValId valExpr = core.valFromOperand(opList[/* expr */ 2]);
    const TValId valComp = compareValues(sh, CL_BINOP_EQ, VAL_FALSE, valExpr);

    if (proveNeq(sh, VAL_FALSE, valComp)) {
        CL_DEBUG_MSG(loc, name << "() got FALSE, skipping this code path!");
        return true;
    }

    // TODO: it would be nice to call reflectCmpResult() in certain cases

    // insert the resulting heap
    CL_DEBUG_MSG(loc, name << "() failed to prove inconsistency");
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handleNondetInt(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (2 != opList.size()) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    SymHeap &sh = core.sh();
    CL_DEBUG_MSG(&insn.loc, "executing " << name << "()");
    TValId val;

    static const char namePrefixUnsigned[] = "__VERIFIER_nondet_u";
    static const size_t namePrefixLength = sizeof(namePrefixUnsigned) - 1U;
    std::string namePrefix(name);
    if (namePrefixLength < namePrefix.size())
        namePrefix.resize(namePrefixLength);

    if (std::string(namePrefixUnsigned) == namePrefix) {
        // an unsigned value
        const IR::Range unsignedRng = {
            /* lo        */ 0,
            /* hi        */ IR::IntMax,
            /* alignment */ IR::Int1
        };
        const CustomValue cv(unsignedRng);
        val = sh.valWrapCustom(cv);
    }
    else {
        // an unknown value
        val = sh.valCreate(VT_UNKNOWN, VO_ASSIGNED);
    }

    // set the returned value to a new value
    const struct cl_operand &opDst = opList[0];
    const FldHandle fldDst = core.fldByOperand(opDst);
    core.setValueOf(fldDst, val);

    // insert the resulting heap
    dst.insert(sh);
    return true;
}

bool handlePlot(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_loc *lw = &insn.loc;

    const int cntArgs = opList.size() - /* dst + fnc */ 2;
    if (cntArgs < 1) {
        emitPrototypeError(lw, name);
        // insufficient count of arguments
        return false;
    }

    if (CL_OPERAND_VOID != opList[/* dst */ 0].code) {
        // not a function returning void
        emitPrototypeError(lw, name);
        return false;
    }

    std::string plotName;
    if (!readPlotName(&plotName, opList, core.lw())) {
        emitPrototypeError(lw, name);
        return false;
    }

    const SymExecCoreParams &ep = core.params();
    if (ep.skipPlot) {
        CL_DEBUG_MSG(lw, name << "() skipped per user's request");
        insertCoreHeap(dst, core, insn);
        return true;
    }

    const SymHeap &sh = core.sh();
    bool ok;

    if (1 == cntArgs)
        ok = plotHeap(sh, plotName, lw);

    else {
        // starting points were given
        TValList startingPoints;
        for (int i = 1; i < cntArgs; ++i) {
            const struct cl_operand &op = opList[i + /* dst + fnc */ 2];
            const TValId val = core.valFromOperand(op);
            startingPoints.push_back(val);
        }

        ok = plotHeap(sh, plotName, lw, startingPoints);
    }

    if (!ok)
        CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");

    // built-in processed, we do not care if successfully at this point
    insertCoreHeap(dst, core, insn);
    return true;
}

bool handlePlotTraceNow(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    return handlePlotTraceGeneric(dst, core, insn, name, /* now */ true);
}

bool handlePlotTraceOnce(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    return handlePlotTraceGeneric(dst, core, insn, name, /* now */ false);
}

bool handleDebuggingOf(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_loc *lw = &insn.loc;

    if (/* dst + fnc + module + enable */ 4 != opList.size()) {
        emitPrototypeError(lw, name);
        return false;
    }

    IR::TInt module;
    const SymHeap &sh = core.sh();
    const TValId valModule = core.valFromOperand(opList[/* module */ 2]);
    if (!numFromVal(&module, sh, valModule))
        module = 0L;

    const TValId valEnable = core.valFromOperand(opList[/* enable */ 3]);
    const bool enable = proveNeq(sh, VAL_FALSE, valEnable);

    switch (module) {
        case ___SL_SYMABSTRACT:
            debugSymAbstract(enable);
            break;

        case ___SL_SYMJOIN:
            debugSymJoin(enable);
            break;

        case ___SL_GARBAGE_COLLECTOR:
            debugGarbageCollector(enable);
            break;

        case ___SL_EVERYTHING:
        default:
            debugSymAbstract(enable);
            debugSymJoin(enable);
            debugGarbageCollector(enable);
    }

    insertCoreHeap(dst, core, insn);
    return true;
}

// void __assert_fail (
//      const char     *cond,
//      const char     *file,
//      unsigned int    line,
//      const char     *fnc);
bool handleAssertFail(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *loc = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() < 3) {
        // no args given
        emitPrototypeError(loc, name);
        return false;
    }

    SymHeap &sh = core.sh();
    if (GlConf::data.exitLeaks) {
        // make the built-in eventually appear in the backtrace
        TempBackTraceTop tempTop(core, opList[/* fnc */ 1]);

        // report immediately visible memory leaks
        destroyProgVars(core);

        // propagate the exit point back to the caller to catch leaks in context
        sh.setExitPoint(core.bt());
        insertCoreHeap(dst, core, insn);
    }

    // obtain the condition string (hopefully given as string literal)
    std::string strCond;
    const TValId valCond = core.valFromOperand(opList[/* cond */ 2]);
    if (!stringFromVal(&strCond, sh, valCond))
        strCond = "<unknown>";

    // print the error message
    CL_ERROR_MSG(loc, "assertion failed: " << strCond);

    // print backtrace
    core.printBackTrace(ML_ERROR);
    return true;
}

bool handleError(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *loc = &insn.loc;

    // unlike ___sl_error(), __VERIFIER_error() does not take any argument
    const bool isVerifierError = STREQ(name, "__VERIFIER_error");
    const unsigned opCntExpected = (isVerifierError) ? 2 : 3;

    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != opCntExpected || opList[0].code != CL_OPERAND_VOID) {
        emitPrototypeError(loc, name);
        return false;
    }

    if (isVerifierError && GlConf::data.exitLeaks) {
        // make the built-in eventually appear in the backtrace
        TempBackTraceTop tempTop(core, opList[/* fnc */ 1]);

        // report immediately visible memory leaks
        destroyProgVars(core);

        // propagate the exit point back to the caller to catch leaks in context
        core.sh().setExitPoint(core.bt());
        insertCoreHeap(dst, core, insn);
    }

    if (isVerifierError && !GlConf::data.verifierErrorIsError) {
        // __VERIFIER_error() called but user does not treat it as error
        CL_WARN_MSG(loc, name
            << "() reached, analysis of this code path will not continue");
        return true;
    }

    // print the error message
    CL_ERROR_MSG(loc, name
            << "() reached, analysis of this code path will not continue");

    if (!isVerifierError)
        // print the user message
        printUserMessage(core, opList[/* msg */ 2]);

    // print backtrace
    core.printBackTrace(ML_ERROR);
    return true;
}

// singleton
class BuiltInTable {
    public:
        typedef const CodeStorage::Insn             &TInsn;

    public:
        static BuiltInTable* inst() {
            return (inst_)
                ? (inst_)
                : (inst_ = new BuiltInTable);
        }

        bool handleBuiltIn(
                SymState                            &dst,
                SymExecCore                         &core,
                TInsn                                insn,
                const char                          *name)
            const;

        const TOpIdxList& lookForDerefs(const char *name) const;

        // TODO: rename and hide
        const TOpIdxList                            emp_;

    private:
        BuiltInTable();

        static BuiltInTable* inst_;

        typedef bool (*THandler)(
                SymState                            &dst,
                SymExecCore                         &core,
                const CodeStorage::Insn             &insn,
                const char                          *name);

        typedef std::map<std::string, THandler>     TMap;
        TMap                                        tbl_;

        typedef std::map<std::string, TOpIdxList>   TDerefMap;
        TDerefMap                                   der_;
};

BuiltInTable *BuiltInTable::inst_;

/// register built-ins
BuiltInTable::BuiltInTable()
{
    // GCC built-in stack allocation
    tbl_["__builtin_alloca"] /* before GCC 4.7.0 */ = handleAlloca;
    tbl_["__builtin_alloca_with_align"]             = handleAlloca;
    tbl_["__builtin_malloc"]                        = handleMalloc;
    tbl_["__builtin_stack_restore"]                 = handleStackRestore;
    tbl_["__builtin_stack_save"]                    = handleStackSave;

    // LLVM built-in stack allocation
    tbl_["llvm.stackrestore"]                       = handleStackRestore;
    tbl_["llvm.stacksave"]                          = handleStackSave;

    // C run-time
    tbl_["abort"]                                   = handleExit;
    tbl_["alloca"]                                  = handleAlloca;
    tbl_["calloc"]                                  = handleCalloc;
    tbl_["exit"]                                    = handleExit;
    tbl_["free"]                                    = handleFree;
    tbl_["malloc"]                                  = handleMalloc;
    tbl_["memcpy"]                                  = handleMemcpy;
    tbl_["memmove"]                                 = handleMemmove;
    tbl_["memset"]                                  = handleMemset;
    tbl_["printf"]                                  = handlePrintf;
    tbl_["puts"]                                    = handlePuts;
    tbl_["realloc"]                                 = handleRealloc;
    tbl_["strcmp"]                                  = handleStrcmp;
    tbl_["strlen"]                                  = handleStrlen;
    tbl_["strncpy"]                                 = handleStrncpy;

    // <assert.h>
    tbl_["__assert_fail"]                           = handleAssertFail;

    // GCC-specific built-in functions
    tbl_["__builtin_expect"]                        = handleExpect;
    tbl_["__builtin_object_size"]                   = handleNoOp;
    tbl_["__builtin___memcpy_chk"]                  = handleMemcpy;
    tbl_["__builtin___memmove_chk"]                 = handleMemmove;
    tbl_["__builtin___memset_chk"]                  = handleMemset;
    tbl_["__builtin_puts"]                          = handlePuts;
    tbl_["__builtin___strncpy_chk"]                 = handleStrncpy;
    tbl_["__builtin_strncpy"]                       = handleStrncpy;
    tbl_["__memcpy_chk"]                            = handleMemcpy;
    tbl_["__memset_chk"]                            = handleMemset;

    // LLVM-specific built-in functions
    tbl_["llvm.memcpy"]                             = handleMemcpy;
    tbl_["llvm.memmove"]                            = handleMemmove;
    tbl_["llvm.memset"]                             = handleMemset;
    tbl_["__strncpy_chk"]                           = handleStrncpy;

    // Linux kernel
    tbl_["kzalloc"]                                 = handleKzalloc;
    tbl_["printk"]                                  = handlePrintf;

    // Predator-specific
    tbl_["___sl_break"]                             = handleBreak;
    tbl_["___sl_error"]                             = handleError;
    tbl_["___sl_get_nondet_int"]                    = handleNondetInt;
    tbl_["__VERIFIER_plot"]                         = handlePlot;
    tbl_["___sl_plot_trace_now"]                    = handlePlotTraceNow;
    tbl_["___sl_plot_trace_once"]                   = handlePlotTraceOnce;
    tbl_["___sl_enable_debugging_of"]               = handleDebuggingOf;

    // used in the Competition on Software Verification held at TACAS
    tbl_["__VERIFIER_assume"]                       = handleAssume;
    tbl_["__VERIFIER_error"]                        = handleError;
    //    __VERIFIER_nondet_*   functions are handled in the above layer

    // just to make life easier to our competitors (TODO: check for collisions)
    tbl_["__nondet"]                                = handleNondetInt;
    tbl_["ldv_initialize"]                          = handleNoOp;
    tbl_["ldv_undefined_int"]                       = handleNondetInt;
    tbl_["nondet_int"]                              = handleNondetInt;
    tbl_["undef_int"]                               = handleNondetInt;

    // initialize lookForDerefs() look-up table
    der_["free"]                   .push_back(/* addr */ 2);
    der_["memcpy"]                 .push_back(/* dst  */ 2);
    der_["memcpy"]                 .push_back(/* src  */ 3);
    der_["memmove"]                .push_back(/* dst  */ 2);
    der_["memmove"]                .push_back(/* src  */ 3);
    der_["memset"]                 .push_back(/* addr */ 2);
    der_["__builtin___memcpy_chk"] .push_back(/* dst  */ 2);
    der_["__builtin___memcpy_chk"] .push_back(/* src  */ 3);
    der_["__builtin___memmove_chk"].push_back(/* dst  */ 2);
    der_["__builtin___memmove_chk"].push_back(/* src  */ 3);
    der_["__builtin___memset_chk"] .push_back(/* addr */ 2);
    der_["__memcpy_chk"]           .push_back(/* dst  */ 2);
    der_["__memcpy_chk"]           .push_back(/* src  */ 3);
    der_["__memset_chk"]           .push_back(/* addr */ 2);
    der_["llvm.memcpy"]            .push_back(/* dst  */ 2);
    der_["llvm.memcpy"]            .push_back(/* src  */ 3);
    der_["llvm.memmove"]           .push_back(/* dst  */ 2);
    der_["llvm.memmove"]           .push_back(/* src  */ 3);
    der_["llvm.memset"]            .push_back(/* addr */ 2);
    // TODO: printf
    der_["puts"]                   .push_back(/* s    */ 2);
    der_["realloc"]                .push_back(/* addr */ 2);
    der_["strcmp"]                 .push_back(/* str1 */ 2);
    der_["strcmp"]                 .push_back(/* str2 */ 3);
    der_["strlen"]                 .push_back(/* s    */ 2);
    der_["strncpy"]                .push_back(/* dst  */ 2);
    der_["strncpy"]                .push_back(/* src  */ 3);
    der_["__builtin___strncpy_chk"].push_back(/* dst  */ 2);
    der_["__builtin___strncpy_chk"].push_back(/* src  */ 3);
    der_["__builtin_strncpy"]      .push_back(/* dst  */ 2);
    der_["__builtin_strncpy"]      .push_back(/* src  */ 3);
    der_["__strncpy_chk"]          .push_back(/* dst  */ 2);
    der_["__strncpy_chk"]          .push_back(/* src  */ 3);
}

bool BuiltInTable::handleBuiltIn(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
    const
{
    THandler hdl;

    TMap::const_iterator it = tbl_.find(name);
    if (tbl_.end() == it) {
        static const char namePrefixNondet[] = "__VERIFIER_nondet";
        static const char namePrefixObjSize[] = "llvm.objectsize.i";

        static_assert(sizeof(namePrefixNondet) == sizeof(namePrefixObjSize),
            "Prefix names must be same length");

        static const size_t namePrefixLength = sizeof(namePrefixNondet) - 1U;
        std::string namePrefix(name);
        if (namePrefixLength < namePrefix.size())
            namePrefix.resize(namePrefixLength);

        if (std::string(namePrefixNondet) == namePrefix)
            hdl = handleNondetInt;
        else if (std::string(namePrefixObjSize) == namePrefix)
            hdl = handleNoOp;
        else
            // no fnc name matched as built-in
            return false;
    }
    else
        hdl = it->second;

    SymHeap &sh = core.sh();
    sh.traceUpdate(new Trace::InsnNode(sh.traceNode(), &insn, /* bin */ true));

    return hdl(dst, core, insn, name);
}

const TOpIdxList& BuiltInTable::lookForDerefs(const char *name) const
{
    TDerefMap::const_iterator it = der_.find(name);
    if (der_.end() == it)
        // no fnc name matched as built-in
        return emp_;

    return it->second;
}

bool fncNameFromOp(
        const char                                **pName,
        SymExecCore                                 &core,
        const struct cl_operand                     &op)
{
    cl_uid_t uid;
    if (!core.fncFromOperand(&uid, op))
        return false;

    TStorRef stor = core.sh().stor();
    const CodeStorage::Fnc *fnc = stor.fncs[uid];
    if (!fnc->def.data.cst.data.cst_fnc.is_extern)
        // only external functions are candidates for built-in functions
        return false;

    const char *name = nameOf(*fnc);
    if (!name)
        return false;

    *pName = name;
    return true;
}

bool handleBuiltIn(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn)
{
    const char *name;
    if (!fncNameFromOp(&name, core, insn.operands[/* fnc */ 1]))
        return false;

    const BuiltInTable *tbl = BuiltInTable::inst();
    return tbl->handleBuiltIn(dst, core, insn, name);
}

const TOpIdxList& opsWithDerefSemanticsInCallInsn(
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn)
{
    const BuiltInTable *tbl = BuiltInTable::inst();

    const char *name;
    if (!fncNameFromOp(&name, core, insn.operands[/* fnc */ 1]))
        return tbl->emp_;

    return tbl->lookForDerefs(name);
}
