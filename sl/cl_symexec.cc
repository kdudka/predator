/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

#include "symbt.hh"
#include "symexec.hh"
#include "symproc.hh"
#include "symstate.hh"

#include <string>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

namespace {

void parseConfigString(SymExecParams &sep, std::string cnf) {
    using std::string;
    if (cnf.empty())
        return;

    if (string("fast") == cnf) {
        CL_DEBUG("SymExec \"fast mode\" requested");
        sep.fastMode = true;
        return;
    }

    // unhandled config string
    TRAP;
}

void digGlJunk(CodeStorage::Storage &stor, SymHeap &heap) {
    using namespace CodeStorage;
    SymBackTrace bt(stor);
    SymHeapProcessor proc(heap, &bt);

    BOOST_FOREACH(const Var &var, stor.vars) {
        if (VAR_GL == var.code) {
            const LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, "(g) destroying gl variable: #"
                    << var.uid << " (" << var.name << ")" );

            const CVar cVar(var.uid, /* gl variable */ 0);
            const TObjId obj = heap.objByCVar(cVar);
            if (obj < 0)
                TRAP;

            proc.setLocation(lw);
            proc.objDestroy(obj);
        }
    }
}

} // namespace

// /////////////////////////////////////////////////////////////////////////////
// see easy.hh for details
void clEasyRun(CodeStorage::Storage &stor, const char *configString) {
    CL_DEBUG("looking for 'main()' at gl scope...");
    const int uid = stor.fncNames.glNames["main"];
    const CodeStorage::Fnc *main = stor.fncs[uid];
    if (!main || CL_OPERAND_VOID == main->def.code) {
        CL_ERROR("main() not declared at global scope");
        return;
    }

    // lw points to declaration of main()
    const LocationWriter lw(&main->def.loc);

    // initialize SymExec
    SymExecParams sep;
    parseConfigString(sep, configString);
    SymExec se(stor, sep);

    // run the symbolic execution
    SymHeapUnion results;
    se.exec(*main, results);

    CL_DEBUG_MSG(lw, "(g) looking for gl junk...");
    int hCnt = 0;
    BOOST_FOREACH(SymHeap &heap, results) {
        if (1 < results.size()) {
            CL_DEBUG_MSG(lw, "*** destroying gl variables in result #"
                    << (++hCnt));
        }

        digGlJunk(stor, heap);
    }
}
