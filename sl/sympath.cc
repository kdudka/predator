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
    const LocationWriter lw = digBlockLocation(bb, backward);
    const std::string &name = bb->name();
    const std::string indent(level << 1, '-');
    CL_NOTE_MSG(lw, "<" << indent << " state reachable from " << name);
}

struct PStackItem {
    const CodeStorage::Block    *block;
    unsigned                    nestLevel;

    PStackItem(const CodeStorage::Block *block_, unsigned nestLevel_ = 0):
        block(block_),
        nestLevel(nestLevel_)
    {
    }
};

void PathTracer::printPath() const {
    typedef const CodeStorage::Block *TBlock;
    if (!block_)
        return;

    // we use std::set to avoid an infinite recursion
    std::set<TBlock> done;

    // DFS stack
    PStackItem item(block_);
    std::stack<PStackItem> pstack;
    pstack.push(item);

    while (!pstack.empty()) {
        item = pstack.top();
        pstack.pop();

        TBlock bb = item.block;

        const unsigned level = item.nestLevel;
        if (level)
            printOneBlock(bb, (level << 1)    , /* backward */ true);

        // FIXME: this may be too chatty under certain circumstances
        printOneBlock(bb, (level << 1) + 1, /* backward */ false);

        // gather inbound edges
        SymStateMap::TContBlock inbound;
        smap_.gatherInboundEdges(inbound, bb);
        BOOST_FOREACH(TBlock src, inbound) {
            if (hasKey(done, src))
                // already processed
                continue;
            done.insert(src);

            // schedule for processing
            item.block = src;
            item.nestLevel = level + 1;
            pstack.push(item);
        }
    }
}
