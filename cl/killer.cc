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

typedef int                                 TVar;
typedef std::set<TVar>                      TSet;
typedef const Block                         *TBlock;
typedef std::set<TBlock>                    TBlockSet;

struct BlockData {
    TBlockSet                               refs;
    TSet                                    gen;
    TSet                                    kill;
};

typedef std::map<TBlock, BlockData>         TMap;

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

    const int uid = varIdFromOperand(&op);
    VK_DEBUG(1, "pointed |= #" << uid);
    pointed.insert(uid);
}

void scanOperand(Data &data, TBlock bb, const cl_operand &op, bool dst) {
    if (!isLcVar(op))
        // not a local variable
        return;

    BlockData &bData = data.blocks[bb];
    VK_DEBUG(4, "scanOperand: " << op << ((dst) ? " [dst]" : " [src"));
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
    if (hasKey(bData.gen, uid))
        // already generated
        return;

    VK_DEBUG(3, "gen(" << bb->name() << ") |= #" << uid);
    bData.gen.insert(uid);

    // schedule this bb for fixed-point computation
    data.todo.insert(bb);
}

void scanInsn(Data &data, TBlock bb, const Insn &insn) {
    VK_DEBUG(3, "scanInsn: " << insn);
    TOperandList opList = insn.operands;
    BOOST_FOREACH(const cl_operand &op, opList)
        scanRefs(data.pointed, op);

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_CALL:
        case CL_INSN_UNOP:
        case CL_INSN_BINOP:
            for (unsigned i = 0; i < opList.size(); ++i)
                scanOperand(data, bb, opList[i], /* dst */ !i);
            return;

        case CL_INSN_RET:
        case CL_INSN_COND:
        case CL_INSN_SWITCH:
            scanOperand(data, bb, opList[/* src */ 0], /* dst */ false);
            return;

        case CL_INSN_JMP:
        case CL_INSN_NOP:
        case CL_INSN_ABORT:
            return;
    }
}

void scanBlock(Data &data, TBlock bb) {
    BOOST_FOREACH(const Insn *insn, *bb) {
        scanInsn(data, bb, *insn);
    }

    BOOST_FOREACH(TBlock target, bb->targets()) {
        BlockData &bTarget = data.blocks[target];
        bTarget.refs.insert(bb);
    }
}

void updateBlock(Data &data, TBlock bb) {
    BlockData &bData = data.blocks[bb];
    bool anyChange = false;

    // pick all generated variables first
    TSet out;
    BOOST_FOREACH(TBlock bbSrc, bb->targets()) {
        BlockData &bDataSrc = data.blocks[bbSrc];

        BOOST_FOREACH(TVar uid, bDataSrc.gen)
            out.insert(uid);
    }

    BOOST_FOREACH(TVar uid, out) {
        if (hasKey(bData.gen, uid))
            // already generated
            continue;

        if (hasKey(bData.kill, uid))
            // we are killing the variable
            continue;

        // propagate the var
        bData.gen.insert(uid);
        anyChange = true;
    }

    if (!anyChange)
        // nothing updated actually
        return;

    // schedule all consumers
    BOOST_FOREACH(TBlock bbDst, bData.refs) {
        data.todo.insert(bbDst);
    }
}

void computeFixPoint(Data &data) {
    unsigned cntSteps = 0;

    TBlockSet &todo = data.todo;
    while (!todo.empty()) {
        TBlockSet::iterator i = todo.begin();
        TBlock bb = *i;
        todo.erase(i);

        updateBlock(data, bb);
        ++cntSteps;
    }

    VK_DEBUG(2, "fixed-point reached in " << cntSteps << " steps");
}

void handleFnc(Data &data, Fnc &fnc) {
    CL_BREAK_IF(!data.todo.empty());
    BOOST_FOREACH(TBlock bb, fnc.cfg)
        scanBlock(data, bb);

    VK_DEBUG_MSG(2, &fnc.def.loc, "computing fixed-point for "
            << nameOf(fnc) << "()...");
    computeFixPoint(data);
}

void killOperand(const Insn *insn, TVar byUid) {
    int cntMatches = 0;

    const TOperandList opList = insn->operands;
    for (unsigned i = 0; i < opList.size(); ++i) {
        const cl_operand &op = opList[i];
        if (CL_OPERAND_VAR != op.code)
            // not a variable
            continue;

        const TVar uid = varIdFromOperand(&op);
        if (uid != byUid)
            // not the operand we are about to kill
            continue;

        // the var should not already be killed
        CL_BREAK_IF(insn->opsToKill[i]);

        Insn &writable = const_cast<Insn &>(*insn);
        writable.opsToKill[i] = true;
        ++cntMatches;
    }

    // we are supposed to kill exactly one variable
    CL_BREAK_IF(1 != cntMatches);
}

void commitBlock(TBlock bb, const BlockData &bData, const TSet &pointed) {
    CL_BREAK_IF(!bb->size());
    TSet live = bData.gen;

    // go backwards through the instructions
    for (int i = bb->size()-1; 0 <= i; --i) {
        const Insn *insn = bb->operator[](i);

        // FIXME: performance waste
        Data arena;
        scanInsn(arena, bb, *insn);
        const BlockData &now = arena.blocks[bb];

        BOOST_FOREACH(TVar vg, now.gen) {
            if (hasKey(pointed, vg))
                // globally black-listed for killing
                continue;

            if (!insertOnce(live, vg))
                // live at this point
                continue;

            VK_DEBUG_MSG(1, &insn->loc, "killing variable #"
                    << vg << " by " << *insn);

            killOperand(insn, vg);
        }
    }
}

void commitResults(Data &data) {
    BOOST_FOREACH(TMap::const_reference item, data.blocks) {
        TBlock bb = item.first;
        VK_DEBUG_MSG(2, &bb->front()->loc, "commitResults: " << bb->name());
        commitBlock(bb, item.second, data.pointed);
    }
}

} // namespace VarKiller

void killLocalVariables(Storage &stor) {
    VarKiller::Data data;

    BOOST_FOREACH(Fnc *pFnc, stor.fncs) {
        Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        VK_DEBUG_MSG(2, &fnc.def.loc, ">>> entering " << nameOf(fnc) << "()...");
        VarKiller::handleFnc(data, fnc);
    }

    if (!::debugVarKiller)
        // better to work in read-only mode unless we are debugging the killer
        return;

    VarKiller::commitResults(data);
    VK_DEBUG(1, "killLocalVariables() has finished...");
}

} // namespace CodeStorage
