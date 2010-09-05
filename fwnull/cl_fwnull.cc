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
#include <cl/location.hh>
#include <cl/storage.hh>

#include <map>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

enum EVarState {
    VS_UNKNOWN,
    VS_DEREF,
    VS_NULL,
    VS_NOT_NULL,
    VS_MIGHT_BE_NULL,

    VS_FALSE,
    VS_TRUE,

    VS_NULL_IFF,
    VS_NOT_NULL_IFF
};

struct VarState {
    EVarState           code;
    LocationWriter      lw;
    int /* uid */       peer;

    VarState(): code(VS_UNKNOWN) { }
};

struct Data {
    typedef const CodeStorage::Block                   *TBlock;
    typedef std::map<int /* var uid */, VarState>       TState;
    typedef std::map<TBlock, TState>                    TStateMap;

    TStateMap stateMap;
};

void handleVarDeref(Data::TState &state, const struct cl_operand &op) {
    const LocationWriter lw(&op.loc);

    const int uid = varIdFromOperand(&op);
    VarState &vs = state[uid];
    const EVarState code = vs.code;
    switch (code) {
        case VS_UNKNOWN:
            vs.code = VS_DEREF;
            vs.lw   = lw;
            // fall through!

        case VS_DEREF:
            return;

        case VS_NULL:
            CL_ERROR_MSG(lw, "dereference of NULL value");
            CL_ERROR_MSG(vs.lw, "NULL value comes from here");
            return;

        default:
            SE_TRAP;
    }
}

void handleDerefs(Data::TState &state, const CodeStorage::TOperandList &opList)
{
    BOOST_FOREACH(const struct cl_operand &op, opList) {
        const struct cl_accessor *ac = op.accessor;
        if (!ac || ac->code != CL_ACCESSOR_DEREF || seekRefAccessor(ac))
            continue;

        const enum cl_operand_e code = op.code;
        switch (code) {
            case CL_OPERAND_ARG:
            case CL_OPERAND_VAR:
            case CL_OPERAND_REG:
                handleVarDeref(state, op);
                return;

            default:
                SE_TRAP;
        }
    }
}

void handleInsnUnop(Data::TState &state, const CodeStorage::Insn *insn) {
    handleDerefs(state, insn->operands);

    const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn->subCode);
    if (CL_UNOP_ASSIGN != code)
        return;

    const struct cl_operand &dst = insn->operands[0];
    if (dst.accessor)
        return;

    const int uid = varIdFromOperand(&dst);
    VarState &vs = state[uid];

    const struct cl_operand &src = insn->operands[1];
    const struct cl_accessor *ac = src.accessor;
    if (seekRefAccessor(ac)) {
        vs.code = VS_NOT_NULL;
        vs.lw   = &src.loc;
        return;
    }

    if (ac) {
        vs.code = VS_UNKNOWN;
        return;
    }

    if (CL_OPERAND_CST == src.code) {
        SE_BREAK_IF(intCstFromOperand(&src));
        vs.code = VS_NULL;
        vs.lw   = &dst.loc;
        return;
    }

    const int uidSrc = varIdFromOperand(&src);
    vs = state[uidSrc];
}

bool handleInsnCmpNull(Data::TState                 &state,
                       VarState                     &vsDst,
                       const struct cl_operand      *src,
                       bool                         neg)
{
    if (src->accessor)
        return false;

    const LocationWriter lw(&src->loc);

    const int uidSrc = varIdFromOperand(src);
    VarState &vsSrc = state[uidSrc];
    const EVarState code = vsSrc.code;
    switch (code) {
        case VS_NULL:
            neg = !neg;
            // fall through!

        case VS_NOT_NULL:
            goto we_know;

        case VS_UNKNOWN:
            break;

        default:
            SE_TRAP;
    }

    vsSrc.code = VS_MIGHT_BE_NULL;
    vsSrc.lw   = lw;

    vsDst.code = (neg)
        ? VS_NOT_NULL_IFF
        : VS_NULL_IFF;

    vsDst.peer = uidSrc;
    vsDst.lw   = lw;
    return true;

we_know:
    vsDst.code = (neg)
        ? VS_TRUE
        : VS_FALSE;

    vsDst.lw = lw;
    return true;
}

void handleInsnBinop(Data::TState &state, const CodeStorage::Insn *insn) {
    const CodeStorage::TOperandList &opList = insn->operands;
#if SE_SELF_TEST
    BOOST_FOREACH(const struct cl_operand &op, opList) {
        const struct cl_accessor *ac = op.accessor;
        if (ac && ac->code == CL_ACCESSOR_DEREF)
            SE_TRAP;
    }
#endif

    const struct cl_operand &src1 = opList[1];
    const struct cl_operand &src2 = opList[2];
    const struct cl_operand *src;
    const struct cl_operand &dst = opList[0];
    SE_BREAK_IF(dst.accessor);

    const int uidDst = varIdFromOperand(&dst);
    VarState &vs = state[uidDst];


    const enum cl_binop_e code = static_cast<enum cl_binop_e>(insn->subCode);
    switch (code) {
        case CL_BINOP_EQ:
        case CL_BINOP_NE:
            break;

        default:
            goto who_knows;
    }

    if (CL_OPERAND_CST != src1.code && CL_OPERAND_CST != src2.code)
        goto who_knows;

    if (CL_OPERAND_CST == src1.code) {
        if (intCstFromOperand(&src1))
            goto who_knows;

        src = &src2;
    }
    else /* if (CL_OPERAND_CST == src2.code) */ {
        if (intCstFromOperand(&src2))
            goto who_knows;

        src = &src1;
    }

    if (handleInsnCmpNull(state, vs, src, (CL_BINOP_NE == code)))
        return;

who_knows:
    vs.code = VS_UNKNOWN;
}

void handleInsnNonTerm(Data::TState &state, const CodeStorage::Insn *insn) {
    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_UNOP:
            handleInsnUnop(state, insn);
            return;

        case CL_INSN_BINOP:
            handleInsnBinop(state, insn);
            return;

        case CL_INSN_CALL:
            // we're not interested in such instructions here
            break;

        default:
            SE_TRAP;
    }
}

bool updateState(Data                           &data,
                 const Data::TState             &state,
                 const CodeStorage::Block       *block)
{
    // TODO
    return false;
}

bool handleInsnCondNondet(Data                              &data,
                          const Data::TState                &state,
                          const struct cl_operand           &cond,
                          const CodeStorage::TTargetList    &targets)
{
    // TODO
    return false;
}

bool handleInsnCond(Data &data, const Data::TState &state,
                    const CodeStorage::Insn *insn)
{
    const struct cl_operand &cond = insn->operands[0];
    const int uid = varIdFromOperand(&cond);
    Data::TState::const_iterator it = state.find(uid);
    SE_BREAK_IF(state.end() == it);
    const VarState &vs = it->second;

    const EVarState code = vs.code;
    switch (code) {
        case VS_TRUE:
            return updateState(data, state, insn->targets[0]);

        case VS_FALSE:
            return updateState(data, state, insn->targets[1]);

        default:
            return handleInsnCondNondet(data, state, cond, insn->targets);
    }
}

bool handleInsnTerm(Data                            &data,
                    const Data::TState              &state,
                    const CodeStorage::Insn         *insn)
{
    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_COND:
            return handleInsnCond(data, state, insn);

        case CL_INSN_JMP:
            return updateState(data, state, insn->targets[0]);

        case CL_INSN_RET:
        case CL_INSN_ABORT:
            return false;

        default:
            SE_TRAP;
            return false;
    }
}

bool /* changed */ handleBlock(Data &data, Data::TBlock bb) {
    bool anyChange = false;

    Data::TState next = data.stateMap[bb];
    BOOST_FOREACH(const CodeStorage::Insn *insn, *bb) {
        if (!cl_is_term_insn(insn->code))
            handleInsnNonTerm(next, insn);
        else if (handleInsnTerm(data, next, insn))
            anyChange = true;
    }

    return anyChange;
}

void handleFnc(const CodeStorage::Fnc &fnc) {
    using namespace CodeStorage;
    Data data;

    bool anyChange = false;
    do {
        BOOST_FOREACH(const Block *bb, fnc.cfg) {
            SE_BREAK_IF(!bb || !bb->size());
            const Insn *insn = bb->operator[](0);
            const LocationWriter lw(&insn->loc);
            CL_DEBUG_MSG(lw, "analyzing block " << bb->name() << "...");

            if (handleBlock(data, bb))
                anyChange = true;
        }
    } while (anyChange);
}

// /////////////////////////////////////////////////////////////////////////////
// see easy.hh for details
void clEasyRun(CodeStorage::Storage &stor, const char *) {
    using namespace CodeStorage;

    BOOST_FOREACH(const Fnc *pFnc, stor.fncs) {
        const Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        const LocationWriter lw(&fnc.def.loc);
        CL_DEBUG_MSG(lw, "analyzing function " << nameOf(fnc) << "()...");
        handleFnc(fnc);
    }
}
