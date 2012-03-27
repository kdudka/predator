/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#include "config_cl.h"
#include "killer.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "util.hh"
#include "stopwatch.hh"

#include <map>
#include <set>

#include <boost/foreach.hpp>

static int debugVarKiller = CL_DEBUG_VAR_KILLER;

#define VK_DEBUG(level, ...) do {                                           \
    if ((level) <= ::debugVarKiller)                                        \
        CL_DEBUG("VarKiller: " << __VA_ARGS__);                             \
} while (0)

#define VK_DEBUG_MSG(level, lw, ...) do {                                   \
    if ((level) <= ::debugVarKiller)                                        \
        CL_DEBUG_MSG(lw, "VarKiller: " << __VA_ARGS__);                     \
} while (0)

namespace CodeStorage {

namespace VarKiller {

typedef CodeStorage::Storage               &TStorRef;
typedef const struct cl_loc                *TLoc;
typedef int                                 TVar;
typedef std::set<TVar>                      TSet;
typedef const Block                        *TBlock;
typedef std::set<TBlock>                    TBlockSet;

/// per-block data
struct BlockData {
    TSet                                    gen;
    TSet                                    kill;
};

typedef std::map<TBlock, BlockData>         TMap;

/// shared data
struct Data {
    TStorRef                                stor;
    TBlockSet                               todo;
    TMap                                    blocks;

    Data(TStorRef stor_):
        stor(stor_)
    {
    }
};

inline bool isLcVar(const cl_operand &op) {
    if (CL_OPERAND_VAR != op.code)
        // not a variable
        return false;

    const enum cl_scope_e code = op.scope;
    switch (code) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
            // global variable
            return false;

        default:
            return true;
    }
}

void scanOperand(BlockData &bData, const cl_operand &op, bool dst) {
    VK_DEBUG(4, "scanOperand: " << op << ((dst) ? " [dst]" : " [src]"));

    bool fieldOfComp = false;

    for (const cl_accessor *ac = op.accessor; ac; ac = ac->next) {
        CL_BREAK_IF(dst && seekRefAccessor(ac));

        const enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_DEREF_ARRAY:
                // FIXME: unguarded recursion
                scanOperand(bData, *(ac->data.array.index), /* dst */ false);
                // fall through!

            case CL_ACCESSOR_DEREF:
                // see whatever operand with CL_ACCESSOR_DEREF as [src]
                dst = false;
                break;

            case CL_ACCESSOR_ITEM:
                fieldOfComp = true;
                // fall through!

            case CL_ACCESSOR_REF:
                break;
        }
    }

    if (dst && fieldOfComp)
        // if we are writing to a field of a composite type, it yet does not
        // mean we are killing the whole composite variable
        return;

    if (!isLcVar(op))
        // not a local variable
        return;

    const char *name = NULL;
    const int uid = varIdFromOperand(&op, &name);
    if (hasKey(bData.kill, uid))
        // already killed
        return;

    if (dst) {
        VK_DEBUG(3, "kill(" << name << ")");
        bData.kill.insert(uid);
        return;
    }

    // we see the operand as [src]
    if (insertOnce(bData.gen, uid))
        VK_DEBUG(3, "gen(" << name << ")");
}

void scanInsn(BlockData &bData, const Insn &insn) {
    VK_DEBUG_MSG(3, &insn.loc, "scanInsn: " << insn);
    const TOperandList opList = insn.operands;

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_CALL:
        case CL_INSN_UNOP:
        case CL_INSN_BINOP:
            // go backwards!
            CL_BREAK_IF(opList.empty());
            for (int i = opList.size() - 1; 0 <= i; --i)
                scanOperand(bData, opList[i], /* dst */ !i);
            return;

        case CL_INSN_RET:
        case CL_INSN_COND:
        case CL_INSN_SWITCH:
            // exactly one operand
            scanOperand(bData, opList[/* src */ 0], /* dst */ false);
            return;

        case CL_INSN_JMP:
        case CL_INSN_NOP:
        case CL_INSN_ABORT:
        case CL_INSN_LABEL:
            // no operand
            return;
    }
}

void updateBlock(Data &data, TBlock bb) {
    VK_DEBUG_MSG(2, &bb->front()->loc, "updateBlock: " << bb->name());
    BlockData &bData = data.blocks[bb];
    bool anyChange = false;

    // go through all variables generated by successors
    BOOST_FOREACH(TBlock bbSrc, bb->targets()) {
        BlockData &bDataSrc = data.blocks[bbSrc];

        // update self
        BOOST_FOREACH(TVar uid, bDataSrc.gen) {
            if (hasKey(bData.kill, uid))
                // we are killing the variable
                continue;

            if (insertOnce(bData.gen, uid))
                anyChange = true;
        }
    }

    if (!anyChange)
        // nothing updated actually
        return;

    // schedule all predecessors
    BOOST_FOREACH(TBlock bbDst, bb->inbound())
        data.todo.insert(bbDst);
}

void computeFixPoint(Data &data) {
    // fixed-point computation
    unsigned cntSteps = 1;
    TBlockSet &todo = data.todo;
    while (!todo.empty()) {
        TBlockSet::iterator i = todo.begin();
        TBlock bb = *i;
        todo.erase(i);

        // (re)compute a single basic block
        updateBlock(data, bb);
        ++cntSteps;
    }

    VK_DEBUG(2, "fixed-point reached in " << cntSteps << " steps");
}

void commitInsn(
        Data                    &data,
        Insn                    &insn,
        TSet                    &live,
        std::vector<TSet>       &livePerTarget)
{
    const TStorRef stor = data.stor;
    const TBlock bb = insn.bb;

    Insn &term = *const_cast<Insn *>(bb->back());
    const TTargetList &targets = bb->targets();
    const unsigned cntTargets = targets.size();
    const bool multipleTargets = (1 < cntTargets);

    // FIXME: performance waste
    // info about which variables are generated and killed by this particular
    // instruction 'insn' could be precomputed already (in analyzeFnc() - before
    // computeFixPoint() call).
    BlockData arena;
    scanInsn(arena, insn);

    // this is done for variables that are assigned by this 'insn' instruction
    // and this particular value will be used anymore -- so kill it immediately
    // by 'insn'.
    BOOST_FOREACH(const TVar vKill, arena.kill)
        arena.gen.insert(vKill);

    // go through variables generated by the current instruction
    BOOST_FOREACH(TVar vKill, arena.gen) {
        const bool isPointed = stor.vars[vKill].mayBePointed;

        if (insertOnce(live, vKill)) {
            // kill a single variable at a single location
            VK_DEBUG_MSG(1, &insn.loc, "killing variable "
                    << varToString(stor, vKill)
                    << " by " << insn);

            const KillVar kv(vKill, isPointed);
            insn.varsToKill.insert(kv);

            if (multipleTargets) {
                // mark "live" in all targets to avoid a double-kill
                for (unsigned i = 0; i < cntTargets; ++i)
                    livePerTarget[i].insert(vKill);
            }
        }

        if (!multipleTargets)
            // we have at most one target
            continue;

        // failed to kill the var for all targets, now try it one by one
        for (unsigned i = 0; i < cntTargets; ++i) {
            if (!insertOnce(livePerTarget[i], vKill))
                continue;

            VK_DEBUG_MSG(1, &term.loc, "killing variable "
                    << varToString(stor, vKill)
                    << " per target " << targets[i]->name()
                    << " by " << term);

            const KillVar kv(vKill, isPointed);
            term.killPerTarget[i].insert(kv);
        }
    }
}

void commitBlock(Data &data, TBlock bb) {
    const TTargetList &targets = bb->targets();
    const unsigned cntTargets = targets.size();
    const bool multipleTargets = (1 < cntTargets);

    std::vector<TSet> livePerTarget;
    if (multipleTargets)
        livePerTarget.resize(cntTargets);

    // build list of live variables coming from all successors
    TSet live;
    for (unsigned i = 0; i < cntTargets; ++i) {
        const TBlock bbSrc = targets[i];
        BOOST_FOREACH(TVar vLive, data.blocks[bbSrc].gen) {
            live.insert(vLive);
            if (multipleTargets)
                livePerTarget[i].insert(vLive);
        }
    }

    // go backwards through the instructions
    CL_BREAK_IF(!bb->size());
    for (int i = bb->size()-1; 0 <= i; --i) {
        const Insn *pInsn = bb->operator[](i);
        Insn &insn = *const_cast<Insn *>(pInsn);
        commitInsn(data, insn, live, livePerTarget);
    }
}

void analyzeFnc(Fnc &fnc) {
    // shared state info
    Data data(*fnc.stor);

    TLoc loc = &fnc.def.data.cst.data.cst_fnc.loc;
    VK_DEBUG_MSG(2, loc, ">>> entering " << nameOf(fnc) << "()");
    CL_BREAK_IF(!data.todo.empty());

    // go through basic blocks
    BOOST_FOREACH(const TBlock bb, fnc.cfg) {
        // go through instructions in forward direction
        VK_DEBUG(3, "in block " << bb->name());

        BOOST_FOREACH(const Insn *insn, *bb) {
            BlockData &bData = data.blocks[bb];
            scanInsn(bData, *insn);
        }

        data.todo.insert(bb);
    }

    // compute a fixed-point for a single function
    VK_DEBUG_MSG(2, loc, "computing fixed-point for " << nameOf(fnc) << "()");
    computeFixPoint(data);

    // commit the results
    BOOST_FOREACH(const TBlock bb, fnc.cfg) {
        VK_DEBUG_MSG(2, &bb->front()->loc, "commitBlock: " << bb->name());
        commitBlock(data, bb);
    }
}

} // namespace VarKiller

void killLocalVariables(Storage &stor) {
    StopWatch watch;

    // analyze all _defined_ functions
    BOOST_FOREACH(Fnc *pFnc, stor.fncs) {
        Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        // analyze a single function
        VarKiller::analyzeFnc(fnc);
    }

    CL_DEBUG("killLocalVariables() took " << watch);
}

} // namespace CodeStorage
