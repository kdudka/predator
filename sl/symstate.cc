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
#include <cl/storage.hh>

#include "symcmp.hh"
#include "symjoin.hh"
#include "symplot.hh"
#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy_if
#include <iomanip>
#include <map>
#include <queue>

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
#if DEBUG_SYMJOIN
        if (!STREQ(name, "join"))
#endif
        if (!::debugSymState)
            return;

        std::ostringstream str;
        str << "symstate-"
            << FIXW(6) << ::cntLookups << "-" << name << "-"
            << FIXW(4) << idx;

        plotHeap(sh, str.str().c_str());
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

bool SymState::insertFast(SymHeap &sh) {
    const int idx = this->lookup(sh);
    if (-1 != idx)
        return false;

    this->insertNewFast(sh);
    return true;
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
void SymStateWithJoin::packSuffix(unsigned idx) {
    const unsigned suffix = idx++;

    while (idx < this->size()) {
        SymHeap &shNew = const_cast<SymHeap &>(this->operator[](suffix));
        SymHeap &shOld = const_cast<SymHeap &>(this->operator[](idx));

        TStorRef stor = shNew.stor();
        CL_BREAK_IF(&stor != &shOld.stor());

        EJoinStatus     status;
        SymHeap         result(stor);
        if (!joinSymHeaps(&status, &result, shNew, shOld)) {
            ++idx;
            continue;
        }

        CL_DEBUG("<J> packSuffix(): suffix = " << suffix
                << ", idx = " << idx
                << ", action = " << status);

        switch (status) {
            case JS_USE_ANY:
            case JS_USE_SH1:
                break;

            case JS_USE_SH2:
                this->swapExisting(suffix, shOld);
                break;

            case JS_THREE_WAY:
                this->swapExisting(suffix, result);
                break;
        }

        this->eraseExisting(idx);
    }
}

bool SymStateWithJoin::insertCore(
        SymHeap                 &shNew,
        const bool              feelFreeToOverwrite)
{
#if SE_DISABLE_SYMJOIN_IN_SYMSTATE
    return SymHeapUnion::insert(shNew);
#endif
    const int cnt = this->size();
    if (!cnt) {
        // no heaps inside, insert the first now
        if (feelFreeToOverwrite)
            // aggressive optimization
            this->insertNewFast(shNew);
        else
            this->insertNew(shNew);

        return true;
    }

    EJoinStatus     status;
    SymHeap         result(shNew.stor());
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

        if (feelFreeToOverwrite)
            // aggressive optimization
            this->insertNewFast(shNew);
        else
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

            if (feelFreeToOverwrite) {
                // aggressive optimization
                this->swapExisting(idx, shNew);
            }
            else {
                result = shNew;
                this->swapExisting(idx, result);
            }

            this->packSuffix(idx);
            return true;

        case JS_THREE_WAY:
            // three-way join
            CL_DEBUG("<J> three-way join with sh #" << idx);
            debugPlot("join", 0, this->operator[](idx));
            debugPlot("join", 1, shNew);
            debugPlot("join", 2, result);

            this->swapExisting(idx, result);
            this->packSuffix(idx);
            return true;
    }

    // nothing changed actually
    return false;
}

bool SymStateWithJoin::insert(const SymHeap &sh) {
    return this->insertCore(
            const_cast<SymHeap &>(sh),
            /* feelFreeToOverwrite */ false);
}

bool SymStateWithJoin::insertFast(SymHeap &sh) {
    return this->insertCore(sh, /* feelFreeToOverwrite */ true);
}


// /////////////////////////////////////////////////////////////////////////////
// BlockScheduler implementation
struct BlockScheduler::Private {
    typedef std::queue<TBlock>                              TFifo;
    typedef std::map<TBlock, unsigned /* cnt */>            TDone;

    TBlockSet           todo;
    TFifo               fifo;
    TDone               done;
};

BlockScheduler::BlockScheduler():
    d(new Private)
{
}

BlockScheduler::BlockScheduler(const BlockScheduler &tpl):
    d(new Private(*tpl.d))
{
}

BlockScheduler::~BlockScheduler() {
    delete d;
}

unsigned BlockScheduler::cntWaiting() const {
    return d->todo.size();
}

const BlockScheduler::TBlockSet& BlockScheduler::todo() const {
    return d->todo;
}

BlockScheduler::TBlockList BlockScheduler::done() const {
    TBlockList dst;
    BOOST_FOREACH(Private::TDone::const_reference item, d->done)
        dst.push_back(/* bb */ item.first);

    return dst;
}

bool BlockScheduler::schedule(const TBlock bb) {
    if (!insertOnce(d->todo, bb))
        // already in the queue
        return false;

    d->fifo.push(bb);
    return true;
}

bool BlockScheduler::getNext(TBlock *dst) {
    if (d->todo.empty())
        return false;

    // take the first block in the queue
    const TBlock bb = d->fifo.front();
    d->fifo.pop();
    if (1 != d->todo.erase(bb))
        CL_BREAK_IF("BlockScheduler malfunction");

    *dst = bb;
    d->done[bb]++;
    return true;
}

void BlockScheduler::printStats() const {
    typedef std::map<unsigned /* cnt */, TBlockList> TRMap;

    // sort d->todo by cnt
    TRMap rMap;
    BOOST_FOREACH(Private::TDone::const_reference item, d->done) {
        rMap[/* cnt */ item.second].push_back(/* bb */ item.first);
    }

    BOOST_FOREACH(TRMap::const_reference item, rMap) {
        const unsigned cnt = item.first;
        BOOST_FOREACH(const TBlock bb, /* TBlockList */ item.second) {
            const CodeStorage::Insn *first = bb->front();
            const std::string &name = bb->name();

            const char *suffix = "";
            if (hasKey(d->todo, bb))
                suffix = " [still in the queue]";

            CL_NOTE_MSG(&first->loc,
                    "___ block " << name
                    << " examined " << cnt
                    << " times" << suffix);
        }
    }
}


// /////////////////////////////////////////////////////////////////////////////
// SymStateMap implementation
struct SymStateMap::Private {
    typedef BlockScheduler::TBlock      TBlock;

    struct BlockState {
        SymStateMarked                  state;
        BlockScheduler                  inbound;
    };

    std::map<TBlock, BlockState>        cont;

    bool insert(
            const CodeStorage::Block    *dst,
            const CodeStorage::Block    *src,
            SymHeap                     &sh,
            const bool                  feelFreeToOverwrite);
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

bool SymStateMap::Private::insert(
        const CodeStorage::Block        *dst,
        const CodeStorage::Block        *src,
        SymHeap                         &sh,
        const bool                      feelFreeToOverwrite)
{
    // look for the _target_ block
    BlockState &ref = this->cont[dst];

    // insert the given symbolic heap
    const bool changed = (feelFreeToOverwrite)
        ? ref.state.insertFast(sh)
        : ref.state.insert(sh);

    if (src)
        // store inbound edge
        ref.inbound.schedule(src);

    return changed;
}

bool SymStateMap::insert(
        const CodeStorage::Block        *dst,
        const CodeStorage::Block        *src,
        const SymHeap                   &sh)
{
    return d->insert(dst, src,
            const_cast<SymHeap &>(sh),
            /* feelFreeToOverwrite */ false);
}

bool SymStateMap::insertFast(
        const CodeStorage::Block        *dst,
        const CodeStorage::Block        *src,
        SymHeap                         &sh)
{
    return d->insert(dst, src, sh, /* feelFreeToOverwrite */ true);
}

void SymStateMap::gatherInboundEdges(TContBlock                  &dst,
                                     const CodeStorage::Block    *ofBlock)
    const
{
    BlockScheduler shed(d->cont[ofBlock].inbound);

    BlockScheduler::TBlock bb;
    while (shed.getNext(&bb))
        dst.push_back(bb);
}
