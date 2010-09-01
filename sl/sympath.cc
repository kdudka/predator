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
#include "sympath.hh"

#include <cl/cl_msg.hh>
#include <cl/location.hh>
#include <cl/storage.hh>

#include "symstate.hh"
#include "worklist.hh"

#include <set>
#include <stack>

#include <boost/foreach.hpp>

/// get location of the first (or last) insn with valid location info
template <class TBlock>
LocationWriter digBlockLocation(TBlock bb, bool backward)
{
    if (backward) {
        // pick up the location of the _last_ insn with valid location
        BOOST_REVERSE_FOREACH(const CodeStorage::Insn *insn, *bb) {
            const LocationWriter lw(&insn->loc);
            if (lw)
                return lw;
        }
    }
    else {
        // pick up the location of the _first_ insn with valid location
        BOOST_FOREACH(const CodeStorage::Insn *insn, *bb) {
            const LocationWriter lw(&insn->loc);
            if (lw)
                return lw;
        }
    }

    // not found
    return LocationWriter();
}

/// print one item of the path trace
template <class TBlock>
void printOneBlock(TBlock bb, int level, bool backward, bool loop = false) {
    using std::string;

    const LocationWriter lw = digBlockLocation(bb, backward);
    const string &name = bb->name();
    const string indent(level << 2, ' ');

    string suffix;
    if (!backward && (bb == bb->cfg()->entry()))
        suffix = " [entry block]";

    if (loop)
        suffix += " [loop]";

    CL_NOTE_MSG(lw, indent << "<-- abstract state reachable from "
            << name << suffix);
}

/// path DFS stack item
struct PStackItem {
    const CodeStorage::Block    *block;
    SymStateMap::TContBlock     inbound;
    unsigned                    nth;

    PStackItem(const SymStateMap &smap, const CodeStorage::Block *block_):
        block(block_),
        nth(0)
    {
        smap.gatherInboundEdges(inbound, block);
    }
};

void PathTracer::printPaths() const {
    typedef const CodeStorage::Block *TBlock;
    if (!block_)
        // no idea where to start, giving up...
        return;

    // first print the starting point
    printOneBlock(block_, 0, /* backward */ false);

    // we use std::set to avoid an infinite recursion
    std::set<TBlock> done;

    // DFS stack
    const PStackItem item(smap_, block_);
    std::stack<PStackItem> pstack;
    pstack.push(item);

    // DFS loop
    while (!pstack.empty()) {
        PStackItem &top = pstack.top();
        const SymStateMap::TContBlock &inbound = top.inbound;
        if (inbound.size() == top.nth) {
            // done at this level
            pstack.pop();
            continue;
        }

        // do one step at the current level
        const unsigned level = pstack.size();
        const TBlock src = inbound[top.nth++];

        // check if the path is already traversed
        const bool loop = hasKey(done, src);
        if (!loop)
            done.insert(src);

        // print end of the inbound block
        printOneBlock(src, (level << 1) - 1, /* backward */ true, loop);
        if (loop) {
            // duplicated node, we're done
            pstack.pop();
            continue;
        }

        // print begin of the inbound block
        printOneBlock(src, (level << 1), /* backward */ false);

        // nest
        const PStackItem next(smap_, src);
        pstack.push(next);
    }
}
