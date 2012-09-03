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

#include "builtins.hh"
#include "stopwatch.hh"
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

typedef CodeStorage::Storage               &TStorRef;
typedef const struct cl_loc                *TLoc;
typedef int                                 TVar;
typedef std::set<TVar>                      TSet;
typedef const Block                        *TBlock;
typedef std::set<TBlock>                    TBlockSet;
typedef std::vector<TSet>                   TLivePerTarget;

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

void scanOperand(BlockData &bData, const cl_operand &op, bool dst)
{
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
            case CL_ACCESSOR_OFFSET:
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

void scanInsn(BlockData &bData, const Insn &insn)
{
    VK_DEBUG_MSG(3, &insn.loc, "scanInsn: " << insn);
    const TOperandList opList = insn.operands;

    const enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_CALL:
            if (isBuiltInCall(insn))
                // just pretend there is no insn
                return;
            // fall through!

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

void updateBlock(Data &data, TBlock bb)
{
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

void computeFixPoint(Data &data)
{
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

// this just finishes the killing-per-target work (with some debug output)
void killVariablePerTarget(
        TStorRef                 stor,
        const TBlock            &bb,
        int                      target,
        int                      uid)
{
    const TTargetList &targets = bb->targets();
    Insn &term = *const_cast<Insn *>(bb->back());
    const bool isPointed = stor.vars[uid].mayBePointed;

    VK_DEBUG_MSG(1, &term.loc, "killing variable "
            << varToString(stor, uid)
            << " per target " << targets[target]->name()
            << " by " << term);

    const KillVar kv(uid, isPointed);
    term.killPerTarget[target].insert(kv);
}

void commitInsn(
        Data                    &data,
        Insn                    &insn,
        TSet                    &live,
        TLivePerTarget          &livePerTarget)
{
    const TStorRef stor = data.stor;
    const TBlock bb = insn.bb;

    const TTargetList &targets = bb->targets();
    const unsigned cntTargets = targets.size();
    const bool multipleTargets = (1 < cntTargets);

    // FIXME: performance waste
    // info about which variables are generated and killed by this particular
    // instruction 'insn' could be precomputed already (in analyzeFnc() - before
    // computeFixPoint() call).
    BlockData arena;
    scanInsn(arena, insn);

    // handle killed variables same way as generated (make an union)
    TSet touched = arena.kill;
    touched.insert(arena.gen.begin(), arena.gen.end());

    // go through variables generated by the current instruction
    BOOST_FOREACH(TVar vKill, touched) {
        const bool isPointed = stor.vars[vKill].mayBePointed;

        if (insertOnce(live, vKill)) {
            // variable was marked as dead in following instruction -- may be
            // killed after execution of this instruction
            VK_DEBUG_MSG(1, &insn.loc, "killing variable "
                    << varToString(stor, vKill)
                    << " by " << insn);

            const KillVar kv(vKill, isPointed);
            insn.varsToKill.insert(kv);

            if (multipleTargets) {
                // we need to mark this as "live" for all target blocks -- just
                // to prevent following code to re-kill it again for particular
                // target
                for (unsigned i = 0; i < cntTargets; ++i)
                    livePerTarget[i].insert(vKill);
            }
        }

        if (!hasKey(arena.gen, vKill)) {
            // this variable is killed by this instruction && is _not_ generated
            // here - it must be switched to dead status
            live.erase(vKill);
            // NOTE: It is not possible to re-kill the 'vKill' for particular
            // targets *only* because:
            //   a) future turns: 'vKill' is is not generated => is dead for
            //      previous instructions => if it will be ever killed in this
            //      BasicBlock, it MUST be done for all targets together
            //   b) actual turn: it may be killed per particular target only if
            //      it was not killed for all targets (No double kill).
        }

        if (!multipleTargets)
            // we have at most one target
            continue;

        // if the actually handled variable was not killed for all targets (it
        // means that it is "live" at least in one of the block targets) try to
        // kill it for those particular targets
        for (unsigned i = 0; i < cntTargets; ++i) {
            if (!insertOnce(livePerTarget[i], vKill))
                continue;

            killVariablePerTarget(stor, bb, i, vKill);
        }
    }
}

void commitBlock(Data &data, TBlock bb)
{
    const TTargetList &targets = bb->targets();
    const unsigned cntTargets = targets.size();
    const bool multipleTargets = (1 < cntTargets);
    TStorRef stor = data.stor;

    TLivePerTarget livePerTarget;
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

    if (!multipleTargets)
        return;

    // finish this block -- there may stay some variables that are untouched by
    // this block and/but these are alive only for some of targets --> lets
    // catch these these fugitives.

    TSet::const_iterator liveIt, perTargetIt;
    for (unsigned target = 0; target < cntTargets; ++target) {
        TLivePerTarget::const_reference perTarget = livePerTarget[target];

        liveIt = live.begin();
        perTargetIt = perTarget.begin();

        // complexity O(N) (N is number of variables in live).  Note that this
        // needs the 'live' and 'livePerTarget' lists to be sorted
        while (liveIt != live.end() && perTarget.end() != perTargetIt) {

            int uidLive = *liveIt;
            int uidPerTarget = *perTargetIt;

            if (uidPerTarget < uidLive) {
                // uidPerTarget is killed for particular target
                ++perTargetIt;
                continue;
            }

            else if (uidLive == uidPerTarget) {
                ++perTargetIt;
                ++liveIt;
                continue;
            }

            // OK, now we have untouched variable 'uidLive'
            killVariablePerTarget(stor, bb, target, uidLive);
            ++liveIt;
        }
    }
}

void analyzeFnc(Fnc &fnc)
{
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

void killLocalVariables(Storage &stor)
{
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
