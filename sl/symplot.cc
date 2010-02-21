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
#define SL_QUOTE(what) \
    "\"" << what << "\""

#define SL_GRAPH(name) \
    "digraph " << SL_QUOTE(name) << " {" << std::endl \
    << "\tlabel=<<FONT POINT-SIZE=\"18\">" << name << "</FONT>>;" << std::endl \
    << "\tlabelloc=t;" << std::endl

struct SymHeapPlotter::Private {
    const CodeStorage::Storage      *stor;
    const SymbolicHeap::SymHeap     *heap;
    std::ofstream                   dotStream;
    LocationWriter                  lw;

    bool openDotFile(const std::string &name);
    bool closeDotFile();

    void plotNode(int id, const char *shape, const char *color,
                  const char *label);

    bool plotValue(int value);
    bool plotObj(int obj);
    bool plotCVar(int uid);
};

bool SymHeapPlotter::Private::openDotFile(const std::string &plotName)
{
    // compute a sort of unique file name
    PlotEnumerator *pe = PlotEnumerator::instance();
    std::string fileName(pe->decorate(plotName));
    fileName += ".dot";

    // now please create the file
    this->dotStream.open(fileName.c_str(), std::ios::out);
    if (!this->dotStream) {
        CL_ERROR("unable to create file '" << fileName << "'");
        return false;
    }

    CL_DEBUG("symplot: created dot file '" << fileName << "'");
    this->dotStream << SL_GRAPH(plotName);
    return this->dotStream;
}

bool SymHeapPlotter::Private::closeDotFile() {
    // close graph
    this->dotStream << "}" << std::endl;

    // close stream
    const bool ok = this->dotStream;
    this->dotStream.close();
    return ok;
}

void SymHeapPlotter::Private::plotNode(int id, const char *shape,
                                       const char *color, const char *label)
{
    this->dotStream << "\t" << SL_QUOTE(id)
        << " [shape=" << shape
        << ", color=" << color
        << ", label=" << SL_QUOTE("#" << id << " [" << label << "]") << "];"
        << std::endl;
}

bool SymHeapPlotter::Private::plotValue(int value) {
    if (value <= 0)
        // TODO
        TRAP;

    const struct cl_type *clt = this->heap->valType(value);
    if (!clt)
        // TODO
        TRAP;

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_PTR:
            this->plotNode(value, "circle", "blue", "PTR");
            break;

        case CL_TYPE_BOOL:
            this->plotNode(value, "circle", "yellow", "BOOL");
            break;

        case CL_TYPE_INT:
            this->plotNode(value, "circle", "gray", "INT");
            break;

        default:
            TRAP;
    }

    // TODO
    return false;
}

bool SymHeapPlotter::Private::plotObj(int obj) {
    if (obj <= 0)
        // TODO
        TRAP;

    const struct cl_type *clt = this->heap->objType(obj);
    if (!clt)
        // TODO
        TRAP;

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_PTR:
            this->plotNode(obj, "box", "blue", "PTR");
            break;

        case CL_TYPE_BOOL:
            this->plotNode(obj, "box", "yellow", "BOOL");
            break;

        case CL_TYPE_INT:
            this->plotNode(obj, "box", "gray", "INT");
            break;

        default:
            TRAP;
    }

    // TODO
    return false;
}

bool SymHeapPlotter::Private::plotCVar(int uid) {
    using namespace SymbolicHeap;

    const CodeStorage::Var &var = varById(*this->stor, uid);
    this->lw = &var.loc;
    CL_DEBUG_MSG(this->lw, "XXX plotting stack variable: #" << var.uid
            << " (" << var.name << ")" );

    const int obj = this->heap->varByCVar(uid);
    if (OBJ_INVALID == obj)
        CL_DEBUG_MSG(this->lw, "varByCVar lookup failed");

    return this->plotObj(obj);
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

bool SymHeapPlotter::plot(const std::string &name) {
    using namespace SymbolicHeap;

    // create dot file
    if (!d->openDotFile(name))
        return false;

    // go through all stack variables
    bool ok = true;
    SymHeap::TCont cVars;
    d->heap->gatherCVars(cVars);
    BOOST_FOREACH(int uid, cVars) {
        if (!d->plotCVar(uid))
            ok = false;
    }

    // close dot file
    d->closeDotFile();
    return ok;
}

bool SymHeapPlotter::plotHeapValue(const std::string &name, int value) {
    // create dot file
    if (!d->openDotFile(name))
        return false;

    // plot by value
    const bool ok = d->plotValue(value);

    // close dot file
    d->closeDotFile();
    return ok;
}

bool SymHeapPlotter::plotStackFrame(const std::string           &name,
                                    const CodeStorage::Fnc      &fnc)
{
    using namespace CodeStorage;

    // create dot file
    if (!d->openDotFile(name))
        return false;

    d->lw = &fnc.def.loc;
    CL_DEBUG_MSG(d->lw, "XXX plotting stack frame of " << nameOf(fnc) << "():");

    // go through all stack variables
    bool ok = true;
    BOOST_FOREACH(const Var &var, fnc.vars) {
        if (!d->plotCVar(var.uid))
            ok = false;
    }

    // close dot file
    d->closeDotFile();
    return ok;
}
