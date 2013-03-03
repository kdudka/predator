/*
 * Copyright (C) 2012 Pavel Raiskup <pavel@raiskup.cz>
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

#include "config_cl.h"
#include "clplot.hh"

#include "worklist.hh"

#include "callgraph.hh"
#include "pointsto.hh"

#include <cl/storage.hh>
#include <cl/cl_msg.hh>

#include <boost/foreach.hpp>

#include <sstream>
#include <ostream>
#include <fstream>
#include <iomanip>

#define PLOT(to, indent, what) to << std::string(indent, ' ') << what
#define PLOTLINE(to, indent, what) PLOT(to, indent, what) << std::endl

namespace CodeStorage {

int graphUniqueId(const std::string &baseName)
{
    static std::map<std::string, int> nameCounter;
    if (hasKey(nameCounter, baseName))
        return ++nameCounter[baseName];

    nameCounter[baseName] = 0;
    return 0;
}

const std::string graphUniqueName(const std::string &baseName)
{
    int id = graphUniqueId(baseName);
    std::stringstream fileName;
    fileName << baseName << "-" << std::setfill('0') << std::setw(4) << id;
    fileName << ".dot";
    return fileName.str();
}

namespace CallGraph {

void dotNode(std::stringstream &dot, const Fnc *fnc)
{
    PLOTLINE(dot, 2, nameOf(*fnc));
}

void dotEdge(
        std::stringstream              &dot,
        const Fnc                      *from,
        const Fnc                      *to,
        const Insn                     *insn)
{
    PLOTLINE(dot, 2, nameOf(*from) << " -> " << nameOf(*to));
    (void)insn;
}

void plotGraph(const Storage &stor, const std::string &baseName)
{
    std::stringstream dot;
    WorkList<const Fnc *> wl;
    BOOST_FOREACH(const Fnc *fnc, stor.fncs) {
        wl.schedule(fnc);
    }

    PLOTLINE(dot, 0, "digraph CallGraph {");
    const Fnc *plotted;
    while (wl.next(plotted)) {
        // plot node
        dotNode(dot, plotted);

        const Node *clrNode = plotted->cgNode;
        BOOST_FOREACH(TInsnListByFnc::const_reference item, clrNode->calls) {
            const Fnc *callee = item.first;
            if (callee) {
                BOOST_FOREACH(const Insn *insn, item.second) {
                    dotEdge(dot, plotted, callee, insn);
                }
                wl.schedule(callee);
            }
            else {
                CL_BREAK_IF("TODO: indirect calls");
            }
        }
    }
    PLOTLINE(dot, 0, "}");

    std::string fileName = graphUniqueName(baseName);
    PT_DEBUG(0, "writing call graph to '" << fileName << "'");
    std::ofstream file(fileName.c_str());
    file << dot.str();
}

} // namespace CallGraph

namespace PointsTo {

typedef std::map<const Node *, int> TDotNodeMap;

struct PlotCtx {
    const Storage      &stor;     // CodeStorage --> to obtain variable uid
    TDotNodeMap         nodeMap;  // mapping of PTEdge *ptr to "dot" graph id
    std::stringstream   output;   // put result here
    const Graph        *ptg;      // graph of actually plotted function

    PlotCtx(const Storage &_stor) :
        stor(_stor),
        ptg(NULL)
    {
    }
};

int dotAssignId(PlotCtx &ctx, const Node *node)
{
    if (!hasKey(ctx.nodeMap, node)) {
        static int nodeCounter = 0;
        ctx.nodeMap[node] = nodeCounter++;
    }

    return ctx.nodeMap[node];
}

inline void dotPlotEdge(
        PlotCtx                         &ctx,
        const Node                    *source,
        const Node                    *target)
{
    TDotNodeMap         &map = ctx.nodeMap;
    std::stringstream   &out = ctx.output;

    dotAssignId(ctx, source);
    dotAssignId(ctx, target);

    PLOTLINE(out, 4, "node" << map[source] << "->" <<
                     "node" << map[target]);
}

bool isArtifName(const char *name)
{
    if (strlen(name) == 0)
        return true;
    if (strchr(name, '.'))
        return true;
    return false;
}

inline void dotPlotFontColor(PlotCtx &ctx, const Item *i)
{
    std::stringstream &out = ctx.output;
    switch (i->code) {
        case PT_ITEM_RET:
            PLOT(out, 0, "green");
            break;
        case PT_ITEM_VAR:
            if (i->isGlobal())
                PLOT(out, 0, "red");
            else if (isArtifName(i->data.var->name.c_str()))
                PLOT(out, 0, "gray");
            else
                PLOT(out, 0, "black");
            break;
        case PT_ITEM_MALLOC:
            PLOT(out, 0, "blue");
            break;
    }
}

inline void dotPlotNodeLabel(PlotCtx &ctx, const Node *node)
{
    std::stringstream &out = ctx.output;

    if (node->isBlackHole) {
        PLOT(out, 0, "{ BLACKHOLE }");
        return;
    }

    PLOT(out, 0, "{");
    int counter = 0;
    BOOST_FOREACH(const Item *i, node->variables) {
        if (counter)
            PLOT(out, 0, ", ");
        else
            PLOT(out, 0, " ");

        PLOT(out, 0, "<font COLOR=\"");
        dotPlotFontColor(ctx, i);
        PLOT(out, 0, "\">");
        PLOT(out, 0, i->name());
        PLOT(out, 0, "</font>");
        counter ++;
    }
    PLOT(out, 0, "}");
}

inline void dotPlotNode(PlotCtx &ctx, const Node *node)
{
    std::stringstream &out = ctx.output;
    TDotNodeMap &nodeMap = ctx.nodeMap;

    dotAssignId(ctx, node);

    PLOT(out, 4, "node" << nodeMap[node] << " [ label=<");
    dotPlotNodeLabel(ctx, node);
    PLOTLINE(out, 0, "> ];");
}

void ptPlotSubGraph(PlotCtx &ctx)
{
    const Graph &ptg = *ctx.ptg;

    WorkList<Node *> wl;
    BOOST_FOREACH(const TMap::value_type &t, ptg.map) {
        wl.schedule(t.second);
    }

    Node *plotNode;
    while (wl.next(plotNode)) {
        dotPlotNode(ctx, plotNode);
        BOOST_FOREACH(Node *outNode, plotNode->outNodes) {
            dotPlotEdge(ctx, plotNode, outNode);
            wl.schedule(outNode);
        }
        BOOST_FOREACH(Node *inNode, plotNode->inNodes) {
            wl.schedule(inNode);
        }
    }
}

void plotGraph(const Storage &stor, const std::string &baseName)
{
    PlotCtx pctx(stor);

    std::stringstream &out = pctx.output;
    PLOTLINE(out, 0, "digraph pt_graph_fics {");

    BOOST_FOREACH(const Fnc *fnc, stor.fncs) {
        pctx.ptg = &fnc->ptg;
        pctx.nodeMap.clear();

            const char *name = nameOf(*fnc);
            PLOTLINE(out, 2, "subgraph cluster_" << name << " {");
            PLOTLINE(out, 4, "label=\"" << name << "()\"");
            PLOTLINE(out, 4, "color=blue");

            PT_DEBUG(5, "Info: plotting out fnc '" << name << "'");
            ptPlotSubGraph(pctx);
            PLOTLINE(out, 2, "}");

    }

    // plot globals
    PLOTLINE(out, 2, "subgraph cluster_GLOBAL {");
    PLOTLINE(out, 4, "label=\"Global PT-Graph\";");
    PLOTLINE(out, 4, "color=red;");

    pctx.ptg = &stor.ptd.gptg;
    pctx.nodeMap.clear();
    ptPlotSubGraph(pctx);

    PLOTLINE(out, 2, "}");
    PLOTLINE(out, 0, "}");

    const std::string fileName = graphUniqueName(baseName);
    PT_DEBUG(0, "writing points-to graph into '" << fileName << "'");
    std::ofstream outfile(fileName.c_str());
    outfile << out.str();
}

} // namespace PointsTo

} // namespace CodeStorage
