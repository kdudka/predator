/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "symexec.hh"

#include "cl_private.hh"
#include "location.hh"
#include "storage.hh"
#include "symheap.hh"

#include <iostream>

#include <boost/foreach.hpp>

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct SymExec::Private {
    CodeStorage::Storage        &stor;
    const CodeStorage::Fnc      *fnc;
    const CodeStorage::Block    *bb;
    LocationWriter              lw;
    SymbolicHeap::SymHeap       heap;

    Private(CodeStorage::Storage &stor_):
        stor(stor_)
    {
    }
};

SymExec::SymExec(CodeStorage::Storage &stor):
    d(new Private(stor))
{
}

SymExec::~SymExec() {
    delete d;
}

void execUnary(SymbolicHeap::SymHeap &heap, const CodeStorage::Insn &insn) {
    enum cl_unop_e code = static_cast<enum cl_unop_e> (insn.subCode);
    if (CL_UNOP_ASSIGN != code)
        // not implemented yet
        TRAP;

}

void SymExec::exec(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;
    d->lw = &insn.loc;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: executing insn...");
    enum cl_insn_e code = insn.code;
    switch (code) {
        case CL_INSN_UNOP:
            execUnary(d->heap, insn);
            break;

        default:
            TRAP;
    }
}

void SymExec::exec(const CodeStorage::Block &bb) {
    using namespace CodeStorage;
    d->bb = &bb;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: entering " << bb.name() << "...");
    BOOST_FOREACH(const Insn *insn, bb) {
        this->exec(*insn);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using namespace CodeStorage;
    d->fnc = &fnc;
    d->lw = &fnc.def.loc;

    CL_MSG_STREAM(cl_debug, d->lw << "debug: entering "
            << nameOf(fnc) << "()...");

    using namespace CodeStorage;
    BOOST_FOREACH(const Var &var, fnc.vars) {
        CL_DEBUG("--- creating stack variable: " << var.uid
                << " (" << var.name << ")" );
        d->heap.varCreate(var.clt, var.uid);
    }

    CL_DEBUG("looking for entry block...");
    const ControlFlow &cfg = fnc.cfg;
    const Block *entry = cfg.entry();
    if (!entry) {
        CL_MSG_STREAM(cl_error, d->lw << "error: "
                << nameOf(fnc) << ": "
                << "entry block not found");
        return;
    }
    this->exec(*entry);
}
