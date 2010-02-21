/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "symplot.hh"

#include "cl_private.hh"
#include "storage.hh"
#include "symheap.hh"

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>

// singleton
class PlotEnumerator {
    public:
        static PlotEnumerator* instance() {
            return (inst_)
                ? inst_
                : (inst_ = new PlotEnumerator);
        }

        // generate kind of more unique name
        std::string decorate(std::string name);

    private:
        static PlotEnumerator *inst_;
        PlotEnumerator() { }
        // FIXME: should we care about the destruction?

    private:
        typedef std::map<std::string, int> TMap;
        TMap map_;
};


// /////////////////////////////////////////////////////////////////////////////
// implementation of PlotEnumerator
PlotEnumerator *PlotEnumerator::inst_ = 0;

std::string PlotEnumerator::decorate(std::string name) {
    // obtain a unique ID for the given name
    const int id = map_[name] ++;

    // convert the ID to string
    std::ostringstream str;
    str << std::fixed
        << std::setfill('0')
        << std::setw(/* width of the ID suffix */ 4)
        << id;

    // merge name with ID
    name += "-";
    name += str.str();
    return name;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapPlotter
struct SymHeapPlotter::Private {
    const CodeStorage::Storage      *stor;
    const SymbolicHeap::SymHeap     *heap;
    std::ofstream                   dotStream;
    LocationWriter                  lw;

    bool plotCVar(int uid);
};

bool SymHeapPlotter::Private::plotCVar(int uid) {
    const CodeStorage::Var &var = varById(*this->stor, uid);
    this->lw = &var.loc;
    CL_DEBUG_MSG(this->lw, "XXX plotting stack variable: #" << var.uid
            << " (" << var.name << ")" );

    // TODO
    return false;
}

SymHeapPlotter::SymHeapPlotter(const CodeStorage::Storage   &stor,
                               const SymbolicHeap::SymHeap  &heap):
    d(new Private)
{
    d->stor = &stor;
    d->heap = &heap;
}

SymHeapPlotter::~SymHeapPlotter() {
    delete d;
}

namespace {
    bool createDotFile(std::ofstream &str, const std::string &plotName) {
        // compute a sort of unique file name
        PlotEnumerator *pe = PlotEnumerator::instance();
        std::string fileName(pe->decorate(plotName));
        fileName += ".dot";

        // now please create the file
        str.open(fileName.c_str(), std::ios::out);
        if (!str) {
            CL_ERROR("unable to create file '" << fileName << "'");
            return false;
        }

        // all OK
        CL_DEBUG("symplot: created dot file '" << fileName << "'");
        return true;
    }
}

bool SymHeapPlotter::plot(const std::string &name) {
    using namespace SymbolicHeap;
    if (!createDotFile(d->dotStream, name))
        return false;

    bool ok = true;
    SymHeap::TCont cVars;
    d->heap->gatherCVars(cVars);
    BOOST_FOREACH(int uid, cVars) {
        if (!d->plotCVar(uid))
            ok = false;
    }

    d->dotStream.close();
    return ok;
}

bool SymHeapPlotter::plotHeapValue(const std::string &name, int value) {
    (void) name;
    (void) value;
    TRAP;
    return false;
}

bool SymHeapPlotter::plotStackFrame(const std::string           &name,
                                    const CodeStorage::Fnc      &fnc)
{
    using namespace CodeStorage;
    if (!createDotFile(d->dotStream, name))
        return false;

    d->lw = &fnc.def.loc;
    CL_DEBUG_MSG(d->lw, "XXX plotting stack frame of " << nameOf(fnc) << "():");

    bool ok = true;
    BOOST_FOREACH(const Var &var, fnc.vars) {
        if (!d->plotCVar(var.uid))
            ok = false;
    }

    d->dotStream.close();
    return ok;
}
