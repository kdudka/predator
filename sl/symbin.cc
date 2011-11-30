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

#include "symabstract.hh"
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

    if (CL_TYPE_PTR != op.type->code
            || CL_TYPE_INT != cst.code
            || cst.data.cst_int.value)
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

void emitPrototypeError(const struct cl_loc *lw, const char *name) {
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
        TSizeOf                                     *pDst,
        SymExecCore                                 &core,
        const CodeStorage::TOperandList             &opList)
{
    SymHeap &sh = core.sh();
    const struct cl_loc *lw = core.lw();

    const TValId valNelem = core.valFromOperand(opList[/* nelem */ 2]);
    IR::TInt nelem;
    if (!numFromVal(&nelem, sh, valNelem)) {
        CL_ERROR_MSG(lw, "'nelem' arg of calloc() is not a known integer");
        return false;
    }

    const TValId valElsize = core.valFromOperand(opList[/* elsize */ 3]);
    IR::TInt elsize;
    if (!numFromVal(&elsize, sh, valElsize)) {
        CL_ERROR_MSG(lw, "'elsize' arg of calloc() is not a known integer");
        return false;
    }

    *pDst = nelem * elsize;
    return true;
}

void printUserMessage(SymProc &proc, const struct cl_operand &opMsg)
{
    const TValId valMsg = proc.valFromOperand(opMsg);

    const char *msg;
    const SymHeap &sh = proc.sh();
    if (!stringFromVal(&msg, sh, valMsg))
        // no user message available
        return;

    const struct cl_loc *loc = proc.lw();
    CL_NOTE_MSG(loc, "user message: " << msg);
}

bool validateStringOp(SymProc &proc, const struct cl_operand &op) {
    const TValId val = proc.valFromOperand(op);

    SymHeap &sh = proc.sh();
    const EValueTarget code = sh.valTarget(val);

    if (VT_CUSTOM == code) {
        if (CV_STRING == sh.valUnwrapCustom(val).code)
            // string literal
            return true;

        // TODO
    }

    // TODO
    CL_ERROR_MSG(proc.lw(), "string validation not implemented yet");
    CL_BREAK_IF("please implement");
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
    const char *nodeName;
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
    Trace::NodeHandle trHandle(new Trace::UserNode(trOrig, &insn, nodeName));
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

bool handleAbort(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 2 || opList[0].code != CL_OPERAND_VOID) {
        emitPrototypeError(&insn.loc, name);
        return false;
    }

    // do nothing for abort()
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

    TSizeOf size;
    if (!resolveCallocSize(&size, core, opList)) {
        core.printBackTrace(ML_ERROR);
        return true;
    }

    const struct cl_loc *lw = &insn.loc;
    CL_DEBUG_MSG(lw, "executing calloc(/* total size */ " << size << ")");
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
    IR::TInt size;
    if (!numFromVal(&size, core.sh(), valSize)) {
        CL_ERROR_MSG(lw, "size arg of " << name << "() is not a known integer");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    CL_DEBUG("FIXME: flags given to " << name << "() are ignored for now");
    CL_DEBUG_MSG(lw, "executing calloc(/* total size */ " << size << ")");
    core.execHeapAlloc(dst, insn, size, /* nullified */ true);
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
    IR::TInt size;
    if (!numFromVal(&size, core.sh(), valSize)) {
        CL_ERROR_MSG(lw, "size arg of malloc() is not a known integer");
        core.printBackTrace(ML_ERROR);
        return true;
    }

    CL_DEBUG_MSG(lw, "executing malloc(" << size << ")");
    core.execHeapAlloc(dst, insn, size, /* nullified */ false);
    return true;
}

bool handleMemset(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (5 != opList.size()) {
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
        const ObjHandle objDst = core.objByOperand(opDst);
        core.objSetValue(objDst, addr);
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
    const char *fmt;
    if (!stringFromVal(&fmt, sh, valFmt)) {
        CL_ERROR_MSG(lw, "fmt arg of printf() is not a string literal");
        core.printBackTrace(ML_ERROR);
        insertCoreHeap(dst, core, insn);
        return true;
    }

    unsigned opIdx = /* 1st vararg */ 3;

    while (*fmt) {
        if ('%' != *(fmt++))
            continue;

        const char c = *fmt;
        if ('%' == c) {
            // %% -> keep going...
            ++fmt;
            continue;
        }

        if (opList.size() <= opIdx) {
            CL_ERROR_MSG(lw, "insufficient count of arguments given to printf()");
            goto fail;
        }

        // skip [0-9.l]+
        while (isdigit(*fmt) || '.' == *fmt || 'l' == *fmt)
            ++fmt;

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

    insertCoreHeap(dst, core, insn);
    return true;

fail:
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

    // set the returned value to a new unknown value
    const struct cl_operand &opDst = opList[0];
    const ObjHandle objDst = core.objByOperand(opDst);
    const TValId val = sh.valCreate(VT_UNKNOWN, VO_ASSIGNED);
    core.objSetValue(objDst, val);

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
    const bool enable = sh.proveNeq(VAL_FALSE, valEnable);

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

bool handleError(
        SymState                                    & /* dst */,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
{
    const struct cl_loc *loc = &insn.loc;

    const CodeStorage::TOperandList &opList = insn.operands;
    if (opList.size() != 3 || opList[0].code != CL_OPERAND_VOID) {
        emitPrototypeError(loc, name);
        return false;
    }

    // print the error message
    CL_ERROR_MSG(loc, name
            << "() reached, analysis of this code path will not continue");

    // print the user message and backtrace
    printUserMessage(core, opList[/* msg */ 2]);
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

        const TOpIdxList& lookForDerefs(TInsn) const;

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
        const TOpIdxList                            emp_;
};

BuiltInTable *BuiltInTable::inst_;

/// register built-ins
BuiltInTable::BuiltInTable() {
    // C run-time
    tbl_["abort"]                                   = handleAbort;
    tbl_["calloc"]                                  = handleCalloc;
    tbl_["free"]                                    = handleFree;
    tbl_["malloc"]                                  = handleMalloc;
    tbl_["memset"]                                  = handleMemset;
    tbl_["printf"]                                  = handlePrintf;
    tbl_["puts"]                                    = handlePuts;

    // Linux kernel
    tbl_["kzalloc"]                                 = handleKzalloc;

    // Predator-specific
    tbl_["___sl_break"]                             = handleBreak;
    tbl_["___sl_error"]                             = handleError;
    tbl_["___sl_get_nondet_int"]                    = handleNondetInt;
    tbl_["___sl_plot"]                              = handlePlot;
    tbl_["___sl_plot_trace_now"]                    = handlePlotTraceNow;
    tbl_["___sl_plot_trace_once"]                   = handlePlotTraceOnce;
    tbl_["___sl_enable_debugging_of"]               = handleDebuggingOf;

    // used in Competition on Software Verification held at TACAS 2012
    tbl_["__VERIFIER_nondet_char"]                  = handleNondetInt;
    tbl_["__VERIFIER_nondet_float"]                 = handleNondetInt;
    tbl_["__VERIFIER_nondet_int"]                   = handleNondetInt;
    tbl_["__VERIFIER_nondet_pointer"]               = handleNondetInt;
    tbl_["__VERIFIER_nondet_short"]                 = handleNondetInt;

    // just to make life easier to our competitors (TODO: check for collisions)
    tbl_["__nondet"]                                = handleNondetInt;
    tbl_["nondet_int"]                              = handleNondetInt;
    tbl_["undef_int"]                               = handleNondetInt;

    // initialize lookForDerefs() look-up table
    der_["free"]        .push_back(/* addr */ 2);
    der_["memset"]      .push_back(/* addr */ 2);
}

bool BuiltInTable::handleBuiltIn(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn,
        const char                                  *name)
    const
{
    TMap::const_iterator it = tbl_.find(name);
    if (tbl_.end() == it)
        // no fnc name matched as built-in
        return false;

    SymHeap &sh = core.sh();
    sh.traceUpdate(new Trace::InsnNode(sh.traceNode(), &insn, /* bin */ true));

    const THandler hdl = it->second;
    return hdl(dst, core, insn, name);
}

const TOpIdxList& BuiltInTable::lookForDerefs(TInsn insn) const {
    const char *name;
    if (!fncNameFromCst(&name, &insn.operands[/* fnc */ 1]))
        return emp_;

    TDerefMap::const_iterator it = der_.find(name);
    if (der_.end() == it)
        // no fnc name matched as built-in
        return emp_;

    return it->second;
}

bool handleBuiltIn(
        SymState                                    &dst,
        SymExecCore                                 &core,
        const CodeStorage::Insn                     &insn)
{
    const char *name;
    if (!fncNameFromCst(&name, &insn.operands[/* fnc */ 1]))
        return false;

    const BuiltInTable *tbl = BuiltInTable::inst();
    return tbl->handleBuiltIn(dst, core, insn, name);
}

const TOpIdxList& opsWithDerefSemanticsInCallInsn(const CodeStorage::Insn &insn)
{
    const BuiltInTable *tbl = BuiltInTable::inst();
    return tbl->lookForDerefs(insn);
}
