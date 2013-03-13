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

#include "pointsto.hh"
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
typedef const CodeStorage::PointsTo::Graph &TPTGraph;
typedef const struct cl_loc                *TLoc;
typedef int                                 TVar;
typedef const CodeStorage::Var             *TStorVar;
typedef const CodeStorage::Fnc             *TFnc;
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

typedef std::map<int, int>                  TAliasMap;

/// shared data
struct Data {
    TStorRef                                stor;
    TBlockSet                               todo;
    TMap                                    blocks;
    TFnc                                    fnc;
    TAliasMap                               derefAliases;

    Data(TStorRef stor_):
        stor(stor_),
        fnc(0)
    {
    }
};

struct VarData {
    const Var  *v;
};

/**
 * this is used just to make some statistics of how many variables is killed
 * with help of PointsTo analysis.
 */
class PTStats {
    static PTStats *inst;

    public:
        int count;
        int fullCount;

    public:
        static PTStats *getInstance() {
            if (!inst)
                inst = new PTStats();
            return inst;
        }



    private:
        // singleton
        PTStats() :
            count(0),
            fullCount(0)
        {
        }
};
// initialize
PTStats *PTStats::inst = 0;

void countPtStat(Data &data, int uid)
{
    PTStats *stats = PTStats::getInstance();
    stats->fullCount++;

    if (hasKey(data.fnc->vars, uid))
        // is local uid
        return;

    stats->count ++;

    // killing pointer target
    VK_DEBUG(0, "killling " << uid << " by its pointer!");
}

bool isLcVar(const Var *v)
{
    return (v->code != VAR_GL);
}

void scanVar(BlockData &bData, const Var *var, bool dst, bool fieldOfComp)
{
    if (!isLcVar(var))
        // not a local variable
        return;

    if (dst && fieldOfComp)
        // if we are writing to a field of a composite type, it yet does not
        // mean we are killing the whole composite variable
        return;

    if (hasKey(bData.kill, var->uid))
        // already killed
        return;

    if (dst) {
        VK_DEBUG(3, "kill(" << var->name << ")");
        bData.kill.insert(var->uid);
        return;
    }

    // we see the operand as [src]
    if (insertOnce(bData.gen, var->uid))
        VK_DEBUG(3, "gen(" << var->name << ")");
}

void scanTarget(Data &data, BlockData &bData, const Var *origin, bool dst)
{
    // find target variable of origin pointer 
    if (!hasKey(data.derefAliases, origin->uid))
        return;
    int uidAlias = data.derefAliases[origin->uid];;

    // scan the target
    const Var *tgtVar = &data.stor.vars[uidAlias];
    scanVar(bData, tgtVar, dst, false /* always non-field-of-composite */);
}

void scanOperand(
        Data                           &data,
        BlockData                      &bData,
        const cl_operand               &op,
        bool                            upDst)
{
    VK_DEBUG(4, "scanOperand: " << op << ((upDst) ? " [dst]" : " [src]"));

    bool fieldOfComp    = false;
    bool dst            = upDst;
    bool deref          = false;

    for (const cl_accessor *ac = op.accessor; ac; ac = ac->next) {
        CL_BREAK_IF(dst && seekRefAccessor(ac));

        const enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_DEREF_ARRAY:
                // FIXME: unguarded recursion
                scanOperand(data, bData, *(ac->data.array.index), false);
                // fall through!

            case CL_ACCESSOR_DEREF:
                // see whatever operand with CL_ACCESSOR_DEREF as [src]
                dst = false;

                // check the target of this operand later on!
                deref = true;
                break;

            case CL_ACCESSOR_ITEM:
            case CL_ACCESSOR_OFFSET:
                fieldOfComp = true;
                // fall through!

            case CL_ACCESSOR_REF:
                break;
        }
    }

    if (!isLcVar(op))
        // even non-locals could have been handled ^^^^ before!
        return;

    const Var *var  = &data.stor.vars[varIdFromOperand(&op)];
    scanVar(bData, var, dst, fieldOfComp);

    if (!deref)
        return;

    if (!fieldOfComp)
        scanTarget(data, bData, var, upDst /* from origin! */ );
}

void scanInsn(Data &data, BlockData &bData, const Insn &insn)
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
            for (int i = opList.size() - 1; 0 <= i; --i) {
                const cl_operand &op = opList[i];
                scanOperand(data, bData, op, /* dst */ !i);
            }
            return;

        case CL_INSN_RET:
        case CL_INSN_COND:
        case CL_INSN_SWITCH:
            // exactly one operand
            scanOperand(data, bData, opList[/* src */ 0], /* dst */ false);
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

inline bool isPointedUid(Data &data, int uid)
{
    BOOST_FOREACH(TAliasMap::const_reference item, data.derefAliases)
        if (item.second == uid)
            return false;

    return data.stor.vars[uid].mayBePointed;
}

// this just finishes the killing-per-target work (with some debug output)
void killVariablePerTarget(
        Data                    &data,
        const TBlock            &bb,
        int                      target,
        int                      uid)
{
    TStorRef stor = data.stor;
    const TTargetList &targets = bb->targets();
    Insn &term = *const_cast<Insn *>(bb->back());
    const bool isPointed = isPointedUid(data, uid);

    VK_DEBUG_MSG(1, &term.loc, "killing variable "
            << varToString(stor, uid)
            << " per target " << targets[target]->name()
            << " by " << term);

    const KillVar kv(uid, isPointed);
    term.killPerTarget[target].insert(kv);
    countPtStat(data, uid);
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
    scanInsn(data, arena, insn);

    // handle killed variables same way as generated (make an union)
    TSet touched = arena.kill;
    touched.insert(arena.gen.begin(), arena.gen.end());

    // go through variables generated by the current instruction
    BOOST_FOREACH(TVar vKill, touched) {
        const bool isPointed = isPointedUid(data, vKill);

        if (insertOnce(live, vKill)) {
            // variable was marked as dead in following instruction -- may be
            // killed after execution of this instruction
            VK_DEBUG_MSG(1, &insn.loc, "killing variable "
                    << varToString(stor, vKill)
                    << " by " << insn);

            const KillVar kv(vKill, isPointed);
            insn.varsToKill.insert(kv);
            countPtStat(data, vKill);

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
            // by following instructions.  Therefore it must be marked as dead.
            live.erase(vKill);
            // NOTE: It is not possible to re-kill the 'vKill' for particular
            // targets *only* because:
            //   a) future turns: 'vKill' is is not generated => is dead for
            //      previous instructions => if it will be ever killed in this
            //      BasicBlock, it MUST be done for all targets together
            //   b) actual turn: it may be killed per particular target only if
            //      it was not killed for all targets (no double kill).
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

            killVariablePerTarget(data, bb, i, vKill);
        }
    }
}

void commitBlock(Data &data, TBlock bb)
{
    const TTargetList &targets = bb->targets();
    const unsigned cntTargets = targets.size();
    const bool multipleTargets = (1 < cntTargets);

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

    if (cntTargets == 0) {
        // make sure those variables are left *live* when going out of function
        BOOST_FOREACH(TAliasMap::const_reference ref, data.derefAliases)
            live.insert(ref.second);
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
            killVariablePerTarget(data, bb, target, uidLive);
            ++liveIt;
        }
    }
}

// pre-compute aliases
int alias(Data &data, int uid)
{
    TFnc fnc = data.fnc;
    const PointsTo::Graph &ptg = fnc->ptg;
    const CallGraph::Graph &cg = data.stor.callGraph;
    const CallGraph::Node *cgn = fnc->cgNode;

    if (hasKey(data.derefAliases, uid))
        return data.derefAliases[uid];

    // not computed yet
    const Var *v = &data.stor.vars[uid];

    if (!cgn || cg.hasIndirectCall || cg.hasCallback || isDead(data.stor.ptd))
        return 0;

    if (cgn->callers.size() != 1)
        // we require at most one calling function because we must be sure that
        // the found variable alias comes from parent in CallGraph and there may
        // not exist any ambiguity.
        return 0;

    const TInsnListByFnc::const_reference insnList = *cgn->callers.begin();
    if (insnList.second /* calls */ .size() != 1)
        return 0;

    const Fnc *caller = insnList.first;

    const PointsTo::Node *target = existsVar(ptg, v);
    if (!target)
        return 0;
    target = hasOutputS(target);
    if (!target)
        return 0;

    if (target->variables.size() != 1)
        return 0;

    const PointsTo::Item *item = *target->variables.begin();
    const Var *tgtVar = item->var();
    if (!tgtVar)
        return 0;

    // on the variable target resides _single_ variable.
    if (!isLcVar(tgtVar))
        return 0;

    // it is local variable, lets see if it comes from parent in CallGraph
    if (!hasKey(caller->vars, tgtVar->uid))
        return 0;

    data.derefAliases[uid] = tgtVar->uid;
    return tgtVar->uid;
}

inline void hitAlias(Data &data, const cl_operand &op)
{
    if (!op.accessor
            || op.accessor->code != CL_ACCESSOR_DEREF
            || op.code != CL_OPERAND_VAR)
        return;

    alias(data, varIdFromOperand(&op));
}

void findAliases(Data &data, Fnc &fnc)
{
    BOOST_FOREACH(const TBlock bb, fnc.cfg)
        BOOST_FOREACH(const Insn *insn, *bb)
            BOOST_FOREACH(const cl_operand &op, insn->operands)
                hitAlias(data, op);
}

void presetLive(Data &data, const TBlock bb)
{
    BlockData &bData = data.blocks[bb];

    int size = bb->targets().size();
    if (size != 0)
        return;

    // those uids must stay alive after processing of this ending block
    BOOST_FOREACH(TAliasMap::const_reference pair, data.derefAliases)
        if (!hasKey(bData.kill, pair.second))
            bData.gen.insert(pair.second);
}

void analyzeFnc(Fnc &fnc)
{
    // shared state info
    Data data(*fnc.stor);
    data.fnc = &fnc;

    TLoc loc = &fnc.def.data.cst.data.cst_fnc.loc;
    VK_DEBUG_MSG(2, loc, ">>> entering " << nameOf(fnc) << "()");
    CL_BREAK_IF(!data.todo.empty());

    // pre-compute dereferences
    findAliases(data, fnc);

    // go through basic blocks
    BOOST_FOREACH(const TBlock bb, fnc.cfg) {
        // go through instructions in forward direction
        VK_DEBUG(3, "in block " << bb->name());

        BlockData &bData = data.blocks[bb];
        BOOST_FOREACH(const Insn *insn, *bb) {
            scanInsn(data, bData, *insn);
        }

        // guarantee to distribute pointer-targests exist when function finishes
        presetLive(data, bb);

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

    VarKiller::PTStats *stats = VarKiller::PTStats::getInstance();
    if (stats->count > 0) {
        VK_DEBUG(0, "there was killed " << stats->count 
                << "/" << stats->fullCount << " variables by PointsTo");
    }

    CL_DEBUG("killLocalVariables() took " << watch);
}

} // namespace CodeStorage
