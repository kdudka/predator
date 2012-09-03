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
#include "symtrace.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy_if
#include <iomanip>
#include <map>

#include <boost/foreach.hpp>

#if !SE_BLOCK_SCHEDULER_KIND
#   include <queue>
#endif

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
void SymState::clear() {
    BOOST_FOREACH(SymHeap *sh, heaps_)
        delete sh;

    heaps_.clear();
}

SymState::~SymState() {
    this->clear();
}

SymState& SymState::operator=(const SymState &ref) {
    // wipe the existing contents (if any)
    this->clear();

    // clone all heaps one by one
    BOOST_FOREACH(const SymHeap *sh, ref.heaps_)
        heaps_.push_back(new SymHeap(*sh));

    return *this;
}

SymState::SymState(const SymState &ref) {
    SymState::operator=(ref);
}

void SymState::insertNew(const SymHeap &sh) {
    // clone the given heap
    SymHeap *dup = new SymHeap(sh);

    // drop the unneeded Trace::CloneNode
    Trace::waiveCloneOperation(*dup);

    // append the pointer to our container
    heaps_.push_back(dup);
}

bool SymState::insert(const SymHeap &sh, bool /* allowThreeWay */ ) {
    if (-1 != this->lookup(sh))
        return false;

    // add given heap to union
    this->insertNew(sh);
    return true;
}

void SymState::rotateExisting(const int idxA, const int idxB) {
    TList::iterator itA = heaps_.begin() + idxA;
    TList::iterator itB = heaps_.begin() + idxB;
    rotate(itA, itB, heaps_.end());
}


// /////////////////////////////////////////////////////////////////////////////
// SymHeapUnion implementation
int SymHeapUnion::lookup(const SymHeap &lookFor) const {
    const int cnt = this->size();
    if (!cnt)
        // empty state --> not found
        return -1;

    ++::cntLookups;
    debugPlot("lookup", 0, lookFor);

    for(int idx = 0; idx < cnt; ++idx) {
        const int nth = idx + 1;

        const SymHeap &sh = this->operator[](idx);
        debugPlot("lookup", nth, sh);

        if (areEqual(lookFor, sh)) {
            CL_DEBUG("<I> sh #" << idx << " is equal to the given one, "
                    << cnt << " heaps in total");

#if 1 < SE_STATE_ON_THE_FLY_ORDERING
            // put the matched heap at the beginning of the list [optimization]
            const_cast<SymHeapUnion *>(this)->rotateExisting(0U, idx);
#endif
            return idx;
        }
    }

    // not found
    return -1;
}


// /////////////////////////////////////////////////////////////////////////////
// SymStateWithJoin implementation
void SymStateWithJoin::packState(unsigned idxNew, bool allowThreeWay) {
    for (unsigned idxOld = 0U; idxOld < this->size();) {
        if (idxNew == idxOld) {
            // do not remove the newly inserted heap based on identity with self
            ++idxOld;
            continue;
        }

        SymHeap &shOld = const_cast<SymHeap &>(this->operator[](idxOld));
        SymHeap &shNew = const_cast<SymHeap &>(this->operator[](idxNew));

        TStorRef stor = shNew.stor();
        CL_BREAK_IF(&stor != &shOld.stor());

        EJoinStatus     status;
        SymHeap         result(stor, new Trace::TransientNode("packState()"));
        if (!joinSymHeaps(&status, &result, shOld, shNew, allowThreeWay)) {
            ++idxOld;
            continue;
        }

        CL_DEBUG("<J> packState(): idxOld = #" << idxOld
                << ", idxNew = #" << idxNew
                << ", action = " << status
                << ", size = " << this->size());

        switch (status) {
            case JS_USE_ANY:
            case JS_USE_SH2:
                break;

            case JS_USE_SH1:
                this->swapExisting(idxNew, shOld);
                break;

            case JS_THREE_WAY:
                this->swapExisting(idxNew, result);
                break;
        }

        if (idxOld < idxNew)
            --idxNew;

        this->eraseExisting(idxOld);
    }

#if SE_STATE_ON_THE_FLY_ORDERING
    // put the matched heap at the beginning of the list [optimization]
    this->rotateExisting(0U, idxNew);
#endif
}

bool SymStateWithJoin::insert(const SymHeap &shNew, bool allowThreeWay) {
#if 1 < SE_JOIN_ON_LOOP_EDGES_ONLY
    if (!allowThreeWay)
        // we are asked not to check for entailment, only isomorphism
        return SymHeapUnion::insert(shNew, allowThreeWay);
#endif

    const int cnt = this->size();
    if (!cnt) {
        // no heaps inside, insert the first now
        this->insertNew(shNew);
        return true;
    }

    EJoinStatus     status;
    SymHeap         result(shNew.stor(),
            new Trace::TransientNode("SymStateWithJoin::insert()"));
    int             idx;

    ++::cntLookups;
    for(idx = 0; idx < cnt; ++idx) {
        const SymHeap &shOld = this->operator[](idx);
        if (joinSymHeaps(&status, &result, shOld, shNew, allowThreeWay))
            // join succeeded
            break;
    }

    if (idx == cnt) {
        // nothing to join here
        this->insertNew(shNew);
        return true;
    }

    CL_BREAK_IF(!allowThreeWay && JS_THREE_WAY == status);

    switch (status) {
        case JS_USE_ANY:
            CL_DEBUG("<I> sh #" << idx << " is equal to the given one, "
                    << cnt << " heaps in total");
            break;

        case JS_USE_SH1:
            CL_DEBUG("<J> sh #" << idx << " covers the given one, "
                    << this->size() << " heaps in total");
            debugPlot("join", 0, shNew);
            debugPlot("join", 1, this->operator[](idx));
            break;

        case JS_USE_SH2:
            // replace the heap inside by the given one
            CL_DEBUG("<J> replacing sh #" << idx
                    << ", " << cnt << " heaps in total");
            debugPlot("join", 0, this->operator[](idx));
            debugPlot("join", 1, shNew);

            result = shNew;
            Trace::waiveCloneOperation(result);
            this->swapExisting(idx, result);

            this->packState(idx, allowThreeWay);
            return true;

        case JS_THREE_WAY:
            // three-way join
            CL_DEBUG("<J> three-way join with sh #" << idx
                    << ", " << cnt << " heaps in total");

            debugPlot("join", 0, this->operator[](idx));
            debugPlot("join", 1, shNew);
            debugPlot("join", 2, result);

            this->swapExisting(idx, result);
            this->packState(idx, allowThreeWay);
            return true;
    }

#if SE_STATE_ON_THE_FLY_ORDERING
    // put the matched heap at the beginning of the list [optimization]
    this->rotateExisting(0U, idx);
#endif

    // nothing changed actually
    return false;
}


// /////////////////////////////////////////////////////////////////////////////
// BlockScheduler implementation
struct BlockScheduler::Private {
#if !SE_BLOCK_SCHEDULER_KIND
    typedef std::queue<TBlock>                              TSched;
#elif SE_BLOCK_SCHEDULER_KIND < 3
    typedef std::vector<TBlock>                             TSched;
#endif
    typedef std::map<TBlock, unsigned /* cnt */>            TDone;

    TBlockSet           todo;
#if SE_BLOCK_SCHEDULER_KIND < 3
    TSched              sched;
#endif
    TDone               done;

    const IPendingCountProvider *pcp;
};

BlockScheduler::BlockScheduler(const IPendingCountProvider &pcp):
    d(new Private)
{
    d->pcp = &pcp;
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
    if (insertOnce(d->todo, bb)) {
#if !SE_BLOCK_SCHEDULER_KIND
        d->sched.push(bb);
#elif SE_BLOCK_SCHEDULER_KIND < 3
        d->sched.push_back(bb);
#endif
        return true;
    }

    // already in the queue

#if 2 == SE_BLOCK_SCHEDULER_KIND
    const int cnt = d->sched.size();

    // seek the given block in the queue
    int idx;
    for (idx = cnt - 1; 0 <= idx; --idx)
        if (bb == d->sched[idx])
            break;

    if (idx < 0) {
        // if not found in the queue, consistency of BlockScheduler is broken
        CL_BREAK_IF("BlockScheduler::schedule() detected inconsistency!");
        return false;
    }

    if (idx + 1 == cnt)
        // already at the top
        return false;

    CL_DEBUG("<Q> prioritizing block " << bb->name()
            << ", found in depth " << (cnt - idx));

    Private::TSched::iterator itIdx = d->sched.begin() + idx;
    Private::TSched::iterator itTop = d->sched.begin() + (cnt - 1);
    rotate(itIdx, itTop, d->sched.end());
#endif

    return false;
}

bool BlockScheduler::getNext(TBlock *dst) {
    if (d->todo.empty())
        return false;

    // select the block for processing according to the policy
    TBlock bb;
#if !SE_BLOCK_SCHEDULER_KIND
    bb = d->sched.front();
    d->sched.pop();
#elif SE_BLOCK_SCHEDULER_KIND < 3
    bb = d->sched.back();
    d->sched.pop_back();

#else // assume load-driven scheduler

    typedef std::map<int /* cntPending */, TBlock> TLoad;
    TLoad load;

    // this really needs to be sorted in getNext()
    BOOST_FOREACH(const TBlock bbNow, d->todo) {
        const int cntPending = d->pcp->cntPending(bbNow);
        load[cntPending] = bbNow;
    }

    const TLoad::const_iterator itTop = load.begin();
    const TLoad::const_reverse_iterator itBottom = load.rbegin();

    bb = itTop->second;

    CL_DEBUG("<Q> load-driven scheduler picks "
            << bb->name() << " with "
            << itTop->first << " pending states, the last one is "
            << itBottom->second->name() << " with "
            << itBottom->first << " pending states");   
#endif
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
// SymStateMarked implementation
void SymStateMarked::swap(SymState &other) {
    // if this fires up one day, it means we need to cover the swap of done_
    CL_BREAK_IF(dynamic_cast<SymStateMarked *>(&other));

    // swap base
    SymStateWithJoin::swap(other);

    // wipe done
    done_.clear();
    done_.resize((cntPending_ = this->size()), false);
}

void SymStateMarked::rotateExisting(const int idxA, const int idxB) {
    SymState::rotateExisting(idxA, idxB);

    TDone::iterator itA = done_.begin() + idxA;
    TDone::iterator itB = done_.begin() + idxB;
    rotate(itA, itB, done_.end());
}


// /////////////////////////////////////////////////////////////////////////////
// SymStateMap implementation
struct SymStateMap::Private {
    typedef BlockScheduler::TBlock      TBlock;

    struct BlockState {
        SymStateMarked                  state;

        // TODO: drop this!
        static SymStateMap              XXX;
        BlockScheduler                  inbound;

        bool                            anyHit;

        BlockState():
            inbound(XXX),
            anyHit(false)
        {
        }
    };

    std::map<TBlock, BlockState>        cont;
};

// TODO: drop this!
SymStateMap SymStateMap::Private::BlockState::XXX;

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

bool SymStateMap::insert(
        const CodeStorage::Block        *dst,
        const CodeStorage::Block        *src,
        const SymHeap                   &sh,
        const bool                      allowThreeWay)
{
    // look for the _target_ block
    Private::BlockState &ref = d->cont[dst];
    const unsigned size = ref.state.size();

    // insert the given symbolic heap
    bool changed = true;
#if 2 < SE_JOIN_ON_LOOP_EDGES_ONLY
    if (1 == dst->inbound().size() && (cl_is_term_insn(dst->front()->code)
                || (CL_INSN_COND == dst->back()->code && 2 == dst->size())))
    {
        CL_DEBUG("SymStateMap::insert() bypasses even the isomorphism check");
        ref.state.insertNew(sh);
    }
    else
#endif
        changed = ref.state.insert(sh, allowThreeWay);

    if (ref.state.size() <= size)
        // if the size did not grow, there must have been at least join
        ref.anyHit = true;

    if (src)
        // store inbound edge
        ref.inbound.schedule(src);

    return changed;
}

bool SymStateMap::anyReuseHappened(const CodeStorage::Block *bb) const {
    return d->cont[bb].anyHit;
}

int SymStateMap::cntPending(const CodeStorage::Block *bb) const {
    return d->cont[bb].state.cntPending();
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
