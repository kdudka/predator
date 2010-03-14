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

#include "config.h"
#include "cl_symexec.hh"

#include "btprint.hh"
#include "cl_msg.hh"
#include "cl_storage.hh"
#include "location.hh"
#include "storage.hh"
#include "symexec.hh"
#include "symproc.hh"
#include "symstate.hh"

#include <string>

#include <boost/foreach.hpp>

class ClSymExec: public ClStorageBuilder {
    public:
        ClSymExec(const char *configString);
        virtual ~ClSymExec();

    protected:
        virtual void run(CodeStorage::Storage &);

    private:
        std::string             configString_;
        CodeStorage::Storage    *stor_;
        LocationWriter          lw_;

    private:
        void digGlJunk(SymHeap &);
        void digGlJunk(SymHeapUnion &);
};

// /////////////////////////////////////////////////////////////////////////////
// ClSymExec implementation
ClSymExec::ClSymExec(const char *configString):
    configString_(configString),
    stor_(0)
{
}

ClSymExec::~ClSymExec() {
}

void ClSymExec::digGlJunk(SymHeap &heap) {
    using namespace CodeStorage;

    // FIXME: move it to SymHeapProcessor?
    struct DummyBtPrinter: public IBtPrinter {
        virtual void printBackTrace() { }
    } bt;
    SymHeapProcessor proc(heap, &bt);

    BOOST_FOREACH(const Var &var, stor_->vars) {
        if (VAR_GL == var.code) {
            const LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, "(g) destroying gl variable: #"
                    << var.uid << " (" << var.name << ")" );

            const TObjId obj = heap.objByCVar(var.uid);
            if (obj < 0)
                TRAP;

            proc.setLocation(lw);
            proc.destroyObj(obj);
        }
    }
}

void ClSymExec::digGlJunk(SymHeapUnion &results) {
    int hCnt = 0;
    BOOST_FOREACH(SymHeap &heap, results) {
        if (1 < results.size()) {
            CL_DEBUG_MSG(lw_, "*** destroying gl variables in result #"
                    << (++hCnt));
        }

        this->digGlJunk(heap);
    }
}

namespace {
    void initExec(SymExec &se, const std::string &cnf) {
        using std::string;
        if (cnf.empty())
            return;

        if (string("fast") == cnf) {
            CL_DEBUG("SymExec \"fast mode\" requested");
            se.setFastMode(true);
            return;
        }

        // unhandled config string
        TRAP;
    }
}

void ClSymExec::run(CodeStorage::Storage &stor) {
    // keep the reference for digGlJunk(...)
    stor_ = &stor;

    CL_DEBUG("looking for 'main()' at gl scope...");
    const int uid = stor.fncNames.glNames["main"];
    const CodeStorage::Fnc *main = stor.fncs[uid];
    if (!main) {
        CL_ERROR("main() not declared at global scope");
        return;
    }

    // lw points to declaration of main()
    lw_ = &main->def.loc;

    // initialize SymExec
    SymExec se(stor);
    initExec(se, configString_);

    // run the symbolic execution
    SymHeapUnion results;
    se.exec(*main, results);

    CL_DEBUG_MSG(lw_, "(g) looking for gl junk...");
    this->digGlJunk(results);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_symexec.hh for more details
ICodeListener* createClSymExec(const char *configString) {
    return new ClSymExec(configString);
}
