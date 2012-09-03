/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#include <cl/easy.hh>
#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include <map>
#include <queue>
#include <set>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

/// variable state enumeration
enum EVarState {
    VS_UNDEF,                   ///< value not assigned yet
    VS_UNKNOWN,                 ///< value has been abstracted out
    VS_DEREF,                   ///< value has been already dereferenced
    VS_NULL,                    ///< NULL value has been assigned
    VS_NOT_NULL,                ///< not-NULL value has been assigned
    VS_NULL_DEDUCED,            ///< value is guaranteed to be NULL
    VS_NOT_NULL_DEDUCED,        ///< value is guaranteed to be not-NULL
    VS_MIGHT_BE_NULL,

    VS_FALSE,                   ///< value is 'false', valid only for booleans
    VS_TRUE,                    ///< value is 'true', valid only for booleans

    VS_NULL_IFF,                ///< if true, peer is NULL and vice versa
    VS_NOT_NULL_IFF             ///< if true, peer is not-NULL and vice versa
};

/// state of variable (scope of its validity is basic block)
struct VarState {
    EVarState           code;   ///< current state (valid per current block)
    const struct cl_loc *lw;    ///< location where the state became valid
    int /* uid */       peer;   ///< used only for VS_NULL_IFF, VS_NOT_NULL_IFF

    VarState():
        code(VS_UNDEF),
        lw(0),
        peer(-1)
    {
    }
};

/// state of computation at function level
struct Data {
    typedef const CodeStorage::Block                   *TBlock;
    typedef std::queue<TBlock>                          TSched;
    typedef std::set<TBlock>                            TSchedLookup;
    typedef std::map<int /* var uid */, VarState>       TState;
    typedef std::map<TBlock, TState>                    TStateMap;

    TSched          todo;       ///< block scheduled for processing
    TSchedLookup    todoLookup; ///< block scheduled for processing
    TStateMap       stateMap;   ///< holds states of all vars per each block
};

/**
 * @param state state valid per current (dereference) instruction
 * @param op either source or destination operand that contains a dereference
 * @param lw location info of the current instruction
 */
void handleVarDeref(Data::TState                        &state,
                    const struct cl_operand             &op,
                    const struct cl_loc                 *lw)
{
    const int uid = varIdFromOperand(&op);
    VarState &vs = state[uid];
    const EVarState code = vs.code;
    switch (code) {
        case VS_UNDEF:
        case VS_UNKNOWN:
            vs.code = VS_DEREF;
            vs.lw   = lw;
            // fall through!

        case VS_NOT_NULL:
        case VS_NOT_NULL_DEDUCED:
        case VS_DEREF:
            return;

        case VS_NULL:
            CL_ERROR_MSG(lw, "dereference of NULL value");
            CL_NOTE_MSG(vs.lw, "the NULL value comes from here");
            return;

        case VS_NULL_DEDUCED:
            CL_ERROR_MSG(lw, "dereference of NULL value");
            CL_NOTE_MSG(vs.lw, "the condition seems to be used incorrectly");

        case VS_MIGHT_BE_NULL:
            CL_WARN_MSG(lw, "dereference of a value that might be NULL");
            CL_NOTE_MSG(vs.lw, "the same value was compared with NULL here");
            return;

        default:
            CL_TRAP;
    }
}

/**
 * scan the given instruction for any dereferences and handle the eventually
 * @param state state valid per current instruction
 * @param insn instruction you want to look into
 */
void handleDerefs(Data::TState &state, const CodeStorage::Insn *insn)
{
    // for each operand
    BOOST_FOREACH(const struct cl_operand &op, insn->operands) {
        const struct cl_accessor *ac = op.accessor;
        if (!ac || ac->code != CL_ACCESSOR_DEREF || seekRefAccessor(ac))
            // no dereference here
            continue;

        CL_BREAK_IF(CL_OPERAND_VAR != op.code);
        handleVarDeref(state, op, &insn->loc);
    }
}

/// returns true for VS_NOT_NULL and VS_NOT_NULL_DEDUCED
inline bool anyNotNull(const EVarState code)
{
    switch (code) {
        case VS_NOT_NULL:
        case VS_NOT_NULL_DEDUCED:
            return true;

        default:
            return false;
    }
}

/**
 * merge values (used for Y nodes of CFG)
 * @param dst destination state (used in read-write mode)
 * @param src source state (used in read-only mode)
 */
bool mergeValues(VarState &dst, const VarState &src)
{
    if (VS_UNDEF == src.code || VS_MIGHT_BE_NULL == dst.code)
        // nothing to propagate actually
        return false;

    if (src.code == dst.code)
        // codes match already
        return false;

    if (VS_NULL_IFF == src.code || VS_NOT_NULL_IFF == src.code
            || VS_NULL_IFF == dst.code || VS_NOT_NULL_IFF == dst.code)
        // we use these only block-locally
        return false;

    if (VS_UNDEF == dst.code) {
        // value not defined in the target block, let's start with the new one
        dst = src;
        return true;
    }

    if ((VS_NULL_DEDUCED == src.code && anyNotNull(dst.code))
            || (anyNotNull(src.code) && VS_NULL_DEDUCED == dst.code))
        // merge NULL and not-NULL alternatives together
    {
        dst.code = VS_MIGHT_BE_NULL;
        return true;
    }

    if (VS_UNKNOWN == dst.code)
        // no news is good news
        return false;

    // let's over-approximate everything else
    dst.code = VS_UNKNOWN;
    return true;
}

/**
 * process unary instruction
 * @param state state valid per current instruction
 * @param insn instruction you want to process
 */
void handleInsnUnop(Data::TState &state, const CodeStorage::Insn *insn)
{
    handleDerefs(state, insn);

    const struct cl_operand &dst = insn->operands[0];
    if (CL_OPERAND_VOID == dst.code || dst.accessor)
        // we're interested only in direct manipulation of variables here
        return;

    // resolve state of the variable
    const int uid = varIdFromOperand(&dst);
    VarState &vs = state[uid];

    const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn->subCode);
    if (CL_UNOP_ASSIGN != code) {
        // we abstract out everything but CL_UNOP_ASSIGN
        vs.code = VS_UNKNOWN;
        return;
    }

    // resolve source operand of the instruction
    const struct cl_operand &src = insn->operands[1];
    const struct cl_accessor *ac = src.accessor;

    if (seekRefAccessor(ac)) {
        // assignment of address of an object implies not-NULL value
        vs.code = VS_NOT_NULL;
        vs.lw   = &insn->loc;
        return;
    }

    if (ac || CL_TYPE_PTR != src.type->code) {
        // we abstract out everything but pointers
        vs.code = VS_UNKNOWN;
        return;
    }

    if (CL_OPERAND_CST == src.code) {
        if (CL_TYPE_INT != src.data.cst.code || intCstFromOperand(&src)) {
            vs.code = VS_UNKNOWN;
            return;
        }

        // looks like assignment of NULL to a variable
        vs.code = VS_NULL;
        vs.lw   = &insn->loc;
        return;
    }

    // single assignment ... let's just propagate the value
    const int uidSrc = varIdFromOperand(&src);
    mergeValues(vs, state[uidSrc]);
}

/**
 * handle comparison of a pointer with NULL
 * @param state state valid per current instruction
 * @param vsDst reference to state of destination variable
 * @param src the operand that is not NULL
 * @param lw location info of the current instruction
 * @param neg if true, we deal with !=; == otherwise
 */
bool handleInsnCmpNull(Data::TState                 &state,
                       VarState                     &vsDst,
                       const struct cl_operand      *src,
                       const struct cl_loc          *lw,
                       bool                         neg)
{
    if (src->accessor)
        // we're interested only in direct manipulation of variables here
        return false;

    if (CL_TYPE_PTR != src->type->code)
        // we're interested only in pointers comparison here
        return false;

    const int uidSrc = varIdFromOperand(src);
    const VarState &vsSrc = state[uidSrc];
    const EVarState code = vsSrc.code;
    switch (code) {
        case VS_NULL:
        case VS_NULL_DEDUCED:
            neg = !neg;
            // fall through!

        case VS_NOT_NULL:
        case VS_NOT_NULL_DEDUCED:
            goto we_know;

        case VS_UNDEF:
        case VS_UNKNOWN:
        case VS_MIGHT_BE_NULL:
            break;

        case VS_DEREF:
            CL_WARN_MSG(lw, "comparing pointer with NULL");
            CL_NOTE_MSG(vsSrc.lw, "the pointer was already dereferenced here");
            break;

        default:
            CL_TRAP;
    }

    // now store the relation among the pointer and the result of the comparison
    vsDst.code = (neg)
        ? VS_NOT_NULL_IFF
        : VS_NULL_IFF;

    vsDst.peer = uidSrc;
    vsDst.lw   = lw;
    return true;

we_know:
    // we already know the result of the comparison at this point
    vsDst.code = (neg)
        ? VS_TRUE
        : VS_FALSE;

    vsDst.lw = lw;
    return true;
}

/**
 * process binary instruction
 * @param state state valid per current instruction
 * @param insn instruction you want to process
 */
void handleInsnBinop(Data::TState &state, const CodeStorage::Insn *insn)
{
    const CodeStorage::TOperandList &opList = insn->operands;

#ifndef NDEBUG
    // binary instructions are said to have no dereferences
    // (better to check anyway)
    BOOST_FOREACH(const struct cl_operand &op, opList) {
        const struct cl_accessor *ac = op.accessor;
        if (ac && ac->code == CL_ACCESSOR_DEREF)
            CL_TRAP;
    }
#endif

    // resolve operands
    const struct cl_operand &dst = opList[0];
    CL_BREAK_IF(dst.accessor);
    const int uidDst = varIdFromOperand(&dst);
    VarState &vs = state[uidDst];

    const struct cl_operand &src1 = opList[1];
    const struct cl_operand &src2 = opList[2];
    const struct cl_operand *src;

    // now check the actual type of the binary instruction
    const enum cl_binop_e code = static_cast<enum cl_binop_e>(insn->subCode);
    switch (code) {
        case CL_BINOP_EQ:
        case CL_BINOP_NE:
            // these are the only instruction we're interested in
            break;

        default:
            goto who_knows;
    }

    if (CL_OPERAND_CST != src1.code && CL_OPERAND_CST != src2.code)
        // we take only comparison with NULL
        goto who_knows;

    if (CL_OPERAND_CST == src1.code) {
        if (CL_TYPE_INT != src1.data.cst.code || intCstFromOperand(&src1))
            // not NULL, sorry
            goto who_knows;

        src = &src2;
    }
    else /* if (CL_OPERAND_CST == src2.code) */ {
        if (CL_TYPE_INT != src2.data.cst.code || intCstFromOperand(&src2))
            // not NULL, sorry
            goto who_knows;

        src = &src1;
    }

    if (handleInsnCmpNull(state, vs, src, &insn->loc, (CL_BINOP_NE == code)))
        // properly handled pointer comparison with NULL
        return;

who_knows:
    vs.code = VS_UNKNOWN;
}

/**
 * process call instruction
 * @param state state valid per current instruction
 * @param insn instruction you want to process
 */
void handleInsnCall(Data::TState &state, const CodeStorage::Insn *insn)
{
    const struct cl_operand &dst = insn->operands[0];
    if (dst.accessor)
        // we're interested only in direct manipulation of variables here
        return;

    if (CL_OPERAND_VAR != dst.code)
        return;

    // abstract out the return value
    const int uid = varIdFromOperand(&dst);
    VarState &vs = state[uid];
    vs.code = VS_UNKNOWN;
}

/**
 * abstract out any reasoning in case of direct reference of an operand
 * @param state state valid per current instruction
 * @param opList list of operands to check for direct references
 */
void treatRefAsSideEffect(Data::TState                          &state,
                          const CodeStorage::TOperandList       &opList)
{
    // for each operand
    BOOST_FOREACH(const struct cl_operand &op, opList) {
        if (CL_OPERAND_VAR != op.code)
            // not a variable
            continue;

        const struct cl_accessor *ac = op.accessor;
        if (!ac || ac->code != CL_ACCESSOR_REF)
            // not a direct reference
            continue;

        // kill any up to now reasoning about the variable
        const int uid = varIdFromOperand(&op);
        state[uid].code = VS_UNKNOWN;
    }
}

/**
 * process a nonterminal instruction
 * @param state state valid per current instruction
 * @param insn instruction you want to process
 */
void handleInsnNonterm(Data::TState &state, const CodeStorage::Insn *insn)
{
    treatRefAsSideEffect(state, insn->operands);

    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_UNOP:
            handleInsnUnop(state, insn);
            return;

        case CL_INSN_BINOP:
            handleInsnBinop(state, insn);
            return;

        case CL_INSN_CALL:
            handleInsnCall(state, insn);
            break;

        case CL_INSN_LABEL:
            // should be safe to ignore
            break;

        default:
            CL_BREAK_IF("unhandled insn in handleInsnNonterm()");
    }
}

/**
 * merge states (used for Y nodes of CFG)
 * @param data state of computation per current function
 * @param state state valid per current instruction
 * @param block block that determines the target state that is about to be
 * updated
 */
void updateState(Data                           &data,
                 const Data::TState             &state,
                 const CodeStorage::Block       *block)
{
    // target state
    typedef Data::TState TState;
    TState &dstState = data.stateMap[block];

    // for each variable
    bool changed = false;
    BOOST_FOREACH(TState::const_reference item, state) {
        if (mergeValues(dstState[item.first], item.second))
            changed = true;
    }

    if (changed && data.todoLookup.insert(block)./* not already in */second)
        data.todo.push(block);
}

/**
 * replace state of the branch-by variable by VS_NULL_DEDUCED or
 * VS_NOT_NULL_DEDUCED
 * @param state state valid per current instruction
 * @param uid CodeStorage uid of the branch-by variable
 * @param val true in 'then' branch, false in 'else' branch
 */
void replaceInBranch(Data::TState &state, int uid, bool val)
{
    VarState &vs = state[uid];
    bool isNull;

    const EVarState code = vs.code;
    switch (code) {
        case VS_NULL_IFF:
            isNull = val;
            break;

        case VS_NOT_NULL_IFF:
            isNull = !val;
            break;

        case VS_UNKNOWN:
            // giving up
            return;

        default:
            CL_TRAP;
            return;
    }

    // kill the pending condition predicate
    vs.code = VS_UNDEF;

    // update state of the pointer accordingly
    VarState &vsPeer = state[vs.peer];
    vsPeer.lw = vs.lw;
    vsPeer.code = (isNull)
        ? VS_NULL_DEDUCED
        : VS_NOT_NULL_DEDUCED;
}

/**
 * handle a condition for which we don't know the branch-by value
 * @param data state of computation per current function
 * @param state state valid per current instruction
 * @param cond branch-by variable given as operand
 * @param targets then/else targets of the condition
 */
void handleInsnCondNondet(Data                              &data,
                          const Data::TState                &state,
                          const struct cl_operand           &cond,
                          const CodeStorage::TTargetList    &targets)
{
    // local copies of the state
    Data::TState stateThen(state);
    Data::TState stateElse(state);

    // reflect the value of branch-by variable (if possible)
    int uid = varIdFromOperand(&cond);
    replaceInBranch(stateThen, uid, true);
    replaceInBranch(stateElse, uid, false);

    // go to both targets and update the state there
    updateState(data, stateThen, targets[0]);
    updateState(data, stateElse, targets[1]);
}

/**
 * @param data state of computation per current function
 * @param state state valid per current instruction
 * @param insn conditional instruction you want to process
 */
void handleInsnCond(Data                                    &data,
                    const Data::TState                      &state,
                    const CodeStorage::Insn                 *insn)
{
    // resolve branch-by operand
    const struct cl_operand &cond = insn->operands[0];
    const int uid = varIdFromOperand(&cond);
    Data::TState::const_iterator it = state.find(uid);
    CL_BREAK_IF(state.end() == it);
    const VarState &vs = it->second;

    // now check if we know the value
    const EVarState code = vs.code;
    switch (code) {
        case VS_TRUE:
            updateState(data, state, insn->targets[0]);
            return;

        case VS_FALSE:
            updateState(data, state, insn->targets[1]);
            return;

        default:
            handleInsnCondNondet(data, state, cond, insn->targets);
            return;
    }
}

/**
 * process a terminal instruction
 * @param data state of computation per current function
 * @param state state valid per current instruction
 * @param insn instruction you want to process
 */
void handleInsnTerm(Data                            &data,
                    const Data::TState              &state,
                    const CodeStorage::Insn         *insn)
{
    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_COND:
            handleInsnCond(data, state, insn);
            return;

        case CL_INSN_JMP:
            // just update the target state and check if anything has changed
            updateState(data, state, insn->targets[0]);
            return;

        case CL_INSN_RET:
        case CL_INSN_ABORT:
            // we're not interested in such instructions here
            return;

        default:
            CL_TRAP;
    }
}

void handleBlock(Data &data, Data::TBlock bb)
{
    // go through the sequence of instructions of the current basic block
    Data::TState next = data.stateMap[bb];
    BOOST_FOREACH(const CodeStorage::Insn *insn, *bb) {
        if (cl_is_term_insn(insn->code))
            // terminal instruction
            handleInsnTerm(data, next, insn);

        else
            // nonterminal instruction
            handleInsnNonterm(next, insn);
    }
}

void handleFnc(const CodeStorage::Fnc &fnc)
{
    using namespace CodeStorage;

    Data data;
    Data::TSched &todo = data.todo;
    const ControlFlow &cfg = fnc.cfg;

    // block-level scheduler
    Data::TBlock bb = cfg.entry();
    todo.push(bb);
    data.todoLookup.insert(bb);
    while (!todo.empty()) {
        Data::TBlock bb = todo.front();
        todo.pop();
        if (1 != data.todoLookup.erase(bb))
            CL_BREAK_IF("BlockScheduler malfunction");

        // process one basic block
        CL_BREAK_IF(!bb || !bb->size());
        const Insn *insn = bb->operator[](0);
        CL_DEBUG_MSG(&insn->loc, "analyzing block " << bb->name() << "...");
        handleBlock(data, bb);
    }
}

// /////////////////////////////////////////////////////////////////////////////
// see easy.hh for details
void clEasyRun(const CodeStorage::Storage &stor, const char *)
{
    using namespace CodeStorage;

    BOOST_FOREACH(const Fnc *pFnc, stor.fncs) {
        const Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        CL_DEBUG_MSG(&fnc.def.data.cst.data.cst_fnc.loc, "analyzing function "
                << nameOf(fnc) << "()...");

        handleFnc(fnc);
    }
}
