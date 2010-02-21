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

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

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

    // merge name with id
    name += "-";
    name += str.str();
    return name;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapPlotter
struct SymHeapPlotter::Private {
    const CodeStorage::Storage  *stor;
    std::ofstream               dotStream;
};

SymHeapPlotter::SymHeapPlotter(const CodeStorage::Storage &stor):
    d(new Private)
{
    d->stor = &stor;
}

SymHeapPlotter::~SymHeapPlotter() {
    delete d;
}

namespace {
    bool createDotFile(std::ofstream &str, const std::string &plotName) {
        PlotEnumerator *pe = PlotEnumerator::instance();
        std::string fileName(pe->decorate(plotName));
        fileName += ".dot";

        str.open(fileName.c_str(), std::ios::out);
        if (!str) {
            CL_ERROR("unable to create file '" << fileName << "'");
            return false;
        }

        CL_DEBUG("symplot: created dot file '" << fileName << "'");
        return true;
    }
}

bool SymHeapPlotter::plotStackFrame(const std::string &name,
                                    const SymbolicHeap::SymHeap &heap,
                                    const CodeStorage::Fnc *fnc)
{
    if (!createDotFile(d->dotStream, name))
        return false;

    (void) heap;
    (void) fnc;
    TRAP;
    return true;
}
