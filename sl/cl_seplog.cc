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

#include "cl_seplog.hh"

#include "cl_private.hh"
#include "cl_storage.hh"
#include "storage.hh"

#include <iostream>

#include <boost/foreach.hpp>

class SymExec {
    public:
        SymExec(CodeStorage::Storage &stor):
            stor_(stor)
        {
        }

        void exec(const CodeStorage::Fnc &);

    protected:
        void exec(const CodeStorage::Insn &);
        void exec(const CodeStorage::Block &);

    private:
        CodeStorage::Storage        &stor_;
        const CodeStorage::Fnc      *fnc_;
        const CodeStorage::Block    *bb_;
        LocationWriter              lw_;
};

class ClSepLog: public ClStorageBuilder {
    public:
        ClSepLog(const char *configString);
        virtual ~ClSepLog();

    protected:
        virtual void run(CodeStorage::Storage &);
};

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
void SymExec::exec(const CodeStorage::Insn &insn) {
    using namespace CodeStorage;
    lw_ = &insn.loc;

    CL_MSG_STREAM(cl_debug, lw_ << "debug: executing insn...");
}

void SymExec::exec(const CodeStorage::Block &bb) {
    using namespace CodeStorage;
    bb_ = &bb;

    CL_MSG_STREAM(cl_debug, lw_ << "debug: entering " << bb.name() << "...");
    BOOST_FOREACH(const Insn *insn, bb) {
        this->exec(*insn);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc) {
    using namespace CodeStorage;
    fnc_ = &fnc;
    lw_ = &fnc.def.loc;

    CL_MSG_STREAM(cl_debug, lw_ << "debug: entering "
            << nameOf(fnc) << "()...");

    CL_DEBUG("looking for entry block...");
    const ControlFlow &cfg = fnc.cfg;
    const Block *entry = cfg.entry();
    if (!entry) {
        CL_MSG_STREAM(cl_error, lw_ << "error: "
                << nameOf(fnc) << ": "
                << "entry block not found");
        return;
    }
    this->exec(*entry);
}


// /////////////////////////////////////////////////////////////////////////////
// ClSepLog implementation
ClSepLog::ClSepLog(const char *) {
    // TODO
}

ClSepLog::~ClSepLog() {
    // TODO
}

void ClSepLog::run(CodeStorage::Storage &stor) {
    CL_DEBUG("looking for 'main()' at gl scope...");
    CodeStorage::Fnc *main = stor.glFncByName["main"];
    if (!main) {
        CL_MSG_STREAM_INTERNAL(cl_error,
                "error: main() not declared at global scope");
        return;
    }

    // this seems to be the way to go...
    SymExec se(stor);
    se.exec(*main);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_seplog.hh for more details
ICodeListener* createClSepLog(const char *configString) {
    return new ClSepLog(configString);
}
