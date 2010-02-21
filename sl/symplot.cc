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
struct SymHeapPlotter::Private {
    const CodeStorage::Storage          *stor;
    const SymbolicHeap::SymHeap         *heap;
    std::ofstream                       dotStream;
    bool                                ok;
    LocationWriter                      lw;
    WorkList<int /* value */>           workList;
    int                                 last;

    bool openDotFile(const std::string &name);
    void closeDotFile();

    void plotNodeObj(int obj, int cVar, enum cl_type_e code);
    void plotNodeValue(int val, enum cl_type_e code, const char *label);
    void plotNodeAux(int src, enum cl_type_e code, const char *label);

    void plotEdgePointsTo(int value, int obj);
    void plotEdgeValueOf(int obj, int value);
    void plotEdgeSub(int obj, int sub);

    void plotSingleValue(int value);
    void plotSingleObj(int obj);
    void plotZeroValue(int obj);

    bool handleCustomValue(int value);
    bool handleUnknownValue(int value, int obj);
    bool resolveValueOf(int /* value */ *pDst, int obj);
    bool resolvePointsTo(int /* obj */ *pDst, int val);

    void digObj(int obj);
    void digValues();
    void plotObj(int obj);
    void plotCVar(int uid);
};

#define SL_QUOTE(what) "\"" << what << "\""

bool SymHeapPlotter::Private::openDotFile(const std::string &plotName)
{
    // compute a sort of unique file name
    PlotEnumerator *pe = PlotEnumerator::instance();
    std::string name(pe->decorate(plotName));
    std::string fileName(name + ".dot");

    // now please create the file
    this->dotStream.open(fileName.c_str(), std::ios::out);
    if (!this->dotStream) {
        CL_ERROR("unable to create file '" << fileName << "'");
        return false;
    }

    // open graph
    this->dotStream << "digraph " << SL_QUOTE(name) << " {" << std::endl
        << "\tlabel=<<FONT POINT-SIZE=\"18\">"
        << name << "</FONT>>;" << std::endl
        << "\tlabelloc=t;" << std::endl;

    CL_DEBUG("symplot: created dot file '" << fileName << "'");
    return this->dotStream;
}

void SymHeapPlotter::Private::closeDotFile() {
    // close graph
    this->dotStream << "}" << std::endl;
    if (!this->dotStream)
        this->ok = false;

    // close stream
    this->dotStream.close();
}

namespace {
    const char* prefixByCode(enum cl_type_e code) {
        switch (code) {
            case CL_TYPE_VOID:      return "void";
            case CL_TYPE_UNKNOWN:   return "?";
            case CL_TYPE_PTR:       return "*";
            case CL_TYPE_FNC:       return "(*)()";
            case CL_TYPE_STRUCT:    return "struct";
            case CL_TYPE_UNION:     return "union";
            case CL_TYPE_ARRAY:     return "array";
            case CL_TYPE_STRING:    return "string";
            case CL_TYPE_CHAR:      return "char";
            case CL_TYPE_BOOL:      return "bool";
            case CL_TYPE_INT:       return "int";
            case CL_TYPE_ENUM:      return "enum";
            default:                return "XXX";
        }
    }

    const char* colorByCode(enum cl_type_e code) {
        switch (code) {
            case CL_TYPE_VOID:      return "red";
            case CL_TYPE_UNKNOWN:   return "gray";
            case CL_TYPE_PTR:       return "blue";
            case CL_TYPE_FNC:       return "green";
            case CL_TYPE_STRUCT:    return "black";
            case CL_TYPE_UNION:     return "gray";
            case CL_TYPE_ARRAY:     return "gray";
            case CL_TYPE_STRING:    return "gray";
            case CL_TYPE_CHAR:      return "gray";
            case CL_TYPE_BOOL:      return "yellow";
            case CL_TYPE_INT:       return "gray";
            case CL_TYPE_ENUM:      return "gray";
            default:                return "black";
        }
    }
}

void SymHeapPlotter::Private::plotNodeObj(int obj, int cVar,
                                          enum cl_type_e code)
{
    this->dotStream << "\t" << SL_QUOTE(obj)
        << " [shape=box"
        << ", color=" << colorByCode(code);

    if (-1 == cVar) {
        this->dotStream
            << ", fontcolor=black"
            << ", label=\"[" << prefixByCode(code) << "] #" << obj
            << "\"];" << std::endl;

        return;
    }

    const CodeStorage::Var &var = varById(*this->stor, cVar);
    // this->lw = &var.loc;
    this->dotStream
        << ", fontcolor=blue"
        << ", label=\"[" << prefixByCode(code) << "] #" << cVar;

    const std::string &name = var.name;
    if (!name.empty())
        this->dotStream << " - " << name;

    this->dotStream << "\"];" << std::endl;
}

void SymHeapPlotter::Private::plotNodeValue(int val, enum cl_type_e code,
                                            const char *label)
{
    this->dotStream << "\t" << SL_QUOTE(val)
        << " [shape=ellipse"
        << ", color=" << colorByCode(code)
        << ", fontcolor=green"
        << ", label=\"[" << prefixByCode(code) << "] #" << val;

    if (label)
        this->dotStream << " [" << label << "]";

    this->dotStream << "\"];" << std::endl;
}

void SymHeapPlotter::Private::plotNodeAux(int src, enum cl_type_e code,
                                          const char *label)
{
    const int id = ++(this->last);
    this->dotStream << "\t"
        << SL_QUOTE("lonely" << id)
        << " [shape=plaintext"
        << ", fontcolor=" << colorByCode(code)
        << ", label=" << SL_QUOTE(label) << "];"
        << std::endl;

    this->dotStream << "\t"
        << SL_QUOTE(src) << " -> " << SL_QUOTE("lonely" << id)
        << " [color=" << colorByCode(code)
        << "];" << std::endl;
}

void SymHeapPlotter::Private::plotEdgePointsTo(int value, int obj) {
    this->dotStream << "\t" << SL_QUOTE(value) << " -> " << SL_QUOTE(obj)
        << " [color=green, fontcolor=green, label=\"pointsTo\"];"
        << std::endl;
}

void SymHeapPlotter::Private::plotEdgeValueOf(int obj, int value) {
    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(value)
        << " [color=blue, fontcolor=blue, label=\"hasValue\"];"
        << std::endl;
}

void SymHeapPlotter::Private::plotEdgeSub(int obj, int sub) {
    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(sub)
        << " [color=gray, style=dotted, arrowhead=open"
        << ", fontcolor=gray, label=\"subVar\"];"
        << std::endl;
}

void SymHeapPlotter::Private::plotSingleValue(int value) {
    if (SymbolicHeap::VAL_NULL == value)
        TRAP;

    if (value < 0)
        TRAP;

    const struct cl_type *clt = this->heap->valType(value);
    if (!clt)
        TRAP;

    const enum cl_type_e code = clt->code;
    this->plotNodeValue(value, code, 0);
}

void SymHeapPlotter::Private::plotSingleObj(int obj) {
    if (obj <= 0)
        TRAP;

    const struct cl_type *clt = this->heap->objType(obj);
    if (!clt)
        TRAP;

    const int cVar = this->heap->cVar(obj);
    const enum cl_type_e code = clt->code;
    this->plotNodeObj(obj, cVar, code);
}

void SymHeapPlotter::Private::plotZeroValue(int obj)
{
    const struct cl_type *clt = this->heap->objType(obj);
    if (!clt)
        TRAP;

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_INT:
            this->plotNodeAux(obj, code, "[int] 0");
            break;

        case CL_TYPE_PTR:
            this->plotNodeAux(obj, code, "NULL");
            break;

        case CL_TYPE_BOOL:
            this->plotNodeAux(obj, code, "FALSE");
            break;

        default:
            TRAP;
    }
}

bool SymHeapPlotter::Private::handleCustomValue(int value) {
    using namespace CodeStorage;

    const struct cl_type *clt;
    const int cVal = this->heap->valGetCustom(&clt, value);
    if (-1 == cVal)
        return false;

    if (!clt || clt->code != CL_TYPE_PTR)
        TRAP;

    clt = clt->items[0].type;
    if (!clt || clt->code != CL_TYPE_FNC)
        TRAP;

    // FIXME: get rid of the const_cast
    Storage &storage = const_cast<Storage &>(*this->stor);
    const Fnc *fnc = storage.anyFncById[cVal];
    if (!fnc)
        TRAP;

    const char *fncName = nameOf(*fnc);
    if (!fncName)
        // anonymous function?
        TRAP;

    std::string name(fncName);
    name += "()";

    this->plotNodeValue(value, CL_TYPE_FNC, 0);
    this->plotNodeAux(value, CL_TYPE_FNC, name.c_str());
    return true;
}

bool SymHeapPlotter::Private::handleUnknownValue(int value, int obj) {
    using namespace SymbolicHeap;

    const EUnknownValue code = this->heap->valGetUnknown(value);
    switch (code) {
        case UV_KNOWN:
            return false;

        case UV_DEREF_FAILED:
            this->plotNodeAux(obj, CL_TYPE_VOID, "DEREF_FAILED");
            return true;

        case UV_UNINITIALIZED:
            this->plotNodeAux(obj, CL_TYPE_UNKNOWN, "UNDEF");
            return true;

        case UV_UNKNOWN:
            this->plotNodeAux(obj, CL_TYPE_UNKNOWN, "?");
            return true;

        default:
            TRAP;
            return true;
    }
}

bool SymHeapPlotter::Private::resolveValueOf(int *pDst, int obj) {
    using namespace SymbolicHeap;
    if (obj < 0)
        TRAP;

    const int value = this->heap->valueOf(obj);
    switch (value) {
        case VAL_INVALID:
            TRAP;
            return false;

        case VAL_NULL /* = VAL_FALSE*/:
            this->plotZeroValue(obj);
            return false;

        case VAL_TRUE:
            this->plotNodeAux(obj, CL_TYPE_BOOL, "TRUE");
            return false;

        default:
            break;
    }

    if (this->handleUnknownValue(value, obj))
        return false;

    if (this->handleCustomValue(value))
        return false;

    *pDst = value;
    return true;
}

bool SymHeapPlotter::Private::resolvePointsTo(int /* obj */ *pDst, int value) {
    using namespace SymbolicHeap;

    const int obj = this->heap->pointsTo(value);
    switch (obj) {
        case OBJ_INVALID:
            this->plotNodeAux(value, CL_TYPE_VOID, "INVALID");
            return false;

        case OBJ_DEREF_FAILED:
            this->plotNodeAux(value, CL_TYPE_VOID, "DEREF_FAILED");
            return false;

        case OBJ_DELETED:
            this->plotNodeAux(value, CL_TYPE_VOID, "DELETED");
            return false;

        case OBJ_LOST:
            this->plotNodeAux(value, CL_TYPE_VOID, "LOST");
            return false;

        case OBJ_UNKNOWN:
            this->plotNodeAux(value, CL_TYPE_UNKNOWN, "?");
            return false;

        case OBJ_RETURN:
        default:
            *pDst = obj;
            return true;
    }
}

void SymHeapPlotter::Private::digObj(int obj) {
    std::stack<int /* obj */> todo;
    todo.push(obj);
    while (!todo.empty()) {
        obj = todo.top();
        todo.pop();

        const struct cl_type *clt = this->heap->objType(obj);
        if (!clt)
            TRAP;

        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_PTR: {
                int value;
                if (this->resolveValueOf(&value, obj)) {
                    this->plotSingleObj(obj);
                    this->plotEdgeValueOf(obj, value);
                    this->workList.schedule(value);
                }
                break;
            }

            case CL_TYPE_STRUCT:
                // TODO: draw subgraph
                this->plotSingleObj(obj);
                for (int i = 0; i < clt->item_cnt; ++i) {
                    const int sub = this->heap->subVar(obj, i);
                    this->plotEdgeSub(obj, sub);
                    todo.push(sub);
                }
                break;

            default:
                TRAP;
        }
    }
}

void SymHeapPlotter::Private::digValues() {
    int value;
    while (workList.next(value)) {
        if (value <= 0)
            // bare value can't be followed
            TRAP;

        // plot the value itself
        this->plotSingleValue(value);

        int obj = this->heap->valGetCompositeObj(value);
        if (SymbolicHeap::OBJ_INVALID != obj) {
            // dig composite object and eventually schedule the values inside
            this->digObj(obj);
            continue;
        }

        // check the value inside
        if (!this->resolvePointsTo(&obj, value))
            // bare value can't be followed
            continue;

        // plot the pointing object and the corresponding "valueOf" edge
        this->plotSingleObj(obj);
        this->plotEdgePointsTo(value, obj);

        // follow values inside the object
        this->digObj(obj);
    }
}

// called only from plotCVar() for now
void SymHeapPlotter::Private::plotObj(int obj) {
    // plot the variable itself
    this->plotSingleObj(obj);

    // look for the value inside
    int value;
    if (!this->resolveValueOf(&value, obj))
        // we got a bare value, which can't be followed, so we're done
        return;

    // connect the variable node with its value
    // FIXME: the edge may appear more times if the variable is referenced
    this->plotEdgeValueOf(obj, value);

    // dig the target value recursively and plot (if not already)
    this->workList.schedule(value);
    this->digValues();
}

void SymHeapPlotter::Private::plotCVar(int uid) {
    // CodeStorage variable lookup
    const CodeStorage::Var &var = varById(*this->stor, uid);
    this->lw = &var.loc;
    CL_DEBUG_MSG(this->lw, "XXX plotting stack variable: #" << var.uid
            << " (" << var.name << ")" );

    // SymbolicHeap variable lookup
    const int obj = this->heap->varByCVar(uid);
    if (SymbolicHeap::OBJ_INVALID == obj)
        CL_DEBUG_MSG(this->lw, "varByCVar lookup failed");

    // plot as regular heap object
    this->plotObj(obj);
}

SymHeapPlotter::SymHeapPlotter(const CodeStorage::Storage   &stor,
                               const SymbolicHeap::SymHeap  &heap):
    d(new Private)
{
    d->stor = &stor;
    d->heap = &heap;
    d->last = 0;
}

SymHeapPlotter::~SymHeapPlotter() {
    delete d;
}

bool SymHeapPlotter::plot(const std::string &name) {
    // create dot file
    d->ok = true;
    if (!d->openDotFile(name))
        return false;

    // go through all stack variables
    SymbolicHeap::SymHeap::TCont cVars;
    d->heap->gatherCVars(cVars);
    BOOST_FOREACH(int uid, cVars) {
        d->plotCVar(uid);
    }

    // close dot file
    d->closeDotFile();
    return d->ok;
}

bool SymHeapPlotter::plotHeapValue(const std::string &name, int value) {
    // create dot file
    d->ok = true;
    if (!d->openDotFile(name))
        return false;

    // plot by value
    d->workList.schedule(value);
    d->digValues();

    // close dot file
    d->closeDotFile();
    return d->ok;
}

bool SymHeapPlotter::plotStackFrame(const std::string           &name,
                                    const CodeStorage::Fnc      &fnc)
{
    using namespace CodeStorage;

    // create dot file
    d->ok = true;
    if (!d->openDotFile(name))
        return false;

    d->lw = &fnc.def.loc;
    CL_DEBUG_MSG(d->lw, "XXX plotting stack frame of " << nameOf(fnc) << "():");

    // go through all stack variables
    BOOST_FOREACH(const Var &var, fnc.vars) {
        d->plotCVar(var.uid);
    }

    // close dot file
    d->closeDotFile();
    return d->ok;
}
