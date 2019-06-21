/*
 * Copyright (C) 2012 Kamil Dudka <kdudka@redhat.com>
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

#include "../config_cl.h"
#include "../util.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/easy.hh>
#include <cl/storage.hh>

#include <map>
#include <set>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" {
    __attribute__ ((__visibility__ ("default"))) int plugin_is_GPL_compatible;
}

typedef const CodeStorage::Fnc             *TFnc;
typedef const CodeStorage::Block           *TBlock;
typedef const CodeStorage::Insn            *TInsn;
typedef const struct cl_loc                *TLoc;
typedef const struct cl_operand            &TOp;
typedef CodeStorage::TKillVarList           TKillList;
typedef std::set<TBlock>                    TBlockSet;
typedef std::set<cl_uid_t>                  TState;
typedef std::map<TBlock, TState>            TStateMap;

struct PerFncData {
    const TFnc                      fnc;
    TStateMap                       stateMap;
    TBlockSet                       todo;
    bool                            done;

    PerFncData(const TFnc fnc_):
        fnc(fnc_),
        done(false)
    {
    }
};

bool chkAssert(
        const TInsn                 insn,
        const TState               &state,
        const char                 *name,
        const bool                  done)
{
    const TLoc loc = &insn->loc;

    const CodeStorage::TOperandList &opList = insn->operands;
    if (opList.size() < /* ret + fnc + state + op0 */ 4) {
        CL_ERROR_MSG(loc, name << ": missing operand");
        return false;
    }

    const bool live = intCstFromOperand(&opList[/* state */ 2]);

    for (unsigned i = /* op0 */ 3; i < opList.size(); ++i) {
        TOp op = opList[i];
        if (!isLcVar(op)) {
            CL_ERROR_MSG(loc, name << ": invalid operand #" << (i - 2));
            continue;
        }

        if (!done)
            // fixed-point not yet computed
            continue;

        const cl_uid_t uid = varIdFromOperand(&op);
        if (hasKey(state, uid) == live)
            // matched
            continue;

        const char *status = (live)
            ? "VK_LIVE"
            : "VK_DEAD";

        const std::string varName = varToString(*insn->stor, uid);
        CL_ERROR_MSG(loc, name << ": property violated: "
                << status << ": " << varName);
    }

    // built-in handled
    return true;
}

bool handleBuiltIn(
        const TInsn                 insn,
        const TState               &state,
        const bool                  done)
{
    if (CL_INSN_CALL != insn->code)
        // not a function call
        return false;

    const char *name;
    if (!fncNameFromCst(&name, &insn->operands[/* fnc */ 1]))
        // indirect function call?
        return false;

    if (STREQ("VK_ASSERT", name))
        return chkAssert(insn, state, name, done);

    // no built-in matched
    return false;
}

bool isLocalUid(PerFncData &data, cl_uid_t uid) {
    if (!hasKey(data.fnc->vars, uid))
        return false;
    return (data.fnc->stor->vars[uid].code != CodeStorage::VAR_GL);
}

void killVars(
        TState                         &state,
        PerFncData                     &data,
        const TInsn                     insn,
        const TKillList                &kList)
{
    BOOST_FOREACH(const CodeStorage::KillVar &kv, kList) {
        if (kv.onlyIfNotPointed)
            // TODO: try all possibilities?
            continue;

        const cl_uid_t uid = kv.uid;
        const std::string varName = varToString(*insn->stor, uid);
        CL_DEBUG_MSG(&insn->loc, "DEAD: " << varName << " by " << *insn);

        if (1 == state.erase(uid))
            // successfully killed a variable
            continue;

        if (!isLocalUid(data, uid))
            // Just skip non-local uids (which are killed just due to points-to
            // analysis).  We are !not! checking whether the uid is live
            // variable.
            // FIXME: some smarter solution should be involved
            continue;

        CL_DEBUG_MSG(&insn->loc, "attempt to kill a dead variable: "
                << varToString(*insn->stor, uid));
    }
}

void updateTargets(
        PerFncData                 &data,
        const TInsn                 insn,
        const TState               &origin)
{
    const CodeStorage::TTargetList &tList = insn->targets;
    for (unsigned target = 0; target < tList.size(); ++target) {
        // kill variables per-target
        TState state(origin);
        killVars(state, data, insn, insn->killPerTarget[target]);

        // resolve the target block
        const TBlock bb = tList[target];
        TState &dst = data.stateMap[bb];

        // update the state in the target block
        const unsigned lastSize = dst.size();
        dst.insert(state.begin(), state.end());

        if (lastSize != dst.size())
            // schedule the _target_ block for processing
            data.todo.insert(bb);
    }
}

void updateBlock(PerFncData &data, const TBlock bb) {
    TState state(data.stateMap[bb]);

    BOOST_FOREACH(const TInsn insn, *bb) {
        if (handleBuiltIn(insn, state, data.done))
            // handled as a built-in function
            continue;

        // first mark all local variables used by this insn as live
        BOOST_FOREACH(TOp op, insn->operands) {
            if (!isLcVar(op))
                continue;

            const cl_uid_t uid = varIdFromOperand(&op);
            const std::string varName = varToString(*insn->stor, uid);
            CL_DEBUG_MSG(&insn->loc, "LIVE: " << varName << " by " << *insn);
            state.insert(uid);
        }

        // then kill all variables suggested by varKiller
        killVars(state, data, insn, insn->varsToKill);

        // if this is a terminal instruction, update all targets
        updateTargets(data, insn, state);
    }
}

void chkFunction(const TFnc fnc) {
    PerFncData data(fnc);

    // mark the function arguments as live for the entry block
    const CodeStorage::ControlFlow &cfg = fnc->cfg;
    TState &state = data.stateMap[cfg.entry()];
    BOOST_FOREACH(const cl_uid_t uid, data.fnc->args)
        state.insert(uid);

    // schedule all basic blocks for processing
    TBlockSet &todo = data.todo;
    BOOST_FOREACH(const TBlock bb, cfg)
        todo.insert(bb);

    // read the location info to be used in the verbose output
    const TLoc loc = &fnc->def.data.cst.data.cst_fnc.loc;
    CL_DEBUG_MSG(loc, "--> computing a fixed-point for "
            << nameOf(*fnc) << "()");

    // fixed-point computation
    int cntSteps = 1 - cfg.size();
    while (!todo.empty()) {
        TBlockSet::iterator i = todo.begin();
        TBlock bb = *i;
        todo.erase(i);

        // (re)compute a single basic block
        updateBlock(data, bb);
        ++cntSteps;
    }

    CL_DEBUG_MSG(loc, "<-- fixed-point for "
            << nameOf(*fnc) << "() reached in " << cntSteps << " steps");

    // now finally report the errors
    data.done = true;
    BOOST_FOREACH(const TBlock bb, cfg)
        updateBlock(data, bb);
}

void clEasyRun(const CodeStorage::Storage &stor, const char *) {
    CL_DEBUG("chk_var_killer started...");

    // go through all _defined_ functions
    BOOST_FOREACH(const TFnc fnc, stor.fncs)
        if (isDefined(*fnc))
            chkFunction(fnc);
}
