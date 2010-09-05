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
#include <cl/location.hh>
#include <cl/storage.hh>

#include <map>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

enum EVarState {
    VS_UNKNOWN,
    VS_NULL,
    VS_NOT_NULL,
    VS_MIGHT_BE_NULL,

    VS_FALSE,
    VS_TRUE
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

void handleInsnUnop(Data::TState &state, const CodeStorage::Insn *insn) {
}

void handleInsnBinop(Data::TState &state, const CodeStorage::Insn *insn) {
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
