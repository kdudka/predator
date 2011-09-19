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
#include "symbt.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symheap.hh"
#include "symutil.hh"
#include "util.hh"

#include <map>
#include <queue>
#include <stack>
#include <utility>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

struct SymBackTrace::Private {
    struct BtStackItem {
        const CodeStorage::Fnc              &fnc;
        const struct cl_loc                 *loc;

        BtStackItem(
                const CodeStorage::Fnc      *fnc_,
                const struct cl_loc         *loc_):
            fnc(*fnc_),
            loc(loc_)
        {
        }
    };

    typedef std::stack<const IPathTracer *>                         TStackPP;
    typedef std::deque<BtStackItem>                                 TStack;
    typedef std::map<const CodeStorage::Fnc *, int /* cnt */>       TMap;

    const CodeStorage::Storage      &stor;
    const bool                      ptrace;
    TStackPP                        ppStack;
    TStack                          btStack;
    TMap                            nestMap;

    Private(const CodeStorage::Storage &stor_, const bool ptrace_):
        stor(stor_),
        ptrace(ptrace_)
    {
    }

    const CodeStorage::Fnc* fncOnTop() const;
    const CodeStorage::Fnc* fncById(int id) const;

    void pushFnc(
            const CodeStorage::Fnc          *fnc,
            const struct cl_loc             *loc);

    void popFnc();
};

const CodeStorage::Fnc* SymBackTrace::Private::fncOnTop() const {
    if (this->btStack.empty())
        // empty stack, so there is no top
        return 0;

    const BtStackItem &top = this->btStack.front();
    const CodeStorage::Fnc *fnc = &top.fnc;

    // check bt integrity
    CL_BREAK_IF(!fnc);

    return fnc;
}

const CodeStorage::Fnc* SymBackTrace::Private::fncById(int id) const {
    const CodeStorage::Fnc *fnc = this->stor.fncs[id];

    // check fnc ID validity
    CL_BREAK_IF(!fnc);

    return fnc;
}

void SymBackTrace::Private::pushFnc(
        const CodeStorage::Fnc          *fnc,
        const struct cl_loc             *loc)
{
    const BtStackItem item(fnc, loc);
    this->btStack.push_front(item);

    int &ref = this->nestMap[fnc];

    // check bt integrity
    CL_BREAK_IF(ref < 0 || static_cast<int>(this->btStack.size()) < ref);

    // increment instance counter
    ++ref;
}

void SymBackTrace::Private::popFnc() {
    const CodeStorage::Fnc *fnc = this->fncOnTop();
    this->btStack.pop_front();

    // decrement instance counter
    int &ref = this->nestMap[fnc];
    --ref;

    if (!ref) {
        // remove any dangling key eventually
        this->nestMap.erase(fnc);
        return;
    }

    // check bt integrity
    CL_BREAK_IF(ref < 0 || static_cast<int>(this->btStack.size()) < ref);
}

SymBackTrace::SymBackTrace(const CodeStorage::Storage &stor, bool ptrace):
    d(new Private(stor, ptrace))
{
}

SymBackTrace::SymBackTrace(const SymBackTrace &ref):
    d(new Private(*ref.d))
{
}

SymBackTrace::~SymBackTrace() {
    delete d;
}

SymBackTrace& SymBackTrace::operator=(const SymBackTrace &ref) {
    delete d;
    d = new Private(*ref.d);
    return *this;
}

const CodeStorage::Storage& SymBackTrace::stor() const {
    return d->stor;
}

void SymBackTrace::printBackTrace(bool forcePtrace) const {
    using namespace CodeStorage;

    Private::TStackPP ppStack(d->ppStack);
    const bool ptrace = !ppStack.empty() && (d->ptrace || forcePtrace);

    const Private::TStack &ref = d->btStack;
    if (!ptrace && ref.size() < 2)
        return;

    BOOST_FOREACH(const Private::BtStackItem &item, ref) {
        if (ptrace) {
            // perform path tracing at the current level
            CL_BREAK_IF(ppStack.empty());

            const IPathTracer *pp = ppStack.top();
            ppStack.pop();
            CL_BREAK_IF(!pp);

            pp->printPaths();
        }

        CL_NOTE_MSG(item.loc, "from call of " << nameOf(item.fnc) << "()");
    }
}

void SymBackTrace::pushCall(
        const int                       fncId,
        const struct cl_loc             *loc)
{
    const CodeStorage::Fnc *fnc = d->fncById(fncId);
    d->pushFnc(fnc, loc);
}

const CodeStorage::Fnc* SymBackTrace::popCall() {
    const CodeStorage::Fnc *fnc = d->fncOnTop();

    // check bt integrity
    CL_BREAK_IF(!fnc);

    d->popFnc();
    return fnc;
}

unsigned SymBackTrace::size() const {
    return d->btStack.size();
}

int SymBackTrace::countOccurrencesOfFnc(int fncId) const {
    const CodeStorage::Fnc *fnc = d->fncById(fncId);
    return d->nestMap[fnc];
}

int SymBackTrace::countOccurrencesOfTopFnc() const {
    const CodeStorage::Fnc *fnc = d->fncOnTop();
    if (!fnc)
        // empty stack --> no occurrence
        return 0;

    return d->nestMap[fnc];
}

const CodeStorage::Fnc* SymBackTrace::topFnc() const {
    return d->fncOnTop();
}

const struct cl_loc* SymBackTrace::topCallLoc() const {
    CL_BREAK_IF(d->btStack.empty());
    const Private::BtStackItem &top = d->btStack.front();
    return top.loc;
}

void SymBackTrace::pushPathTracer(const IPathTracer *pp) {
    d->ppStack.push(pp);
}

void SymBackTrace::popPathTracer(const IPathTracer *pp) {
    CL_BREAK_IF(d->ppStack.empty());
    CL_BREAK_IF(d->ppStack.top() != pp);
    (void) pp;
    d->ppStack.pop();
}
