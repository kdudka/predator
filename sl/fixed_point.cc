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
#include "fixed_point.hh"

#include "cont_shape.hh"
#include "symplot.hh"
#include "symstate.hh"
#include "symtrace.hh"
#include "worklist.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/storage.hh>

#include <fstream>
#include <map>

#include <boost/foreach.hpp>

namespace FixedPoint {

typedef const struct cl_loc                        *TLoc;
typedef const CodeStorage::Fnc                     *TFnc;
typedef int                                         TFncUid;
typedef std::map<TFncUid, TFnc>                     TFncMap;

typedef const CodeStorage::ControlFlow             &TControlFlow;
typedef const CodeStorage::Block                   *TBlock;
typedef WorkList<TBlock>                            TWorkList;

typedef std::vector<TInsn>                          TInsnList;
typedef std::map<TInsn, SymStateWithJoin>           TStateMap;

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
#define INSN(insn) QUOT("ins" << insn)
#define DOT_LINK(to) "\"" << to << ".svg\""

void plotInsn(PlotData &plot, const TInsn insn)
{
    // open cluster
    plot.out << "subgraph \"cluster" << insn << "\" {\n\tlabel=\"\"\n";

    // plot the root node
    plot.out << INSN(insn) << "[label=" << QUOT(*insn) << ", shape=box];\n";

    const SymState &state = plot.stateByInsn[insn];

    // XXX: detect container shapes
    ContShape::TShapeListByHeapIdx contShapes;
    detectContShapes(&contShapes, state);

    const unsigned cntHeaps = state.size();
    for (unsigned i = 0; i < cntHeaps; ++i) {
        const SymHeap &sh = state[i];

        // plot the shape graph
        std::string shapeName;
        plotHeap(sh, plot.name + "-sh", /* loc */ 0, &shapeName);

        // plot the trace graph
        std::string traceName;
        plotTrace(sh.traceNode(), plot.name + "-tr", &traceName);

        // open cluster
        plot.out << "subgraph \"cluster" << shapeName
            << "\" {\n\tlabel=\"#" << i << "\"\n";

        // plot the link to shape
        plot.out << QUOT(shapeName)
            << "[label=\"sh\", URL=" << DOT_LINK(shapeName) << "];\n";

        // plot the link to trace
        plot.out << QUOT(traceName)
            << "[label=\"tr\", URL=" << DOT_LINK(traceName) << "];\n";

        // close cluster
        plot.out << "}\n";
    }

    // close cluster
    plot.out << "}\n";

    // for terminal instructions, plot the outgoing edges
    const unsigned cntTargets = insn->targets.size();
    for (unsigned i = 0; i < cntTargets; ++i) {
        const TBlock bb = insn->targets[i];

        const char *label = "";
        if (CL_INSN_COND == insn->code)
            label = (!i) ? "T" : "F";

        plot.out << INSN(insn) << " -> " << INSN(bb->front())
            << "[label=" << QUOT(label) << "];\n";
    }
}

void plotInnerEdge(PlotData &plot, const TInsn last, const TInsn insn)
{
    plot.out << INSN(last) << " -> " << INSN(insn) << ";\n";
}

/// traverse all basic blocks of the control-flow in a predictable order
void plotCfg(PlotData &plot, const TControlFlow cfg)
{
    TBlock bb = cfg.entry();
    TWorkList wl(bb);
    while (wl.next(bb)) {
        const TInsn entry = bb->front();
        TInsn last = entry;

        BOOST_FOREACH(const TInsn insn, *bb) {
            plotInsn(plot, insn);

            if (insn != entry)
                plotInnerEdge(plot, last, insn);

            last = insn;
        }

        BOOST_FOREACH(const TBlock bbNext, bb->back()->targets)
            wl.schedule(bbNext);
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
    plotCfg(plot, fnc->cfg);

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
