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

static bool debugVarKiller = static_cast<bool>(CL_DEBUG_VAR_KILLER);

#define VK_DEBUG(...) do {                                                  \
    if (::debugVarKiller)                                                   \
        CL_DEBUG("VarKiller: " << __VA_ARGS__);                             \
} while (0)

#define VK_DEBUG_MSG(lw, ...) do {                                          \
    if (::debugVarKiller)                                                   \
        CL_DEBUG_MSG(lw, "VarKiller: " << __VA_ARGS__);                     \
} while (0)

namespace CodeStorage {

namespace VarKiller {

typedef int                                 TVar;
typedef std::set<TVar>                      TSet;
typedef const Block                         *TBlock;

struct BlockData {
    std::set<TBlock>                        refs;
    TSet                                    gen;
    TSet                                    kill;
    TSet                                    in;
    TSet                                    out;
};

typedef std::map<TBlock, BlockData>         TMap;

struct Data {
    TMap                                    blocks;
    TSet                                    pointed;
};

void scanRefs(TSet &pointed, const cl_operand &op) {
    if (CL_OPERAND_VAR != op.code)
        // not a variable
        return;

    const cl_accessor *ac = op.accessor;
    if (!ac || ac->code == CL_ACCESSOR_DEREF || !seekRefAccessor(ac))
        // not a reference
        return;

    if (::debugVarKiller)
        CL_BREAK_IF("not tested");

    const int uid = varIdFromOperand(&op);
    pointed.insert(uid);
}

void scanOperand(Data &data, TBlock bb, const cl_operand &op, bool dst) {
    if (CL_OPERAND_VAR != op.code)
        // not a variable
        return;

    BlockData &bData = data.blocks[bb];
    VK_DEBUG("scanOperand: " << op << ((dst) ? " [dst]" : " [src"));
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
        VK_DEBUG("kill(" << bb->name() << ") |= #" << uid);
        bData.kill.insert(uid);
        return;
    }

    // we see the operand as [src]
    if (hasKey(bData.gen, uid))
        // already generated
        return;

    VK_DEBUG("gen(" << bb->name() << ") |= #" << uid);
    bData.gen.insert(uid);
}

void scanInsn(Data &data, TBlock bb, const Insn &insn) {
    VK_DEBUG("scanInsn: " << insn);
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

void handleFnc(Fnc &fnc) {
    const ControlFlow &cfg = fnc.cfg;

    Data data;
    BOOST_FOREACH(TBlock bb, cfg)
        scanBlock(data, bb);

    if (::debugVarKiller)
        CL_BREAK_IF("not implemented yet");
}

} // namespace VarKiller

void killLocalVariables(Storage &stor) {
    BOOST_FOREACH(Fnc *pFnc, stor.fncs) {
        Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        VK_DEBUG_MSG(&fnc.def.loc, ">>> entering " << nameOf(fnc) << "()...");
        VarKiller::handleFnc(fnc);
    }

    VK_DEBUG("killLocalVariables() has finished...");
}

} // namespace CodeStorage
