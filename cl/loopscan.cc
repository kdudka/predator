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

void analyseFnc(Fnc &fnc) {
    const TLoc loc = &fnc.def.data.cst.data.cst_fnc.loc;
    LS_DEBUG_MSG(2, loc, ">>> entering " << nameOf(fnc) << "()");
}

} // namespace LoopScan

void findLoopClosingEdges(Storage &stor) {
    StopWatch watch;

    // first go through all _defined_ functions and compute the fixed-point
    BOOST_FOREACH(Fnc *pFnc, stor.fncs) {
        Fnc &fnc = *pFnc;
        if (!isDefined(fnc))
            continue;

        // analyse a single function
        LoopScan::analyseFnc(fnc);
    }

    // commit the results in batch mode (we needed to build Data::pointed first)
    CL_DEBUG("findLoopClosingEdges() took " << watch);
}

} // namespace CodeStorage
