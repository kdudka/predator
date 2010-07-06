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
#include "symplot.hh"

#include <cl/cl_msg.hh>
#include <cl/location.hh>
#include <cl/storage.hh>

#include "symbt.hh"
#include "symheap.hh"
#include "util.hh"
#include "worklist.hh"

#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifndef DEBUG_SYMPLOT
#   define DEBUG_SYMPLOT 0
#endif

#ifndef SYMPLOT_STOP_AFTER_N_STATES
#   define SYMPLOT_STOP_AFTER_N_STATES 0
#endif

// singleton
class PlotEnumerator {
    public:
        static PlotEnumerator* instance() {
            return (inst_)
                ? (inst_)
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
#if SYMPLOT_STOP_AFTER_N_STATES
    if (SYMPLOT_STOP_AFTER_N_STATES < id) {
        CL_ERROR("SYMPLOT_STOP_AFTER_N_STATES (" << SYMPLOT_STOP_AFTER_N_STATES
                << ") exceeded, now stopping per user's request...");
        TRAP;
    }
#endif

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
    const SymHeap                       *heap;
    std::ofstream                       dotStream;
    bool                                ok;
    LocationWriter                      lw;
    WorkList<TValueId>                  workList;
    std::set<TObjId>                    objDone;
    int                                 last;

    typedef std::pair<TObjId, TValueId> TEdgeValueOf;
    std::vector<TEdgeValueOf>           evList;

    typedef std::pair<TValueId, TValueId> TEdgeNeq;
    std::set<TEdgeNeq>                  neqSet;

    std::set<TObjId>                    binders;
    std::set<TObjId>                    peers;

    bool openDotFile(const std::string &name);
    void closeDotFile();

    bool digFieldName(std::string &dst, TObjId obj);
    void plotNodeObj(TObjId obj, enum cl_type_e code);
    void plotNodeValue(TValueId val, enum cl_type_e code, const char *label);
    void plotNodeAux(int src, enum cl_type_e code, const char *label);

    void plotEdgePointsTo(TValueId value, TObjId obj);
    void plotEdgeValueOf(TObjId obj, TValueId value);
    void plotEdgeNeq(TValueId val1, TValueId val2);
    void plotEdgeSub(TObjId obj, TObjId sub);

    void gobbleEdgeValueOf(TObjId obj, TValueId value);
    void gobbleEdgeNeq(TValueId val1, TValueId val2);
    void emitPendingEdges();

    void plotSingleValue(TValueId value);
    void plotSingleObj(TObjId obj);
    void plotZeroValue(TObjId obj);
    void digBinder(TObjId obj);
    void openCluster(TObjId obj);

    bool handleCustomValue(TValueId value);
    bool handleUnknownValue(TValueId value);
    bool resolveValueOf(TValueId *pDst, TObjId obj);
    bool resolvePointsTo(TObjId *pDst, TValueId val);

    void digObj(TObjId obj);
    void digValues();
    void plotObj(TObjId obj);
    void plotCVar(CVar cVar);
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
    // emit pending edges
    this->emitPendingEdges();

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
            case CL_TYPE_FNC:       return "fnc*";
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

bool SymHeapPlotter::Private::digFieldName(std::string &dst, TObjId obj) {
    const TObjId parent = this->heap->objParent(obj);
    if (OBJ_INVALID == parent)
        // no chance since there is no parent
        return false;

    const struct cl_type *clt = this->heap->objType(parent);
    if (!clt || clt->code != CL_TYPE_STRUCT)
        // type info problem
        TRAP;

    // dig field name
    for (int i = 0; i < clt->item_cnt; ++i) {
        const TObjId sub = this->heap->subObj(parent, i);
        if (obj == sub) {
            dst = clt->items[i].name;
            return true;
        }
    }

    // not found?
    TRAP;
    return false;
}

void SymHeapPlotter::Private::plotNodeObj(TObjId obj, enum cl_type_e code) {
    this->dotStream << "\t" << SL_QUOTE(obj);
    this->dotStream << " [shape=box";

    if (hasKey(this->peers, obj))
        this->dotStream << ", color=yellow, penwidth=3.0, style=dashed";
    else if (hasKey(this->binders, obj))
        this->dotStream << ", color=red, penwidth=3.0, style=dashed";
    else
        this->dotStream << ", color=" << colorByCode(code);

    // dig root object
    TObjId root = obj, next;
    while (OBJ_INVALID != (next = this->heap->objParent(root)))
        root = next;

    if (this->heap->cVar(0, root))
        // colorize on-stack object
        this->dotStream << ", fontcolor=blue";

    else
        // colorize heap (sub)object
        this->dotStream << ", fontcolor=red";

    this->dotStream << ", label=\"[" << prefixByCode(code) << "] #";

    CVar cVar;
    if (this->heap->cVar(&cVar, obj)) {
        this->dotStream << cVar.uid;
        const CodeStorage::Var &var = this->stor->vars[cVar.uid];
        std::string name = var.name;
        if (!name.empty())
            this->dotStream << " - " << name;
    }
    else {
        this->dotStream << obj;
    }

    std::string filedName;
    if (digFieldName(filedName, obj))
        this->dotStream << " ." << filedName;

    this->dotStream << "\"];" << std::endl;
}

void SymHeapPlotter::Private::plotNodeValue(TValueId val, enum cl_type_e code,
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

void SymHeapPlotter::Private::plotEdgePointsTo(TValueId value, TObjId obj) {
    this->dotStream << "\t" << SL_QUOTE(value) << " -> " << SL_QUOTE(obj)
        << " [color=green, fontcolor=green, label=\"pointsTo\"];"
        << std::endl;
}

void SymHeapPlotter::Private::plotEdgeValueOf(TObjId obj, TValueId value) {
    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(value)
        << " [color=blue, fontcolor=blue, label=\"hasValue\"];"
        << std::endl;
}

void SymHeapPlotter::Private::plotEdgeNeq(TValueId val1, TValueId val2) {
    this->dotStream << "\t" << SL_QUOTE(val1) << " -> " << SL_QUOTE(val2)
        << " [color=yellow, fontcolor=red, label=\"Neq\", arrowhead=none];"
        << std::endl;
}

void SymHeapPlotter::Private::plotEdgeSub(TObjId obj, TObjId sub) {
    this->dotStream << "\t" << SL_QUOTE(obj) << " -> " << SL_QUOTE(sub)
        << " [color=gray, style=dotted, arrowhead=open"
        << ", fontcolor=gray, label=\"field\"];"
        << std::endl;
}

void SymHeapPlotter::Private::gobbleEdgeValueOf(TObjId obj, TValueId value) {
    TEdgeValueOf edge(obj, value);
    this->evList.push_back(edge);
}

void SymHeapPlotter::Private::gobbleEdgeNeq(TValueId val1, TValueId val2) {
    // Neq predicates induce a symmetric relation, let's handle them such
    sortValues(val1, val2);

    TEdgeNeq edge(val1, val2);
    this->neqSet.insert(edge);
}

void SymHeapPlotter::Private::emitPendingEdges() {
    // plot all valueOf edges
    BOOST_FOREACH(const TEdgeValueOf &edge, this->evList) {
        this->plotEdgeValueOf(edge.first, edge.second);
    }

    // plot all Neq edges
    BOOST_FOREACH(const TEdgeNeq &edge, this->neqSet) {
        this->plotEdgeNeq(edge.first, edge.second);
    }

    // cleanup for next wheel
    this->evList.clear();
    this->neqSet.clear();
}

void SymHeapPlotter::Private::plotSingleValue(TValueId value) {
    if (value <= 0) {
        this->plotNodeValue(value, CL_TYPE_UNKNOWN, 0);
        return;
    }

    // traverse all Neq/EqIf predicates
    SymHeap::TContValue relatedVals;
    this->heap->gatherRelatedValues(relatedVals, value);
    BOOST_FOREACH(TValueId peer, relatedVals) {
        if (0 < peer)
            this->workList.schedule(peer);

        bool eq;
        if (!this->heap->proveEq(&eq, value, peer))
            goto unhandled_pred;

        if (!eq) {
            if (VAL_NULL == peer) {
                // 'value' is said to be non-zero
                this->plotNodeAux(value, CL_TYPE_BOOL, "non-zero");
                continue;
            }
            else if (VAL_NULL < peer) {
                // regular Neq predicate
                this->gobbleEdgeNeq(value, peer);
                continue;
            }
        }

unhandled_pred:
        CL_WARN("SymHeapPlotter: unhandled predicate over values #"
                << value << " and #" << peer);
    }

    const struct cl_type *clt = this->heap->valType(value);
    const enum cl_type_e code = (clt)
        ? clt->code
        : CL_TYPE_UNKNOWN;

    this->plotNodeValue(value, code, 0);
}

void SymHeapPlotter::Private::plotSingleObj(TObjId obj) {
    if (obj <= 0)
        TRAP;

    const struct cl_type *clt = this->heap->objType(obj);
    if (!clt)
        TRAP;

    this->plotNodeObj(obj, clt->code);
}

void SymHeapPlotter::Private::plotZeroValue(TObjId obj)
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

void SymHeapPlotter::Private::digBinder(TObjId obj) {
    EObjKind kind = this->heap->objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
            return;

        case OK_SLS:
        case OK_DLS:
            break;
    }

    const TFieldIdxChain icBind = this->heap->objBinderField(obj);
    const TObjId objBind = subObjByChain(*this->heap, obj, icBind);
    if (objBind <= 0)
        TRAP;

    // store icBind
    this->binders.insert(objBind);
    if (OK_SLS == kind)
        return;

    const TFieldIdxChain icPeer = this->heap->objPeerField(obj);
    const TObjId objPeer = subObjByChain(*this->heap, obj, icPeer);
    if (objBind <= 0)
        TRAP;

    // store icPeer
    this->peers.insert(objPeer);
}

void SymHeapPlotter::Private::openCluster(TObjId obj) {
    const char *label, *color, *pw;
    EObjKind kind = this->heap->objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
            label = "";
            color = "black";
            pw = "1.0";
            break;

        case OK_SLS:
            label = "SLS";
            color = "red";
            pw = "3.0";
            break;

        case OK_DLS:
            label = "DLS/2";
            color = "yellow";
            pw = "3.0";
            break;
    }
    this->dotStream
        << "subgraph \"cluster" << obj << "\" {"    << std::endl
        << "\tlabel=" << SL_QUOTE(label) << ";"     << std::endl
        << "\tcolor=" << color << ";"               << std::endl
        << "\tfontcolor=" << color << ";"           << std::endl
        << "\tbgcolor=gray98;"                      << std::endl
        << "\tstyle=dashed;"                        << std::endl
        << "\tpenwidth=" << pw << ";"               << std::endl;
}

bool SymHeapPlotter::Private::handleCustomValue(TValueId value) {
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
    const Fnc *fnc = storage.fncs[cVal];
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

bool SymHeapPlotter::Private::handleUnknownValue(TValueId value) {
    const EUnknownValue code = this->heap->valGetUnknown(value);
    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            return false;

        case UV_DEREF_FAILED:
            this->plotNodeAux(value, CL_TYPE_VOID, "UV_DEREF_FAILED");
            return true;

        case UV_UNINITIALIZED:
            this->plotNodeAux(value, CL_TYPE_UNKNOWN, "UV_UNINITIALIZED");
            return true;

        case UV_UNKNOWN:
            this->plotNodeAux(value, CL_TYPE_UNKNOWN, "UV_UNKNOWN");
            return true;

        default:
            TRAP;
            return true;
    }
}

bool SymHeapPlotter::Private::resolveValueOf(TValueId *pDst, TObjId obj) {
    if (obj < 0)
        TRAP;

    // avoid duplicates
    if (hasKey(this->objDone, obj))
        return false;
    this->objDone.insert(obj);

    const TValueId value = this->heap->valueOf(obj);
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

    if (this->handleCustomValue(value))
        return false;

    *pDst = value;
    return true;
}

bool SymHeapPlotter::Private::resolvePointsTo(TObjId *pDst, TValueId value) {
    if (this->handleUnknownValue(value))
        return false;

    const TObjId obj = this->heap->pointsTo(value);
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

void SymHeapPlotter::Private::digObj(TObjId obj) {
    typedef std::pair<TObjId, bool /* last */> TStackItem;
    std::stack<TStackItem> todo;
    push(todo, obj, false);
    while (!todo.empty()) {
        bool last;
        boost::tie(obj, last) = todo.top();
        todo.pop();

        const struct cl_type *clt = this->heap->objType(obj);
        if (!clt)
            TRAP;

        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_INT:
            case CL_TYPE_PTR: {
                this->plotSingleObj(obj);
                TValueId value;
                if (this->resolveValueOf(&value, obj)) {
                    this->gobbleEdgeValueOf(obj, value);
                    this->workList.schedule(value);
                }
                break;
            }

            case CL_TYPE_STRUCT:
                this->digBinder(obj);
                this->openCluster(obj);
                this->plotSingleObj(obj);
                for (int i = 0; i < clt->item_cnt; ++i) {
                    const TObjId sub = this->heap->subObj(obj, i);
                    if (!hasKey(this->objDone, sub))
                        this->plotEdgeSub(obj, sub);

                    push(todo, sub, /* last */ (0 == i));
                }
                break;

            default:
                CL_DEBUG_MSG(this->lw, "SymHeapPlotter::Private::digObj("<<obj<<"): Unimplemented type: " << code );
                TRAP;
        }

        if (last)
            // we are done with the current cluster, close it now
            this->dotStream << "}" << std::endl;
    }
}

void SymHeapPlotter::Private::digValues() {
    TValueId value;
    while (workList.next(value)) {
        // plot the value itself
        this->plotSingleValue(value);

        if (value <= 0)
            // bare value can't be followed
            continue;

        TObjId obj = this->heap->valGetCompositeObj(value);
        if (OBJ_INVALID != obj) {
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
void SymHeapPlotter::Private::plotObj(TObjId obj) {
    // plot the variable itself
    this->plotSingleObj(obj);

    // look for the value inside
    TValueId value;
    if (!this->resolveValueOf(&value, obj))
        // we got a bare value, which can't be followed, so we're done
        return;

    if (OBJ_INVALID != this->heap->valGetCompositeObj(value)) {
        // dig composite object and eventually schedule the values inside
        this->digObj(obj);
        goto wl_ready;
    }

    // connect the variable node with its value
    this->plotEdgeValueOf(obj, value);

    // dig the target value recursively and plot (if not already)
    this->workList.schedule(value);

wl_ready:
    this->digValues();
}

void SymHeapPlotter::Private::plotCVar(CVar cVar) {
    // CodeStorage variable lookup
    const CodeStorage::Var &var = this->stor->vars[cVar.uid];
    this->lw = &var.loc;
#if DEBUG_SYMPLOT
    CL_DEBUG_MSG(this->lw, "XXX plotting stack variable: #" << var.uid
            << " (" << var.name << ")" );
#endif

    // SymbolicHeap variable lookup
    const TObjId obj = this->heap->objByCVar(cVar);
    if (OBJ_INVALID == obj)
        CL_DEBUG_MSG(this->lw, "objByCVar lookup failed");

    // plot as regular heap object
    this->plotObj(obj);
}

SymHeapPlotter::SymHeapPlotter(const CodeStorage::Storage   &stor,
                               const SymHeap                &heap):
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
    SymHeap::TContCVar cVars;
    d->heap->gatherCVars(cVars);
    BOOST_FOREACH(CVar cv, cVars) {
        d->plotCVar(cv);
    }

    // close dot file
    d->closeDotFile();
    return d->ok;
}

bool SymHeapPlotter::plotHeapValue(const std::string &name, TValueId value) {
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
                                    const CodeStorage::Fnc      &fnc,
                                    const SymBackTrace          *bt)
{
    using namespace CodeStorage;

    // create dot file
    d->ok = true;
    if (!d->openDotFile(name))
        return false;

    d->lw = &fnc.def.loc;
#if DEBUG_SYMPLOT
    CL_DEBUG_MSG(d->lw, "XXX plotting stack frame of " << nameOf(fnc) << "():");
#endif

    // go through all stack variables
    BOOST_FOREACH(const int uid, fnc.vars) {
        const int nestLevel = bt->countOccurrencesOfFnc(uidOf(fnc));
        const CVar cVar(uid, nestLevel);
        d->plotCVar(cVar);
    }

    // close dot file
    d->closeDotFile();
    return d->ok;
}

// vim: tw=80
