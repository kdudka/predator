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
#include "symbin.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symplot.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <cstring>
#include <libgen.h>

template <int N_ARGS, class TOpList>
bool chkVoidCall(const TOpList &opList)
{
    if (/* dst + fnc */ 2 != opList.size() - N_ARGS)
        return false;
    else
        return (CL_OPERAND_VOID == opList[0].code);
}

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

template <int NTH, class TOpList, class TProc>
bool readHeapVal(TValId *dst, const TOpList opList, TProc &proc) {
    const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
    const TValId value = proc.valFromOperand(op);
    if (value < 0)
        return false;

    *dst = value;
    return true;
}

template <class TInsn, class TProc>
bool readNameAndValue(std::string       *pName,
                      TValId            *pValue,
                      const TInsn       &insn,
                      TProc             &proc)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (!chkVoidCall<2>(opList))
        return false;

    if (!readHeapVal<0>(pValue, opList, proc))
        return false;

    if (!readPlotName<1>(pName, opList, proc.lw()))
        return false;

    return true;
}

template <class TStor, class TFnc, class THeap>
bool fncFromHeapVal(const TStor &stor, const TFnc **dst, TValId value,
                    const THeap &heap)
{
    const int uid = heap.valGetCustom(value);
    if (-1 == uid)
        return false;

    const CodeStorage::FncDb &fncs = stor.fncs;
    const TFnc *fnc = fncs[uid];
    if (!fnc)
        return false;

    *dst = fnc;
    return true;
}

namespace {
void emitPrototypeError(const struct cl_loc *lw, const std::string &fnc) {
    CL_WARN_MSG(lw, "incorrectly called "
            << fnc << "() not recognized as built-in");
}

void emitPlotError(const struct cl_loc *lw, const std::string &plotName) {
    CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");
}
} // namespace

#define DO_PLOT(method) do {                                                \
    SymPlot plotter(sh);                                                    \
    if (!plotter.method)                                                    \
        emitPlotError(lw, plotName);                                        \
} while (0)

template <class TInsn, class TProc>
bool callPlot(const TInsn &insn, TProc &proc) {
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_loc *lw = &insn.loc;

    std::string plotName;
    if (!chkVoidCall<1>(opList)
            || !readPlotName<0>(&plotName, opList, proc.lw())) {
        emitPrototypeError(lw, "___sl_plot");
        return false;
    }

    const SymHeap &sh = proc.sh();
    DO_PLOT(plot(plotName));
    return true;
}

template <class TInsn, class TProc>
bool callPlotByPtr(const TInsn &insn, TProc &proc) {
    const struct cl_loc *lw = &insn.loc;

    std::string plotName;
    TValId value;
    if (!readNameAndValue(&plotName, &value, insn, proc)) {
        emitPrototypeError(lw, "___sl_plot_by_ptr");
        return false;
    }

    const SymHeap &sh = proc.sh();
    DO_PLOT(plotHeapValue(plotName, value));
    return true;
}

template <class TInsn, class TProc>
bool callPlotStackFrame(const TInsn &insn, TProc &proc) {
    const CodeStorage::Storage &stor = *insn.stor;
    const struct cl_loc *lw = &insn.loc;
    const SymHeap &sh = proc.sh();

    std::string plotName;
    TValId value;
    const CodeStorage::Fnc *fnc;

    if (!readNameAndValue(&plotName, &value, insn, proc)
            || !fncFromHeapVal(stor, &fnc, value, sh))
    {
        emitPrototypeError(lw, "___sl_plot_stack_frame");
        return false;
    }

    DO_PLOT(plotStackFrame(plotName, *fnc, proc.bt()));
    return true;
}

#define HANDLE_PLOT_CALL(name, call) do {                                   \
    if (STREQ(fncName, #name)) {                                            \
        if (ep.skipPlot)                                                    \
            CL_DEBUG_MSG(lw, #name " skipped per user's request");          \
                                                                            \
        else if (!(call(insn, core)))                                       \
            return false;                                                   \
                                                                            \
        core.killInsn(insn);                                                \
        dst.insert(sh);                                                     \
        return true;                                                        \
    }                                                                       \
} while (0)

bool handleBuiltIn(SymState                     &dst,
                   SymExecCore                  &core,
                   const CodeStorage::Insn      &insn)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_operand &fnc = opList[1];
    CL_BREAK_IF(CL_OPERAND_CST != fnc.code);

    const struct cl_cst &cst = fnc.data.cst;
    CL_BREAK_IF(CL_TYPE_FNC != cst.code);
    CL_BREAK_IF(CL_SCOPE_GLOBAL != fnc.scope || !cst.data.cst_fnc.is_extern);

    const char *fncName = cst.data.cst_fnc.name;
    CL_BREAK_IF(!fncName);

    SymHeap                     &sh = core.sh();
    const struct cl_loc         *lw = core.lw();
    const SymExecCoreParams     &ep = core.params();

    if (STREQ(fncName, "abort")) {
        CL_BREAK_IF(opList.size() != 2 || opList[0].code != CL_OPERAND_VOID);

        // do nothing for abort()
        dst.insert(sh);
        return true;
    }

    if (STREQ(fncName, "___sl_break")) {
        CL_WARN_MSG(lw, "___sl_break() reached, stopping per user's request");
        dst.insert(sh);

        CL_TRAP;
        return true;
    }

    if (STREQ(fncName, "___sl_get_nondet_int")) {
        if (2 != opList.size()) {
            emitPrototypeError(lw, fncName);
            return false;
        }

        // set the returned value to a new unknown value
        CL_DEBUG_MSG(lw, "executing ___sl_get_nondet_int()");
        const struct cl_operand &opDst = opList[0];
        const TObjId objDst = core.objByOperand(opDst);
        const TValId val = sh.valCreateUnknown(VT_UNKNOWN, VO_UNKNOWN);
        core.objSetValue(objDst, val);

        // insert the resulting heap
        dst.insert(sh);
        return true;
    }

    HANDLE_PLOT_CALL(___sl_plot,             callPlot          );
    HANDLE_PLOT_CALL(___sl_plot_by_ptr,      callPlotByPtr     );
    HANDLE_PLOT_CALL(___sl_plot_stack_frame, callPlotStackFrame);

    // no built-in has been matched
    return false;
}
