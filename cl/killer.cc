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

typedef const struct cl_loc                 *TLoc;
typedef int                                 TVar;
typedef std::set<TVar>                      TSet;
typedef const Block                         *TBlock;
typedef std::set<TBlock>                    TBlockSet;

/// per-block data
struct BlockData {
    TBlockSet                               refs;
    TSet                                    gen;
    TSet                                    kill;
};

typedef std::map<TBlock, BlockData>         TMap;

/// shared data
struct Data {
    TBlockSet                               todo;
    TMap                                    blocks;
    TSet                                    pointed;
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

void scanRefs(TSet &pointed, const cl_operand &op) {
    if (!isLcVar(op))
        // not a local variable
        return;

    const cl_accessor *ac = op.accessor;
    if (!ac || ac->code == CL_ACCESSOR_DEREF || !seekRefAccessor(ac))
        // not a reference
        return;

    // black-list a variable that we take a reference to
    const int uid = varIdFromOperand(&op);
    if (insertOnce(pointed, uid))
        VK_DEBUG(1, "pointed |= #" << uid);
}

void scanOperand(Data &data, TBlock bb, const cl_operand &op, bool dst) {
    if (!isLcVar(op))
        // not a local variable
        return;

    BlockData &bData = data.blocks[bb];
    VK_DEBUG(4, "scanOperand: " << op << ((dst) ? " [dst]" : " [src]"));
    const int uid = varIdFromOperand(&op);
    if (hasKey(bData.kill, uid))
        // already killed
        return;

    const cl_accessor *ac = op.accessor;
    CL_BREAK_IF(dst && seekRefAccessor(ac));
    if (dst && ac && ac->code == CL_ACCESSOR_DEREF)
        // we should see whatever operand with CL_ACCESSOR_DEREF as [src]
        dst = false;

    if (dst) {
        VK_DEBUG(3, "kill(" << bb->name() << ") |= #" << uid);
        bData.kill.insert(uid);
        return;
    }

    // we see the operand as [src]
    if (insertOnce(bData.gen, uid))
        VK_DEBUG(3, "gen(" << bb->name() << ") |= #" << uid);
}

void scanInsn(Data &data, TBlock bb, const Insn &insn) {
    VK_DEBUG_MSG(3, &insn.loc, "scanInsn: " << insn);

    // black-list all variables that we take a reference to by this instruction
    TOperandList opList = insn.operands;
    BOOST_FOREACH(const cl_operand &op, opList)
        scanRefs(data.pointed, op);

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_CALL:
        case CL_INSN_UNOP:
        case CL_INSN_BINOP:
            // go backwards!
            CL_BREAK_IF(opList.empty());
            for (int i = opList.size() - 1; 0 <= i; --i)
                scanOperand(data, bb, opList[i], /* dst */ !i);
            return;

        case CL_INSN_RET:
        case CL_INSN_COND:
        case CL_INSN_SWITCH:
            // exactly one operand
            scanOperand(data, bb, opList[/* src */ 0], /* dst */ false);
            return;

        case CL_INSN_JMP:
        case CL_INSN_NOP:
        case CL_INSN_ABORT:
            // no operand
            return;
    }
}

void scanBlock(Data &data, TBlock bb) {
    // go through instructions in forward direction
    BOOST_FOREACH(const Insn *insn, *bb)
        scanInsn(data, bb, *insn);

    // dig backward references
    // FIXME: should they already be provided by CodeStorage?
    BOOST_FOREACH(TBlock target, bb->targets()) {
        BlockData &bTarget = data.blocks[target];
        bTarget.refs.insert(bb);
    }
}

bool /* changed */ updateBlockBy(BlockData &bData, TVar uid) {
    if (hasKey(bData.kill, uid))
        // we are killing the variable
        return false;

    return insertOnce(bData.gen, uid);
}

void updateBlock(Data &data, TBlock bb) {
    VK_DEBUG_MSG(2, &bb->front()->loc, "updateBlock: " << bb->name());
    BlockData &bData = data.blocks[bb];
    bool anyChange = false;

    // go through all variables generated by successors
    BOOST_FOREACH(TBlock bbSrc, bb->targets()) {
        BlockData &bDataSrc = data.blocks[bbSrc];

        // update self
        BOOST_FOREACH(TVar uid, bDataSrc.gen)
            if (updateBlockBy(bData, uid))
                anyChange = true;
    }

    if (!anyChange)
        // nothing updated actually
        return;

    // schedule all predecessors
    BOOST_FOREACH(TBlock bbDst, bData.refs)
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

void analyseFnc(Data &data, Fnc &fnc) {
    TLoc loc = &fnc.def.data.cst.data.cst_fnc.loc;
    VK_DEBUG_MSG(2, loc, ">>> entering " << nameOf(fnc) << "()");
    CL_BREAK_IF(!data.todo.empty());

    // go through basic blocks
    BOOST_FOREACH(TBlock bb, fnc.cfg) {
        scanBlock(data, bb);
        data.todo.insert(bb);
    }

    // compute a fixed-point for a single function
    VK_DEBUG_MSG(2, loc, "computing fixed-point for " << nameOf(fnc) << "()");
    computeFixPoint(data);
}

void killOperand(Insn &insn, TVar byUid, bool isPointed) {
    int idx = -1;

    const TOperandList opList = insn.operands;
    for (unsigned i = 0; i < opList.size(); ++i) {
        const cl_operand &op = opList[i];
        if (CL_OPERAND_VAR != op.code)
            // not a variable
            continue;

        const TVar uid = varIdFromOperand(&op);
        if (uid != byUid)
            // not the operand we are about to kill
            continue;

        if (-1 == idx) {
            // first match
            idx = i;
            continue;
        }

        // withdraw it
        VK_DEBUG(1, "killOperand: ambiguous match of var #"
                << byUid << " in "
                << insn << " (cowardly refusing to kill it)");
        return;
    }

    // kill a single variable at a single location
    CL_BREAK_IF(idx < 0);

    EKillStatus code = KS_ALWAYS_KILL;
    const char *suffix = "";
    if (isPointed) {
        code = KS_KILL_IF_NOT_POINTED;
        suffix = " [KS_KILL_IF_NOT_POINTED]";
    }

    insn.opsToKill[idx] = code;
    VK_DEBUG_MSG(1, &insn.loc, "killing variable #" << byUid << " by " << insn
            << suffix);
}

void commitBlock(Data &data, TBlock bb, const TSet &pointed) {
    CL_BREAK_IF(!bb->size());

    // build list of live variables coming from all successors
    TSet live;
    BOOST_FOREACH(TBlock bbSrc, bb->targets()) {
        BOOST_FOREACH(TVar vLive, data.blocks[bbSrc].gen)
            live.insert(vLive);
    }

    // go backwards through the instructions
    for (int i = bb->size()-1; 0 <= i; --i) {
        const Insn *insn = bb->operator[](i);

        // FIXME: performance waste
        Data arena;
        arena.pointed = /* only suppress duplicated verbose messages */ pointed;
        scanInsn(arena, bb, *insn);
        const BlockData &now = arena.blocks[bb];

        // go through variables generated by the current instruction
        BOOST_FOREACH(TVar vg, now.gen) {
            if (!insertOnce(live, vg))
                // live at this point
                continue;

            // attempt to kill a single variable at a single location
            const bool isPointed = hasKey(pointed, vg);
            killOperand(const_cast<Insn &>(*insn), vg, isPointed);
        }
    }
}

void commitResults(Data &data) {
    // go through all basic blocks (no matter which fnc each of them belongs to)
    BOOST_FOREACH(TMap::const_reference item, data.blocks) {
        TBlock bb = item.first;
        VK_DEBUG_MSG(2, &bb->front()->loc, "commitResults: " << bb->name());
        commitBlock(data, bb, data.pointed);
    }
}

} // namespace VarKiller

void killLocalVariables(Storage &stor) {
    StopWatch watch;

    // shared state info
    VarKiller::Data data;

    // first go through all _defined_ functions and compute the fixed-point
    BOOST_FOREACH(Fnc *pFnc, stor.fncs) {
        Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        // analyse a single function
        VarKiller::analyseFnc(data, fnc);
    }

    // commit the results in batch mode (we needed to build Data::pointed first)
    VarKiller::commitResults(data);
    CL_DEBUG("killLocalVariables() took " << watch);
}

} // namespace CodeStorage
