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
#include "symstate.hh"

#include <cl/cl_msg.hh>
#include <cl/code_listener.h>

#include "symcmp.hh"
#include "symdump.hh"
#include "symjoin.hh"
#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy_if
#include <iomanip>
#include <map>

#include <boost/foreach.hpp>

#define SS_DEBUG(...) do {                                                  \
    if (::debugSymState)                                                    \
        CL_DEBUG("SymState: " << __VA_ARGS__);                              \
} while (0)

// set to 'true' if you wonder why SymState matches states as it does (noisy)
static bool debugSymState = static_cast<bool>(DEBUG_SYMSTATE);

static int cntLookups = -1;

namespace {
    void debugPlot(const char *name, int idx, const SymHeap &sh) {
#if 0//DEBUG_SYMJOIN
        if (!STREQ(name, "join"))
#endif
        if (!::debugSymState)
            return;

        std::ostringstream str;
        str << "symstate-"
            << FIXW(6) << ::cntLookups << "-" << name << "-"
            << FIXW(4) << idx;

        dump_plot(sh, str.str().c_str());
    }
}

// /////////////////////////////////////////////////////////////////////////////
// SymState implementation
bool SymState::insert(const SymHeap &sh) {
    int idx = this->lookup(sh);
    if (-1 == idx) {
        idx = this->size();

        // add given heap to union
        this->insertNew(sh);
#if DEBUG_SYMSTATE_INSERT
        CL_DEBUG("SymState::insert() has appended a new heap #" << idx);
#endif
        return true;
    }

#if DEBUG_SYMSTATE_INSERT
    CL_DEBUG("SymState::insert() has matched the given heap with heap #"
             << idx << " of "<< this->size() << " heaps total");
#endif
    return false;
}

void SymState::insert(const SymState &huni) {
    BOOST_FOREACH(const SymHeap &current, huni) {
        this->insert(current);
    }
}


// /////////////////////////////////////////////////////////////////////////////
// SymHeapUnion implementation
int SymHeapUnion::lookup(const SymHeap &lookFor) const {
    const int cnt = this->size();
    if (!cnt)
        // empty state --> not found
        return -1;

    ++::cntLookups;
    SS_DEBUG(">>> lookup() starts, cnt = " << cnt);
    debugPlot("lookup", 0, lookFor);

    for(int idx = 0; idx < cnt; ++idx) {
        const int nth = idx + 1;
        SS_DEBUG("--> lookup() tries sh #" << idx << ", cnt = " << cnt);

        const SymHeap &sh = this->operator[](idx);
        debugPlot("lookup", nth, sh);

        if (areEqual(lookFor, sh)) {
            SS_DEBUG("<<< lookup() returns sh #" << idx << ", cnt = " << cnt);
            return idx;
        }
    }

    // not found
    SS_DEBUG("<<< lookup() failed, cnt = " << cnt);
    return -1;
}


// /////////////////////////////////////////////////////////////////////////////
// SymStateWithJoin implementation
bool SymStateWithJoin::insert(const SymHeap &shNew) {
#if SE_DISABLE_SYMJOIN_IN_SYMSTATE
    return SymHeapUnion::insert(shNew);
#endif
    const int cnt = this->size();
    if (!cnt) {
        // no heaps inside, insert the first now
        this->insertNew(shNew);
        return true;
    }

    EJoinStatus     status;
    SymHeap         result;
    int             idx;

    ++::cntLookups;
    for(idx = 0; idx < cnt; ++idx) {
        const SymHeap &shOld = this->operator[](idx);
        if (joinSymHeaps(&status, &result, shOld, shNew))
            // join succeeded
            break;
    }

    if (idx == cnt) {
        // nothing to join here
        this->insertNew(shNew);
        return true;
    }

    switch (status) {
        case JS_USE_ANY:
            // JS_USE_ANY means exact match
            break;

        case JS_USE_SH1:
            CL_DEBUG("<J> sh #" << idx << " is stronger than the given one");
            debugPlot("join", 0, shNew);
            debugPlot("join", 1, this->operator[](idx));
            break;

        case JS_USE_SH2:
            // replace the heap inside by the given one
            CL_DEBUG("<J> replacing sh #" << idx);
            debugPlot("join", 0, this->operator[](idx));
            debugPlot("join", 1, shNew);

            result = shNew;
            this->swapExisting(idx, result);
            return true;

        case JS_THREE_WAY:
            // three-way join
            CL_DEBUG("<J> three-way join with sh #" << idx);
            debugPlot("join", 0, this->operator[](idx));
            debugPlot("join", 1, shNew);
            debugPlot("join", 2, result);

            this->swapExisting(idx, result);
            return true;
    }

    // nothing changed actually
    return false;
}


// /////////////////////////////////////////////////////////////////////////////
// SymStateMap implementation
struct SymStateMap::Private {
    typedef const CodeStorage::Block    *TBlock;
    typedef std::set<TBlock>            TInbound;

    struct BlockState {
        SymStateMarked                  state;
        TInbound                        inbound;
    };

    std::map<TBlock, BlockState>        cont;
};

SymStateMap::SymStateMap():
    d(new Private)
{
}

SymStateMap::~SymStateMap() {
    delete d;
}

SymStateMarked& SymStateMap::operator[](const CodeStorage::Block *bb) {
    return d->cont[bb].state;
}

bool SymStateMap::insert(const CodeStorage::Block                *dst,
                         const CodeStorage::Block                *src,
                         const SymHeap                           &sh)
{
    // look for the _target_ block
    Private::BlockState &ref = d->cont[dst];

    // insert the given symbolic heap
    const bool changed = ref.state.insert(sh);

    if (src)
        // store inbound edge
        ref.inbound.insert(src);

    return changed;
}

void SymStateMap::gatherInboundEdges(TContBlock                  &dst,
                                     const CodeStorage::Block    *ofBlock)
    const
{
    const Private::TInbound &inbound = d->cont[ofBlock].inbound;
    std::copy(inbound.begin(), inbound.end(), std::back_inserter(dst));
}
