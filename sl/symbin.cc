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

template <int N_ARGS, class TOpList>
bool chkVoidCall(const TOpList &opList)
{
    if (/* dst + fnc */ 2 != opList.size() - N_ARGS)
        return false;
    else
        return (CL_OPERAND_VOID == opList[0].code);
}

template <int NTH, class TOpList>
bool readPlotName(std::string *dst, const TOpList opList)
{
    const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
    if (CL_OPERAND_CST != op.code)
        return false;

    const cl_cst &cst = op.data.cst;
    if (CL_TYPE_STRING != cst.code)
        return false;

    *dst = cst.data.cst_string.value;
    return true;
}

template <int NTH, class TOpList, class TProc>
bool readHeapVal(TValueId *dst, const TOpList opList, TProc &proc) {
    const cl_operand &op = opList[NTH + /* dst + fnc */ 2];
    const TValueId value = proc.heapValFromOperand(op);
    if (value < 0)
        return false;

    *dst = value;
    return true;
}

template <class TInsn, class TProc>
bool readNameAndValue(std::string       *pName,
                      TValueId          *pValue,
                      const TInsn       &insn,
                      TProc             &proc)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    const LocationWriter lw(&insn.loc);

    if (!chkVoidCall<2>(opList))
        return false;

    if (!readHeapVal<0>(pValue, opList, proc))
        return false;

    if (!readPlotName<1>(pName, opList))
        return false;

    return true;
}

template <class TStor, class TFnc, class THeap>
bool fncFromHeapVal(const TStor &stor, const TFnc **dst, TValueId value,
                    const THeap &heap)
{
    const int uid = heap.valGetCustom(/* pClt */ 0, value);
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
void emitPrototypeError(const LocationWriter &lw, const std::string &fnc) {
    CL_WARN_MSG(lw, "incorrectly called "
            << fnc << "() not recognized as built-in");
}

void emitPlotError(const LocationWriter &lw, const std::string &plotName) {
    CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");
}
} // namespace

#define DO_PLOT(method) do {                                                \
    const CodeStorage::Storage &stor = *insn.stor;                          \
    SymPlot plotter(stor, sh);                                              \
    if (!plotter.method)                                                    \
        emitPlotError(lw, plotName);                                        \
} while (0)

template <class TInsn, class THeap>
bool callPlot(const TInsn &insn, const THeap &sh) {
    const CodeStorage::TOperandList &opList = insn.operands;
    const LocationWriter lw(&insn.loc);

    std::string plotName;
    if (!chkVoidCall<1>(opList) || !readPlotName<0>(&plotName, opList)) {
        emitPrototypeError(lw, "___sl_plot");
        return false;
    }

    DO_PLOT(plot(plotName));
    return true;
}

template <class TInsn, class TProc>
bool callPlotByPtr(const TInsn &insn, TProc &proc) {
    const LocationWriter lw(&insn.loc);

    std::string plotName;
    TValueId value;
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
    const LocationWriter lw(&insn.loc);
    const SymHeap &sh = proc.sh();

    std::string plotName;
    TValueId value;
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
        else if (!(call))                                                   \
            return false;                                                   \
                                                                            \
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
    SE_BREAK_IF(CL_OPERAND_CST != fnc.code);

    const struct cl_cst &cst = fnc.data.cst;
    SE_BREAK_IF(CL_TYPE_FNC != cst.code);
    SE_BREAK_IF(CL_SCOPE_GLOBAL != fnc.scope || !cst.data.cst_fnc.is_extern);

    const char *fncName = cst.data.cst_fnc.name;
    SE_BREAK_IF(!fncName);

    SymHeap                     &sh = core.sh();
    const LocationWriter        &lw = core.lw();
    const SymExecCoreParams     &ep = core.params();

    if (STREQ(fncName, "abort")) {
        SE_BREAK_IF(opList.size() != 2 || opList[0].code != CL_OPERAND_VOID);

        // do nothing for abort()
        dst.insert(sh);
        return true;
    }

    // TODO: automatic plot name generation when (name == NULL)
    //       filename-lineno-serial.dot
    HANDLE_PLOT_CALL(___sl_plot,             callPlot(insn, sh)             );
    HANDLE_PLOT_CALL(___sl_plot_by_ptr,      callPlotByPtr(insn, core)      );
    HANDLE_PLOT_CALL(___sl_plot_stack_frame, callPlotStackFrame(insn, core) );

    // no built-in has been matched
    return false;
}
