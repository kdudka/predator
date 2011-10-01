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
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "memdebug.hh"
#include "symbt.hh"
#include "symdump.hh"
#include "symexec.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <string>

#include <boost/foreach.hpp>

// required by the gcc plug-in API
extern "C" { int plugin_is_GPL_compatible; }

// FIXME: the implementation is amusing
void parseConfigString(SymExecParams &sep, std::string cnf) {
    using std::string;
    if (cnf.empty())
        return;

    if (string("oom") == cnf) {
        CL_DEBUG("parseConfigString: \"OOM simulation\" mode requested");
        sep.oomSimulation = true;
        return;
    }

    // TODO: make it possible to handle both parameters together (comma
    // separated list or whatever)
    // TODO: document all the parameters somewhere
    if (string("noplot") == cnf) {
        CL_DEBUG("parseConfigString: \"noplot\" mode requested");
        sep.skipPlot = true;
        return;
    }

    if (string("ptrace") == cnf) {
        CL_DEBUG("parseConfigString: \"ptrace\" mode requested");
        sep.ptrace = true;
        return;
    }

    const char *cstr = cnf.c_str();
    const char *elPrefix = "error_label:";
    const size_t elPrefixLen = strlen(elPrefix);
    if (!strncmp(cstr, elPrefix, elPrefixLen)) {
        cstr += elPrefixLen;
        CL_DEBUG("parseConfigString: error label is \"" << cstr << "\"");
        sep.errLabel = cstr;
        return;
    }

    CL_WARN("unhandled config string: \"" << cnf << "\"");
}

void digGlJunk(SymHeap &sh) {
    using namespace CodeStorage;
    TStorRef stor = sh.stor();
    SymBackTrace bt(stor);
    SymProc proc(sh, &bt);

    TValList glVars;
    sh.gatherRootObjects(glVars, isProgramVar);
    BOOST_FOREACH(const TValId root, glVars) {
        // ensure we are dealing with a gl variable
        const CVar cv(sh.cVarByRoot(root));
        CL_BREAK_IF(cv.inst);

        // dig var identity and location info
        const struct cl_loc *loc = 0;
        std::string varString = varToString(stor, cv.uid, &loc);
        CL_DEBUG_MSG(loc, "(g) destroying gl variable: " << varString);

        // destroy the junk if needed
        proc.setLocation(loc);
        proc.valDestroyTarget(root);
    }
}

void execFnc(const CodeStorage::Fnc &fnc, const SymExecParams &ep,
             bool lookForGlJunk = false)
{
    const CodeStorage::Storage &stor = *fnc.stor;
    const struct cl_loc *lw = locationOf(fnc);
    CL_DEBUG_MSG(lw, "creating fresh initial state for "
            << nameOf(fnc) << "()...");

    // run the symbolic execution
    SymStateWithJoin results;
    execute(results, SymHeap(stor), fnc, ep);
    if (!lookForGlJunk)
        return;

    CL_DEBUG_MSG(lw, "(g) looking for gl junk...");
    int hCnt = 0;
    BOOST_FOREACH(SymHeap &heap, results) {
        if (1 < results.size()) {
            CL_DEBUG_MSG(lw, "*** destroying gl variables in result #"
                    << (++hCnt));
        }

        digGlJunk(heap);
    }
}

template <class TDst>
void gatherCaleeSet(TDst &dst, const CodeStorage::FncDb &fncs) {
    using namespace CodeStorage;

    // for each function
    BOOST_FOREACH(const Fnc *pFnc, fncs) {
        const Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        // for each BB
        BOOST_FOREACH(const Block *bb, fnc.cfg) {

            // for each instruction
            BOOST_FOREACH(const Insn *insn, *bb) {

                // for each operand
                BOOST_FOREACH(const struct cl_operand &op, insn->operands) {
                    if (CL_OPERAND_CST != op.code)
                        // not a literal
                        continue;

                    const struct cl_cst &cst = op.data.cst;
                    if (CL_TYPE_FNC != cst.code)
                        // not a function
                        continue;

                    const int callee = cst.data.cst_fnc.uid;
                    dst.insert(callee);
                }
            }
        }
    }
}

void execVirtualRoots(const CodeStorage::FncDb &fncs, const SymExecParams &ep) {
    using namespace CodeStorage;
    std::set<int /* uid */> callees;
    gatherCaleeSet(callees, fncs);

    BOOST_FOREACH(const Fnc *pFnc, fncs) {
        const Fnc &fnc = *pFnc;
        if (!isDefined(fnc) || hasKey(callees, uidOf(fnc)))
            continue;

        const struct cl_loc *lw = locationOf(fnc);
        CL_DEBUG_MSG(lw, nameOf(fnc)
                << "() is defined, but not called from anywhere");

        // perform symbolic execution for a virtual root
        execFnc(fnc, ep);
        printMemUsage("execFnc");
    }
}

void launchSymExec(const CodeStorage::Storage &stor, const SymExecParams &ep) {
    using namespace CodeStorage;

    // look for main() by name
    CL_DEBUG("looking for 'main()' at gl scope...");
    const NameDb::TNameMap &glNames = stor.fncNames.glNames;
    const NameDb::TNameMap::const_iterator iter = glNames.find("main");
    if (glNames.end() == iter) {
        CL_WARN("main() not found at global scope");
        execVirtualRoots(stor.fncs, ep);
        return;
    }

    // look for definition of main()
    const FncDb &fncs = stor.fncs;
    const Fnc *main = fncs[iter->second];
    if (!main || !isDefined(*main)) {
        CL_WARN("main() not defined");
        execVirtualRoots(stor.fncs, ep);
        return;
    }

    // just execute the main() function
    execFnc(*main, ep, /* lookForGlJunk */ true);
    printMemUsage("execFnc");
}

// /////////////////////////////////////////////////////////////////////////////
// see easy.hh for details
void clEasyRun(const CodeStorage::Storage &stor, const char *configString) {
    // read parameters of symbolic execution
    SymExecParams ep;
    parseConfigString(ep, configString);

    // run symbolic execution
    launchSymExec(stor, ep);
    printPeakMemUsage();
}
