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
#include <cl/location.hh>
#include <cl/storage.hh>

#include "util.hh"

#include <map>
#include <stack>
#include <utility>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

struct SymBackTrace::Private {
    struct BtStackItem {
        const CodeStorage::Fnc      &fnc;
        const LocationWriter        lw;

        BtStackItem(const CodeStorage::Fnc *fnc_, const LocationWriter &lw_):
            fnc(*fnc_),
            lw(lw_)
        {
        }
    };

    typedef std::stack<const IPathTracer *>                         TStackPP;
    typedef std::stack<BtStackItem>                                 TStack;
    typedef std::map<const CodeStorage::Fnc *, int /* cnt */>       TMap;

    const CodeStorage::Storage      &stor;
    TStackPP                        ppStack;
    TStack                          btStack;
    TMap                            nestMap;
    TFncSeq                         fncSeq;

    Private(const CodeStorage::Storage &stor_):
        stor(stor_)
    {
    }

    const CodeStorage::Fnc* fncOnTop() const;
    const CodeStorage::Fnc* fncById(int id) const;
    void pushFnc(const CodeStorage::Fnc *, const LocationWriter &);
    void popFnc();
};

const CodeStorage::Fnc* SymBackTrace::Private::fncOnTop() const {
    if (this->btStack.empty())
        // empty stack, so there is no top
        return 0;

    const BtStackItem &top = this->btStack.top();
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

void SymBackTrace::Private::pushFnc(const CodeStorage::Fnc *fnc,
                                    const LocationWriter   &lw)
{
    const BtStackItem item(fnc, lw);
    this->btStack.push(item);
    this->fncSeq.push_back(uidOf(*fnc));

    int &ref = this->nestMap[fnc];

    // check bt integrity
    CL_BREAK_IF(ref < 0 || static_cast<int>(this->btStack.size()) < ref);

    // increment instance counter
    ++ref;
}

void SymBackTrace::Private::popFnc() {
    const CodeStorage::Fnc *fnc = this->fncOnTop();
    this->btStack.pop();
    this->fncSeq.pop_back();

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

SymBackTrace::SymBackTrace(const CodeStorage::Storage &stor):
    d(new Private(stor))
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

void SymBackTrace::printBackTrace() const {
    using namespace CodeStorage;

    Private::TStackPP ppStack(d->ppStack);
    const bool ptrace = !ppStack.empty();

    const Private::TStack &ref = d->btStack;
    if (!ptrace && ref.size() < 2)
        return;

    Private::TStack bt(ref);
    while (!bt.empty()) {
        const Private::BtStackItem item = bt.top();
        bt.pop();

        if (ptrace) {
            // perform path tracing at the current level
            CL_BREAK_IF(ppStack.empty());

            const IPathTracer *pp = ppStack.top();
            ppStack.pop();
            CL_BREAK_IF(!pp);

            pp->printPaths();
        }

        CL_NOTE_MSG(item.lw, "from call of " << nameOf(item.fnc) << "()");
    }
}

void SymBackTrace::pushCall(int fncId, const LocationWriter &lw) {
    const CodeStorage::Fnc *fnc = d->fncById(fncId);
    d->pushFnc(fnc, lw);
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

LocationWriter SymBackTrace::topCallLoc() const {
    if (d->btStack.empty())
        // empty stack, so there is no top
        return LocationWriter();

    const Private::BtStackItem &top = d->btStack.top();
    return top.lw;
}

bool SymBackTrace::hasRecursiveCall() const {
    BOOST_FOREACH(const Private::TMap::value_type &item, d->nestMap) {
        const int cnt = item.second;
        CL_BREAK_IF(cnt <= 0);

        if (1 < cnt)
            // a recursive call has been found
            return true;
    }

    // found nothing interesting, looks like a regular bt
    return false;
}

SymBackTrace::TFncSeq& SymBackTrace::getFncSequence() const {
    return d->fncSeq;
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
