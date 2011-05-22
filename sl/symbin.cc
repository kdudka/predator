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

namespace {
void emitPrototypeError(const struct cl_loc *lw, const std::string &fnc) {
    CL_WARN_MSG(lw, "incorrectly called "
            << fnc << "() not recognized as built-in");
}

void emitPlotError(const struct cl_loc *lw, const std::string &plotName) {
    CL_WARN_MSG(lw, "error while plotting '" << plotName << "'");
}
} // namespace

bool callPlot(const CodeStorage::Insn &insn, SymProc &proc) {
    const CodeStorage::TOperandList &opList = insn.operands;
    const struct cl_loc *lw = &insn.loc;

    const int cntArgs = opList.size() - /* dst + fnc */ 2;
    if (cntArgs < 1)
        // insufficient count of arguments
        return false;

    if (CL_OPERAND_VOID != opList[/* dst */ 0].code)
        // not a function returning void
        return false;

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
        CL_BREAK_IF("not tested");

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
        const TValId val = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        core.objSetValue(objDst, val);

        // insert the resulting heap
        dst.insert(sh);
        return true;
    }

    if (STREQ(fncName, "___sl_plot")) {
        if (ep.skipPlot)
            CL_DEBUG_MSG(lw, "___sl_plot() skipped per user's request");

        else if (!(callPlot(insn, core)))
            return false;

        core.killInsn(insn);
        dst.insert(sh);
        return true;
    }

    // no built-in has been matched
    return false;
}
