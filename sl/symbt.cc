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

struct BtStackItem {
    const CodeStorage::Fnc          &fnc;
    const struct cl_loc             *loc;

    BtStackItem(
            const CodeStorage::Fnc  *fnc_,
            const struct cl_loc     *loc_):
        fnc(*fnc_),
        loc(loc_)
    {
    }
};

bool operator==(const BtStackItem &a, const BtStackItem &b)
{
    return (&a.fnc == &b.fnc) && (a.loc == b.loc);
}

bool operator!=(const BtStackItem &a, const BtStackItem &b)
{
    return !operator==(a, b);
}

struct SymBackTrace::Private {
    typedef std::deque<BtStackItem>                                 TStack;
    typedef std::map<const CodeStorage::Fnc *, int /* cnt */>       TMap;

    const CodeStorage::Storage      &stor;
    TStack                          btStack;
    TMap                            nestMap;

    Private(const CodeStorage::Storage &stor_):
        stor(stor_)
    {
    }

    const CodeStorage::Fnc* fncOnTop() const;
    const CodeStorage::Fnc* fncById(int id) const;

    void pushFnc(
            const CodeStorage::Fnc          *fnc,
            const struct cl_loc             *loc);

    void popFnc();
};

const CodeStorage::Fnc* SymBackTrace::Private::fncOnTop() const
{
    if (this->btStack.empty())
        // empty stack, so there is no top
        return 0;

    const BtStackItem &top = this->btStack.front();
    const CodeStorage::Fnc *fnc = &top.fnc;

    // check bt integrity
    CL_BREAK_IF(!fnc);

    return fnc;
}

const CodeStorage::Fnc* SymBackTrace::Private::fncById(int id) const
{
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

void SymBackTrace::Private::popFnc()
{
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

SymBackTrace::SymBackTrace(const CodeStorage::Storage &stor):
    d(new Private(stor))
{
}

SymBackTrace::SymBackTrace(const SymBackTrace &ref):
    d(new Private(*ref.d))
{
}

SymBackTrace::~SymBackTrace()
{
    delete d;
}

const CodeStorage::Storage& SymBackTrace::stor() const
{
    return d->stor;
}

bool SymBackTrace::printBackTrace() const
{
    const Private::TStack &ref = d->btStack;
    if (ref.size() < 2)
        return false;

    BOOST_FOREACH(const BtStackItem &item, ref)
        CL_NOTE_MSG(item.loc, "from call of " << nameOf(item.fnc) << "()");

    return true;
}

void SymBackTrace::pushCall(
        const int                       fncId,
        const struct cl_loc             *loc)
{
    const CodeStorage::Fnc *fnc = d->fncById(fncId);
    d->pushFnc(fnc, loc);
}

const CodeStorage::Fnc* SymBackTrace::popCall()
{
    const CodeStorage::Fnc *fnc = d->fncOnTop();

    // check bt integrity
    CL_BREAK_IF(!fnc);

    d->popFnc();
    return fnc;
}

unsigned SymBackTrace::size() const
{
    return d->btStack.size();
}

int SymBackTrace::countOccurrencesOfFnc(cl_uid_t fncId) const
{
    const CodeStorage::Fnc *fnc = d->fncById(fncId);
    return d->nestMap[fnc];
}

int SymBackTrace::countOccurrencesOfTopFnc() const
{
    const CodeStorage::Fnc *fnc = d->fncOnTop();
    if (!fnc)
        // empty stack --> no occurrence
        return 0;

    return d->nestMap[fnc];
}

const CodeStorage::Fnc* SymBackTrace::topFnc() const
{
    return d->fncOnTop();
}

const struct cl_loc* SymBackTrace::topCallLoc() const
{
    CL_BREAK_IF(d->btStack.empty());
    const BtStackItem &top = d->btStack.front();
    return top.loc;
}

bool areEqual(const SymBackTrace *btA, const SymBackTrace *btB)
{
    if (!btA && !btB)
        // both NULL
        return true;

    if (!btA || !btB)
        // NULL vs. non-NULL
        return false;

    if (btA->d->btStack != btB->d->btStack)
        return false;

    if (btA->d->nestMap != btB->d->nestMap)
        return false;

    // look equal
    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// part of the implementation of CallInst
CallInst::CallInst(const SymBackTrace *bt):
    uid(uidOf(*bt->topFnc())),
    inst(bt->countOccurrencesOfTopFnc())
{
}
