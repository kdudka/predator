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
#include "worklist.hh"

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stack>
#include <string>

#include <boost/foreach.hpp>
//#include <boost/tuple/tuple.hpp>

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
    const CodeStorage::Storage          *stor;
    const SymbolicHeap::SymHeap         *heap;
    std::ofstream                       dotStream;
    LocationWriter                      lw;
    WorkList<int /* value */>           wl;

    int                                 last;
    int acquireId() {
        return (this->last)++;
    }

    bool openDotFile(const std::string &name);
    bool closeDotFile();

    void plotNode(int id, const char *shape, const char *color,
                  const char *label);
    void plotLonelyNode(int obj, const char *shape, const char *color,
                        const char *text);

    void plotPointsTo(int value, int obj);
    void plotValueOf(int obj, int value);
    void plotCompEdge(int obj, int sub);

    void plotValue(int value);
    void plotObj(int obj);

    bool resolveValueOf(int /* value */ *pDst, int obj);
    bool resolvePointsTo(int /* obj */ *pDst, int val);

    bool handleCustomValue(int value);
    bool handleUnknownValue(int value);

    template <class TWL> void digValueOf(TWL &dst, int obj);

    bool digValue(int value);
    bool digObj(int obj);
    bool digCVar(int uid);
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

void SymHeapPlotter::Private::plotLonelyNode(int obj, const char *shape,
                                             const char *color,
                                             const char *text)
{
    const int id = this->acquireId();
    this->dotStream << "\t" << SL_QUOTE(id)
        << " [shape=" << shape
        << ", color=" << color
        << ", label=" << SL_QUOTE(text) << "];"
        << std::endl;

    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(id)
        << std::endl;
}

void SymHeapPlotter::Private::plotPointsTo(int value, int obj) {
    this->dotStream << "\t" << SL_QUOTE(value) << " -> " << SL_QUOTE(obj)
        << " [color=red];"
        << std::endl;
}

void SymHeapPlotter::Private::plotValueOf(int obj, int value) {
    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(value)
        << " [color=blue];"
        << std::endl;
}

void SymHeapPlotter::Private::plotCompEdge(int obj, int sub) {
    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(sub)
        << " [color=gray, style=dotted, arrowhead=open];"
        << std::endl;
}

void SymHeapPlotter::Private::plotValue(int value) {
    if (SymbolicHeap::VAL_NULL == value)
        TRAP;

    if (value < 0)
        TRAP;

    const struct cl_type *clt = this->heap->valType(value);
    if (!clt)
        TRAP;

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_PTR:
            this->plotNode(value, "ellipse", "blue", "PTR");
            break;

        case CL_TYPE_BOOL:
            this->plotNode(value, "ellipse", "yellow", "BOOL");
            break;

        case CL_TYPE_INT:
            this->plotNode(value, "ellipse", "gray", "INT");
            break;

        case CL_TYPE_STRUCT:
            this->plotNode(value, "circle", "green", "place-holder");
            break;

        default:
            TRAP;
    }
}

void SymHeapPlotter::Private::plotObj(int obj) {
    std::stack<int /* obj */> todo;
    todo.push(obj);
    while (!todo.empty()) {
        obj = todo.top();
        todo.pop();

        if (obj <= 0)
            TRAP;

        const struct cl_type *clt = this->heap->objType(obj);
        if (!clt)
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

            case CL_TYPE_STRUCT:
                // TODO: draw subgraph
                this->plotNode(obj, "box", "gray", "STRUCT");
                for (int i = 0; i < clt->item_cnt; ++i) {
                    const int sub = this->heap->subVar(obj, i);
                    this->plotCompEdge(obj, sub);
                    todo.push(sub);
                }
                break;

            default:
                TRAP;
        }
    }
}

bool SymHeapPlotter::Private::resolveValueOf(int *pDst, int obj) {
    using namespace SymbolicHeap;
    if (obj < 0)
        TRAP;

    const struct cl_type *clt = this->heap->objType(obj);
    if (!clt)
        TRAP;

    const int value = this->heap->valueOf(obj);
    switch (value) {
        case VAL_INVALID:
            TRAP;
            return false;

        case VAL_NULL /* = VAL_FALSE*/:
            switch (clt->code) {
                case CL_TYPE_INT:
                    this->plotLonelyNode(obj, "ellipse", "black", "(int) 0");
                    return false;

                case CL_TYPE_PTR:
                    this->plotLonelyNode(obj, "ellipse", "blue", "NULL");
                    return false;

                case CL_TYPE_BOOL:
                    this->plotLonelyNode(obj, "ellipse", "yellow", "FALSE");
                    return false;

                default:
                    TRAP;
            }
            return false;

        case VAL_TRUE:
            if (CL_TYPE_BOOL != clt->code)
                TRAP;
            this->plotLonelyNode(obj, "circle", "yellow", "TRUE");
            return false;

        default:
            break;
    }

    const EUnknownValue code = this->heap->valGetUnknown(value);
    switch (code) {
        case UV_KNOWN:
            *pDst = value;
            return true;

        case UV_DEREF_FAILED:
            this->plotLonelyNode(obj, "ellipse", "red", "DEREF_FAILED");
            return false;

        case UV_UNINITIALIZED:
            this->plotLonelyNode(obj, "ellipse", "gray", "UNINITIALIZED");
            return false;

        case UV_UNKNOWN:
            this->plotLonelyNode(obj, "circle", "gray", "?");
            return false;

        default:
            TRAP;
            return false;
    }
}

bool SymHeapPlotter::Private::resolvePointsTo(int /* obj */ *pDst, int value) {
    using namespace SymbolicHeap;

    const int obj = this->heap->pointsTo(value);
    switch (obj) {
        case OBJ_INVALID:
            this->plotLonelyNode(obj, "box", "red", "INVALID");
            return false;

        case OBJ_DEREF_FAILED:
            this->plotLonelyNode(obj, "box", "gray", "DEREF_FAILED");
            return false;

        case OBJ_DELETED:
            this->plotLonelyNode(obj, "box", "gray", "DELETED");
            return false;

        case OBJ_LOST:
            this->plotLonelyNode(obj, "box", "gray", "LOST");
            return false;

        case OBJ_UNKNOWN:
            this->plotLonelyNode(obj, "box", "gray", "?");
            return false;

        case OBJ_RETURN:
        default:
            *pDst = obj;
            return true;
    }
}

bool SymHeapPlotter::Private::handleCustomValue(int value) {
    const struct cl_type *clt;
    const int cVal = this->heap->valGetCustom(&clt, value);
    if (-1 == cVal)
        return false;

    CL_ERROR("handleCustomValue not implemented");
    return true;
}

bool SymHeapPlotter::Private::handleUnknownValue(int value) {
    using namespace SymbolicHeap;

    const EUnknownValue code = this->heap->valGetUnknown(value);
    if (UV_KNOWN == code)
        return false;

    CL_ERROR("handleUnknownValue not implemented");
    return true;
}

// FIXME: copy-pasted from plotObj()
template <class TWL>
void SymHeapPlotter::Private::digValueOf(TWL &dst, int obj) {
    std::stack<int /* obj */> todo;
    todo.push(obj);
    while (!todo.empty()) {
        obj = todo.top();
        todo.pop();

        if (obj < 0)
            continue;

        const struct cl_type *clt = this->heap->objType(obj);
        if (!clt)
            continue;

        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_PTR:
            case CL_TYPE_BOOL:
            case CL_TYPE_INT: {
                int value;
                if (this->resolveValueOf(&value, obj))
                    dst.schedule(value);
                break;
            }

            case CL_TYPE_STRUCT:
                // TODO: draw edges
                for (int i = 0; i < clt->item_cnt; ++i)
                    todo.push(this->heap->subVar(obj, i));
                break;

            default:
                TRAP;
        }
    }
}

bool SymHeapPlotter::Private::digValue(int value) {
    using namespace SymbolicHeap;
    bool ok = true;

    wl.schedule(value);
    while (wl.next(value)) {
        if (value <= VAL_NULL)
            TRAP;

        if (this->handleCustomValue(value))
            continue;

        if (this->handleUnknownValue(value))
            continue;

        int obj = this->heap->valGetCompositeObj(value);
        if (OBJ_INVALID != obj) {
            this->digValueOf(wl, obj);
            continue;
        }

        this->plotValue(value);
        if (!this->resolvePointsTo(&obj, value))
            continue;

        this->plotObj(obj);
        this->plotPointsTo(value, obj);
        this->digValueOf(wl, obj);
    }

    return ok;
}

bool SymHeapPlotter::Private::digObj(int obj) {
    this->plotObj(obj);
    if (obj < 0)
        return true;

    int value;
    if (!this->resolveValueOf(&value, obj))
        return true;

    return this->digValue(value);
}

bool SymHeapPlotter::Private::digCVar(int uid) {
    using namespace SymbolicHeap;

    const CodeStorage::Var &var = varById(*this->stor, uid);
    this->lw = &var.loc;
    CL_DEBUG_MSG(this->lw, "XXX plotting stack variable: #" << var.uid
            << " (" << var.name << ")" );

    const int obj = this->heap->varByCVar(uid);
    if (OBJ_INVALID == obj)
        CL_DEBUG_MSG(this->lw, "varByCVar lookup failed");

    return this->digObj(obj);
}

SymHeapPlotter::SymHeapPlotter(const CodeStorage::Storage   &stor,
                               const SymbolicHeap::SymHeap  &heap):
    d(new Private)
{
    d->stor = &stor;
    d->heap = &heap;
    d->last = /* XXX */ 0x10000;
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
        if (!d->digCVar(uid))
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
    const bool ok = d->digValue(value);

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
        if (!d->digCVar(var.uid))
            ok = false;
    }

    // close dot file
    d->closeDotFile();
    return ok;
}
