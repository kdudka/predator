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

#include "util.hh"

#include <stack>
#include <utility>

#include <boost/tuple/tuple.hpp>

struct SymBackTrace::Private {
    typedef std::pair<const CodeStorage::Fnc *, LocationWriter>     TStackItem;
    typedef std::stack<TStackItem>                                  TStack;

    const CodeStorage::Storage      &stor;
    TStack                          btStack;

    Private(const CodeStorage::Storage &stor_):
        stor(stor_)
    {
    }
};

SymBackTrace::SymBackTrace(const CodeStorage::Storage &stor, int rootFncId):
    d(new Private(stor))
{
    const CodeStorage::Fnc *root = d->stor.fncs[rootFncId];
    if (!root)
        // unable to resolve Fnc by UID
        TRAP;

    // first item of usual backtraces is usually main()
    push(d->btStack, root, &root->def.loc);
}

SymBackTrace::~SymBackTrace() {
    delete d;
}

void SymBackTrace::printBackTrace() {
    using namespace CodeStorage;

    const Private::TStack &ref = d->btStack;
    if (ref.size() < 2)
        return;

    Private::TStack bt(ref);
    while (!bt.empty()) {
        const Fnc *btFnc;
        LocationWriter btLw;
        boost::tie(btFnc, btLw) = bt.top();
        bt.pop();

        CL_NOTE_MSG(btLw, "from call of " << nameOf(*btFnc) << "()");
    }
}

void SymBackTrace::pushCall(int fncId, const LocationWriter &lw) {
    const CodeStorage::Fnc *fnc = d->stor.fncs[fncId];
    if (!fnc)
        // unable to resolve Fnc by UID
        TRAP;

    // TODO
    push(d->btStack, fnc, lw);
}

const CodeStorage::Fnc* SymBackTrace::popCall() {
    const Private::TStackItem top = d->btStack.top();
    const CodeStorage::Fnc *fnc = top.first;
    if (!fnc)
        // bt corruption detected
        TRAP;

    d->btStack.pop();
    return fnc;
}

unsigned SymBackTrace::size() const {
    return d->btStack.size();
}

int SymBackTrace::countOccurrencesOfFnc(int fncId) const {
    // TODO
    (void) fncId;
    TRAP;
    return -1;
}
