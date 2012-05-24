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

#include "config_cl.h"
#define __CL_IN

#include <cl/cl_msg.hh>
#include <cl/easy.hh>
#include <cl/memdebug.hh>
#include <cl/storage.hh>

#include "callgraph.hh"
#include "cl_storage.hh"
#include "killer.hh"
#include "loopscan.hh"
#include "pointsto.hh"
#include "stopwatch.hh"

#include <string>

#define _CL_PRINT_TIME(mech, watch) mech("clEasyRun() took " << watch)

#if CL_EASY_TIMER
#   define CL_PRINT_TIME(watch) _CL_PRINT_TIME(CL_NOTE, watch)
#else
#   define CL_PRINT_TIME(watch) _CL_PRINT_TIME(CL_DEBUG, watch)
#endif

class ClEasy: public ClStorageBuilder {
    public:
        ClEasy(const char *configString):
            configString_(configString)
        {
            CL_DEBUG("ClEasy initialized: \"" << configString << "\"");
            printMemUsage("ClEasy::ClEasy");
        }

    protected:
        virtual void run(CodeStorage::Storage &stor) {
            printMemUsage("buildStorage");

            if (!stor.fncs.size() && !stor.vars.size()) {
                // avoid confusing the ccache wrapper when called on empty input
                CL_DEBUG("CodeStorage::Storage appears empty, giving up...");
                return;
            }

            CL_DEBUG("building call-graph...");
            CodeStorage::CallGraph::buildCallGraph(stor);
            printMemUsage("buildCallGraph");

            CL_DEBUG("scanning CFG for loop-closing edges...");
            findLoopClosingEdges(stor);
            printMemUsage("findLoopClosingEdges");

            CL_DEBUG("perform points-to analysis...");
            pointsToAnalyse(stor, configString_);
            printMemUsage("pointsToAnalyse");

            CL_DEBUG("killing local variables...");
            killLocalVariables(stor);
            printMemUsage("killLocalVariables");

            CL_DEBUG("ClEasy is calling the analyzer...");
            StopWatch watch;
            clEasyRun(stor, configString_.c_str());
            CL_PRINT_TIME(watch);
        }

    private:
        std::string configString_;
};


// /////////////////////////////////////////////////////////////////////////////
// interface, see cl_easy.hh for details
ICodeListener* createClEasy(const char *configString)
{
    return new ClEasy(configString);
}
