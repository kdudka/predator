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

template <class TBlock>
LocationWriter digBlockLocation(TBlock bb, bool backward)
{
    using CodeStorage::Insn;

    if (backward) {
        // pick up the location of the _last_ insn with valid location
        BOOST_REVERSE_FOREACH(const Insn *insn, *bb) {
            const LocationWriter lw(&insn->loc);
            if (lw)
                return lw;
        }
    }

    else {
        // pick up the location of the _first_ insn with valid location
        BOOST_FOREACH(const Insn *insn, *bb) {
            const LocationWriter lw(&insn->loc);
            if (lw)
                return lw;
        }
    }

    // not found
    return LocationWriter();
}

template <class TBlock>
void printOneBlock(TBlock bb, int level, bool backward) {
    using std::string;

    const LocationWriter lw = digBlockLocation(bb, backward);
    const string &name = bb->name();
    const string indent(level << 2, ' ');

    string suffix;
    if (!backward && (bb == bb->cfg()->entry()))
        suffix = " [entry block]";

    CL_NOTE_MSG(lw, indent << "<-- abstract state reachable from "
            << name << suffix);
}

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

void PathTracer::printPath() const {
    typedef const CodeStorage::Block *TBlock;
    if (!block_)
        return;

    // first print the starting point
    printOneBlock(block_, 0, /* backward */ false);

    // we use std::set to avoid an infinite recursion
    std::set<TBlock> done;

    // DFS stack
    const PStackItem item(smap_, block_);
    std::stack<PStackItem> pstack;
    pstack.push(item);

    while (!pstack.empty()) {
        PStackItem &top = pstack.top();
        const SymStateMap::TContBlock &inbound = top.inbound;
        if (inbound.size() == top.nth) {
            // done at this level
            pstack.pop();
            continue;
        }

        const unsigned level = pstack.size();
        const TBlock src = inbound[top.nth++];

        // print end of the inbound block
        printOneBlock(src, (level << 1) - 1, /* backward */ true);

        // check if the path is already traversed
        if (hasKey(done, src)) {
            pstack.pop();
            continue;
        }
        else
            done.insert(src);

        // print begin of the inbound block
        printOneBlock(src, (level << 1), /* backward */ false);

        const PStackItem next(smap_, src);
        pstack.push(next);
    }
}
