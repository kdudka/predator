/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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
#include "loopscan.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "util.hh"
#include "stopwatch.hh"

#include <set>
#include <stack>

#include <boost/foreach.hpp>

static int debugLoopScan = CL_DEBUG_LOOP_SCAN;

#define LS_DEBUG(level, ...) do {                                           \
    if ((level) <= ::debugLoopScan)                                         \
        CL_DEBUG("LoopScan: " << __VA_ARGS__);                              \
} while (0)

#define LS_DEBUG_MSG(level, lw, ...) do {                                   \
    if ((level) <= ::debugLoopScan)                                         \
        CL_DEBUG_MSG(lw, "LoopScan: " << __VA_ARGS__);                      \
} while (0)

namespace CodeStorage {

namespace LoopScan {

typedef const struct cl_loc                *TLoc;
typedef const Block                        *TBlock;
typedef std::set<TBlock>                    TBlockSet;

struct DfsItem {
    TBlock                      bb;
    unsigned                    target;

    DfsItem(TBlock bb_):
        bb(bb_),
        target(0)
    {
    }
};

typedef std::stack<DfsItem>                 TDfsStack;

typedef std::pair<TBlock, TBlock>           TCfgEdge;
typedef std::set<TCfgEdge>                  TEdgeSet;

void analyzeFnc(Fnc &fnc)
{
    const TLoc loc = &fnc.def.data.cst.data.cst_fnc.loc;
    LS_DEBUG_MSG(2, loc, ">>> entering " << nameOf(fnc) << "()");

    TEdgeSet loopClosingEdges;
    TBlockSet pathSet, done;

    const TBlock entry = fnc.cfg.entry();
    if (!entry->inbound().empty())
        pathSet.insert(entry);

    const DfsItem item(entry);
    TDfsStack dfsStack;
    dfsStack.push(item);

    while (!dfsStack.empty()) {
        DfsItem &top = dfsStack.top();
        const TBlock bb = top.bb;

        const TTargetList &tlist = top.bb->targets();
        if (tlist.size() <= top.target) {
            // done at this level
            if (!insertOnce(done, bb))
                CL_BREAK_IF("LoopScan::analyzeFnc() malfunction");

            pathSet.erase(bb);
            dfsStack.pop();
            continue;
        }

        const unsigned target = top.target++;
        const TBlock bbNext = tlist[target];
        if (hasKey(done, bbNext))
            // already traversed
            continue;

        if (!hasKey(pathSet, bbNext)) {
            // nest
            const DfsItem next(bbNext);
            dfsStack.push(next);
            if (1 < bbNext->inbound().size())
                pathSet.insert(bbNext);

            continue;
        }

        const TCfgEdge edge(bb, bbNext);
        if (!insertOnce(loopClosingEdges, edge))
            // already handled
            continue;

#if CL_DEBUG_LOOP_SCAN
        // pick up the location of the _last_ insn with valid location
        TLoc edgeLoc = 0;
        BOOST_REVERSE_FOREACH(const CodeStorage::Insn *insn, *bb) {
            const TLoc loc = &insn->loc;
            if (loc->file) {
                edgeLoc = loc;
                break;
            }
        }

        LS_DEBUG_MSG(1, edgeLoc, "loop-closing edge detected: "
                << bb->name() << " -> " << bbNext->name()
                << " (target #" << target << ")");
#endif

        // append a new loop-edge
        Insn *term = const_cast<Insn *>(bb->back());
        term->loopClosingTargets.push_back(target);
    }
}

} // namespace LoopScan

void findLoopClosingEdges(Storage &stor)
{
    StopWatch watch;

    // go through all _defined_ functions
    BOOST_FOREACH(Fnc *pFnc, stor.fncs) {
        Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        // analyze a single function
        LoopScan::analyzeFnc(fnc);
    }

    // print time elapsed
    CL_DEBUG("findLoopClosingEdges() took " << watch);
}

} // namespace CodeStorage
