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
    VS_MIGHT_BE_NULL
};

struct VarState {
    EVarState           code;
    LocationWriter      lw;

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
        const int val = intCstFromOperand(&src);
        SE_BREAK_IF(val);
        vs.code = VS_NULL;
        vs.lw   = &dst.loc;
        return;
    }

    const int uidSrc = varIdFromOperand(&src);
    vs = state[uidSrc];
}

void handleInsnBinop(Data::TState &state, const CodeStorage::Insn *insn) {
    handleDerefs(state, insn->operands);
    // TODO
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

bool handleInsnTerm(const Data::TState &next, Data &data,
                    const CodeStorage::Insn *insn)
{
    return false;
}

bool /* changed */ handleBlock(Data &data, Data::TBlock bb) {
    bool anyChange = false;

    Data::TState next = data.stateMap[bb];
    BOOST_FOREACH(const CodeStorage::Insn *insn, *bb) {
        if (!cl_is_term_insn(insn->code))
            handleInsnNonTerm(next, insn);
        else if (handleInsnTerm(next, data, insn))
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
