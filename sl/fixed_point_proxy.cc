/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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
#include "fixed_point_proxy.hh"

#include "cont_shape.hh"
#include "fixed_point.hh"
#include "symplot.hh"
#include "symstate.hh"
#include "symtrace.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/storage.hh>

#include <fstream>
#include <map>

#include <boost/foreach.hpp>

namespace FixedPoint {

typedef const struct cl_loc                        *TLoc;
typedef int                                         TFncUid;
typedef std::map<TFncUid, TFnc>                     TFncMap;

typedef const CodeStorage::Block                   *TBlock;

// TODO: drop this!
typedef StateByInsn::TStateMap                      TStateMap;
bool isTransparentInsn(const TInsn insn);

class TraceIndex {
    public:
        void indexTraceOf(const SymHeap *sh);
        const SymHeap* nearestPredecessorOf(const SymHeap *sh) const;

    private:
        typedef std::map<const Trace::Node *, const SymHeap *> TLookup;
        TLookup lookup_;
};

void TraceIndex::indexTraceOf(const SymHeap *sh)
{
    const Trace::Node *tr = sh->traceNode();

    // we should never change the target heap of an already indexed trace node
    CL_BREAK_IF(hasKey(lookup_, tr) && lookup_[tr] != sh);

    lookup_[tr] = sh;
}

const SymHeap* TraceIndex::nearestPredecessorOf(const SymHeap *sh) const
{
    const Trace::Node *tr = sh->traceNode();

    while (0U < tr->parents().size()) {
        // TODO: handle trace nodes with more than one parent!
        tr = tr->parent();

        // check the current trace node
        const TLookup::const_iterator it = lookup_.find(tr);
        if (it == lookup_.end())
            continue;

        // found!
        const SymHeap *shPred = it->second;
        CL_BREAK_IF(shPred->traceNode() != tr);
        return shPred;
    }

    return /* not found */ 0;
}

struct StateByInsn::Private {
    TFncMap             visitedFncs;
    TStateMap           stateByInsn;
};

StateByInsn::StateByInsn():
    d(new Private)
{
}

StateByInsn::~StateByInsn()
{
    delete d;
}

bool /* any change */ StateByInsn::insert(const TInsn insn, const SymHeap &sh)
{
    SymStateWithJoin &state = d->stateByInsn[insn];

    if (!state.size()) {
        // update the map of visited functions
        const TFnc fnc = fncByCfg(insn->bb->cfg());
        const TFncUid uid = uidOf(*fnc);
        d->visitedFncs[uid] = fnc;
    }

    return state.insert(sh, /* allowThreeWay */ false);
}

const TStateMap& StateByInsn::stateMap() const
{
    return d->stateByInsn;
}

struct PlotData {
    std::ostream       &out;
    TStateMap          &stateByInsn;
    std::string         name;

    PlotData(
            std::ostream           &out_,
            TStateMap              &stateByInsn_,
            const std::string      &name_):
        out(out_),
        stateByInsn(stateByInsn_),
        name(name_)
    {
    }
};

#define QUOT(what) "\"" << what << "\""
#define LOC_NODE(locIdx) QUOT("loc" << locIdx)
#define SHID(sh) QUOT("sh" << sh)
#define DOT_LINK(to) "\"" << to << ".svg\""

void plotInsn(PlotData &plot, const TLocIdx locIdx, const LocalState &locState)
{
    // open cluster
    plot.out << "subgraph \"cluster" << locIdx << "\" {\n\tlabel=\"\"\n";

    // plot the root node
    plot.out << LOC_NODE(locIdx) << " [label=" << QUOT(*locState.insn)
        << ", shape=box, color=blue, fontcolor=blue];\n";

    const SymState &state = locState.heapList;

    // XXX: detect container shapes
    TShapeListByHeapIdx contShapes;
    detectContShapes(&contShapes, state);

    const int cntHeaps = state.size();
    for (int i = 0; i < cntHeaps; ++i) {
        const SymHeap &sh = state[i];
        const TShapeList &shapeList = contShapes[i];

        TIdSet contShapeIds;
        BOOST_FOREACH(const Shape &shape, shapeList) {
            TObjSet contShapeObjs;
            objSetByShape(&contShapeObjs, sh, shape);
            BOOST_FOREACH(const TObjId obj, contShapeObjs)
                contShapeIds.insert(static_cast<int>(obj));
        }

        // plot the shape graph
        std::string shapeName;
        plotHeap(sh, plot.name + "-sh", /* loc */ 0, &shapeName, &contShapeIds);

        // plot the link to shape
        plot.out << SHID(&sh) << " [label=\"sh #" << i
            << "\", URL=" << DOT_LINK(shapeName);

        if (!shapeList.empty())
            plot.out << ", color=red, penwidth=3.0";

        plot.out << "];\n";
    }

    // close cluster
    plot.out << "}\n";
}

void plotFncCore(PlotData &plot, const GlobalState &fncState)
{
    const TLocIdx locCnt = fncState.size();
    for (TLocIdx locIdx = 0; locIdx < locCnt; ++locIdx) {
        const LocalState &locState = fncState[locIdx];
        plotInsn(plot, locIdx, locState);

        const unsigned cntTargets = locState.cfgOutEdges.size();
        for (unsigned i = 0; i < cntTargets; ++i) {
            const CfgEdge &edge = locState.cfgOutEdges[i];

            const char *color = "blue";
            if (edge.closesLoop)
                // loop-closing edge
                color = "green";

            const char *label = "";
            if (2U == cntTargets)
                // assume CL_INSN_COND
                label = (!i) ? "T" : "F";

            plot.out << LOC_NODE(locIdx) << " -> " << LOC_NODE(edge.targetLoc)
                << " [label=" << QUOT(label)
                << ", color=" << color
                << ", fontcolor=" << color << "];\n";
        }
    }
}

void plotFnc(const TFnc fnc, TStateMap &stateByInsn)
{
    const std::string fncName = nameOf(*fnc);
    std::string plotName("fp-");
    plotName += fncName;

    // create a dot file
    const std::string fileName(plotName + ".dot");
    std::fstream out(fileName.c_str(), std::ios::out);
    if (!out) {
        CL_ERROR("unable to create file '" << fileName << "'");
        return;
    }

    // open graph
    out << "digraph " << QUOT(plotName)
        << " {\n\tlabel=<<FONT POINT-SIZE=\"36\">" << fncName
        << "()</FONT>>;\n\tclusterrank=local;\n\tlabelloc=t;\n";

    // plot the body
    PlotData plot(out, stateByInsn, plotName);
    const GlobalState *fncState = computeStateOf(fnc, stateByInsn);
    plotFncCore(plot, *fncState);

    // build trace index
    TraceIndex trIndex;
    for (TLocIdx locIdx = 0; locIdx < fncState->size(); ++locIdx) {
        const SymState &state = (*fncState)[locIdx].heapList;
        BOOST_FOREACH(const SymHeap *sh, state)
            trIndex.indexTraceOf(sh);
    }

    // plot trace edges
    for (TLocIdx locIdx = 0; locIdx < fncState->size(); ++locIdx) {
        const SymState &state = (*fncState)[locIdx].heapList;
        BOOST_FOREACH(const SymHeap *sh, state) {
            const SymHeap *shPred = trIndex.nearestPredecessorOf(sh);
            if (shPred)
                plot.out << SHID(shPred) << " -> " << SHID(sh) << ";\n";
        }
    }

    delete fncState;

    // close graph
    out << "}\n";
    if (!out)
        CL_ERROR("unable to write file '" << fileName << "'");
    out.close();
}

void StateByInsn::plotAll()
{
    BOOST_FOREACH(TFncMap::const_reference fncItem, d->visitedFncs) {
        const TFnc fnc = fncItem.second;
        const TLoc loc = locationOf(*fnc);
        CL_NOTE_MSG(loc, "plotting fixed-point of " << nameOf(*fnc) << "()...");

        plotFnc(fnc, d->stateByInsn);
    }
}

} // namespace FixedPoint
