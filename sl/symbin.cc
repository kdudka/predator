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
#include "sl.h"
#undef PREDATOR

#include "symabstract.hh"
#include "symbt.hh"
#include "symgc.hh"
#include "symjoin.hh"
#include "symplot.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "util.hh"

#include <cstring>
#include <libgen.h>
#include <map>

template <int NTH, class TOpList>
bool readPlotName(std::string *dst, const TOpList opList,
                  const struct cl_loc *loc)
{
    const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
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

void emitPrototypeError(const struct cl_loc *lw, const std::string &fnc) {
    CL_WARN_MSG(lw, "incorrectly called "
            << fnc << "() not recognized as built-in");
}

void emitPlotError(const struct cl_loc *lw, const std::string &plotName) {
    CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");
}

void insertCoreHeap(
        SymState                                    &dst,
        SymProc                                     &core,
        const CodeStorage::Insn                     &insn,
        const bool                                  printBt = false)
{
    if (printBt)
        printBackTrace(core);

    core.killInsn(insn);

    const SymHeap &sh = core.sh();
    dst.insert(sh);
}

bool callPlot(const CodeStorage::Insn &insn, SymProc &proc) {
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_loc *lw = &insn.loc;

    const int cntArgs = opList.size() - /* dst + fnc */ 2;
    if (cntArgs < 1) {
        emitPrototypeError(lw, "___sl_plot");
        // insufficient count of arguments
        return false;
    }

    if (CL_OPERAND_VOID != opList[/* dst */ 0].code) {
        // not a function returning void
        emitPrototypeError(lw, "___sl_plot");
        return false;
    }

    std::string plotName;
    if (!readPlotName<0>(&plotName, opList, proc.lw())) {
        emitPrototypeError(lw, "___sl_plot");
        return false;
    }

    const SymHeap &sh = proc.sh();

    if (1 == cntArgs) {
        if (!plotHeap(sh, plotName))
            emitPlotError(lw, plotName);
    }
    else {
        // starting points were given
        TValList startingPoints;
        for (int i = 1; i < cntArgs; ++i) {
            const struct cl_operand &op = opList[i + /* dst + fnc */ 2];
            const TValId val = proc.valFromOperand(op);
            startingPoints.push_back(val);
        }

        if (!plotHeap(sh, plotName, startingPoints))
            emitPlotError(lw, plotName);
    }

    // built-in processed, we do not care if successfully at this point
    return true;
}

bool resolveCallocSize(
        unsigned                                    *pDst,
        SymExecCore                                 &core,
        const CodeStorage::TOperandList             &opList)
{
    SymHeap &sh = core.sh();
    const struct cl_loc *lw = core.lw();

    const TValId valNelem = core.valFromOperand(opList[/* nelem */ 2]);
    long nelem;
    if (!numFromVal(&nelem, sh, valNelem)) {
        CL_ERROR_MSG(lw, "'nelem' arg of calloc() is not a known integer");
        return false;
    }

    const TValId valElsize = core.valFromOperand(opList[/* elsize */ 3]);
    long elsize;
    if (!numFromVal(&elsize, sh, valElsize)) {
        CL_ERROR_MSG(lw, "'elsize' arg of calloc() is not a known integer");
        return false;
    }

    *pDst = nelem * elsize;
    return true;
}

void printUserMessage(SymProc &proc, const struct cl_operand &opMsg)
{
    const SymHeap &sh = proc.sh();

    const TValId valDesc = proc.valFromOperand(opMsg);
    const EValueTarget code = sh.valTarget(valDesc);
    if (VT_CUSTOM != code)
        // not a custom value
        return;

    const CustomValue cVal = sh.valUnwrapCustom(valDesc);
    if (CV_STRING != cVal.code)
        // not a string custom value
        return;

    const struct cl_loc *loc = proc.lw();
    CL_NOTE_MSG(loc, "user message: " << cVal.data.str);
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
    printBackTrace(core);

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

    unsigned size;
    if (!resolveCallocSize(&size, core, opList))
        return false;

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

    dst.insert(core.sh());
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
    long size;
    if (!numFromVal(&size, core.sh(), valSize)) {
        CL_ERROR_MSG(lw, "size arg of malloc() is not a known integer");
        return false;
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
    SymHeap &sh = core.sh();
    const struct cl_loc *lw = &insn.loc;
    const CodeStorage::TOperandList &opList = insn.operands;
    if (5 != opList.size() || opList[0].code != CL_OPERAND_VOID) {
        emitPrototypeError(lw, name);
        return false;
    }

    // how much we are going to write?
    const TValId valSize = core.valFromOperand(opList[/* size */ 4]);
    long size;
    if (!numFromVal(&size, sh, valSize)) {
        CL_ERROR_MSG(lw, "size arg of memset() is not a known integer");
        insertCoreHeap(dst, core, insn, /* printBt */ true);
        return true;
    }
    if (!size) {
        CL_WARN_MSG(lw, "ignoring call of memset() with size == 0");
        insertCoreHeap(dst, core, insn, /* printBt */ true);
        return true;
    }

    // check the pointer - is it valid? do we have enough allocated memory?
    const TValId addr = core.valFromOperand(opList[/* addr */ 2]);
    if (core.checkForInvalidDeref(addr, size)) {
        // error message already printed out
        insertCoreHeap(dst, core, insn, /* printBt */ true);
        return true;
    }

    // what are we going to write?
    TValId tplValue = core.valFromOperand(opList[/* char */ 3]);
    if (VAL_NULL != tplValue) {
        CL_DEBUG_MSG(lw, "memset() writing nonzero value writes unknown value");
        tplValue = sh.valCreate(VT_UNKNOWN, VO_ASSIGNED);
    }

    // enter leak monitor
    LeakMonitor lm(sh);
    lm.enter();

    // write the block!
    TValSet killedPtrs;
    CL_DEBUG_MSG(lw, "executing memset() as a built-in function");
    sh.writeUniformBlock(addr, VAL_NULL, size, &killedPtrs);

    // check for memory leaks
    if (lm.collectJunkFrom(killedPtrs)) {
        CL_WARN_MSG(lw, "memory leak detected while executing memset()");
        printBackTrace(core);
    }

    // leave monitor and write the result
    lm.leave();
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
    const TObjId objDst = core.objByOperand(opDst);
    const TValId val = sh.valCreate(VT_UNKNOWN, VO_ASSIGNED);
    core.objSetValue(objDst, val);
    sh.objReleaseId(objDst);

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
    const SymExecCoreParams &ep = core.params();
    if (ep.skipPlot)
        CL_DEBUG_MSG(&insn.loc, name << "() skipped per user's request");

    else if (!(callPlot(insn, core)))
        return false;

    insertCoreHeap(dst, core, insn);
    return true;
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

    long module;
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
    printBackTrace(core);
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

BuiltInTable::BuiltInTable() {
    // register built-ins
    tbl_["abort"]                                   = handleAbort;
    tbl_["calloc"]                                  = handleCalloc;
    tbl_["free"]                                    = handleFree;
    tbl_["malloc"]                                  = handleMalloc;
    tbl_["memset"]                                  = handleMemset;
    tbl_["___sl_break"]                             = handleBreak;
    tbl_["___sl_error"]                             = handleError;
    tbl_["___sl_get_nondet_int"]                    = handleNondetInt;
    tbl_["___sl_plot"]                              = handlePlot;
    tbl_["___sl_enable_debugging_of"]               = handleDebuggingOf;

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
