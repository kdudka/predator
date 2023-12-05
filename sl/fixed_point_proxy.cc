/*
 * Copyright (C) 2013-2022 Kamil Dudka <kdudka@redhat.com>
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

#include "adt_op.hh"
#include "adt_op_def.hh"
#include "adt_op_match.hh"
#include "adt_op_replace.hh"
#include "cont_shape_seq.hh"
#include "cont_shape_var.hh"
#include "fixed_point.hh"
#include "glconf.hh"
#include "symplot.hh"
#include "symtrace.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/storage.hh>

#include <fstream>
#include <iomanip>
#include <map>

namespace FixedPoint {

using AdtOp::TShapeVarByShape;

typedef const struct cl_loc                        *TLoc;
typedef cl_uid_t                                    TFncUid;
typedef std::map<TFncUid, TFnc>                     TFncMap;

typedef const CodeStorage::Block                   *TBlock;

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

const StateByInsn::TStateMap& StateByInsn::stateMap() const
{
    return d->stateByInsn;
}

struct PlotData {
    int                             subGraphIdx;
    std::ostream                   &out;
    StateByInsn::TStateMap         &stateByInsn;
    std::string                     name;

    PlotData(
            std::ostream           &out_,
            StateByInsn::TStateMap &stateByInsn_,
            const std::string      &name_):
        subGraphIdx(0),
        out(out_),
        stateByInsn(stateByInsn_),
        name(name_)
    {
    }
};

#define QUOT(what) "\"" << what << "\""
#define LOC_NODE(plot, locIdx) QUOT("loc" << plot.subGraphIdx << "." << locIdx)
#define SH_NODE(sh) QUOT("loc" << (sh.first) << "-sh" << (sh.second))
#define DOT_LINK(to) "\"" << to << ".svg\""
#define STD_SETW(n) std::fixed << std::setfill('0') << std::setw(n)

typedef std::set<THeapIdent>                        THeapSet;
typedef AdtOp::TShapeVarByShape                     TVarMap;

class RewriteCapture: public IStateRewriter {
    public:
        virtual void insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn);
        virtual void replaceInsn(TLocIdx at, GenericInsn *insn);
        virtual void dropInsn(TLocIdx at);
        virtual void dropEdge(TLocIdx src, TLocIdx dst);
        virtual void redirEdge(TLocIdx from, TLocIdx to, TLocIdx redirTo);

    public:
        std::string locLabel(TLocIdx loc) const;
        std::string edgeLabel(TLocIdx from, TLocIdx to) const;

    private:
        typedef std::pair<TLocIdx, TLocIdx>         TEdge;
        typedef std::map<TLocIdx, std::string>      TLocLabels;
        typedef std::map<TEdge, std::string>        TEdgeLabels;

        TLocLabels                  locLabels_;
        TEdgeLabels                 edgeLabels_;
};

void RewriteCapture::insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn)
{
    const TEdge edge(src, dst);

    std::ostringstream str;
    if (hasKey(edgeLabels_, edge))
        str << "; ";
    str << *insn;

    edgeLabels_[edge] += str.str();
}

void RewriteCapture::replaceInsn(TLocIdx at, GenericInsn *insn)
{
    std::ostringstream str;
    str << " ... " << *insn;
    locLabels_[at] = str.str();
}

void RewriteCapture::dropInsn(TLocIdx at)
{
    locLabels_[at] = " ... to be removed";
}

void RewriteCapture::dropEdge(TLocIdx src, TLocIdx dst)
{
    const TEdge edge(src, dst);
    edgeLabels_[edge] += " ... to be removed";
}

void RewriteCapture::redirEdge(TLocIdx from, TLocIdx to, TLocIdx redirTo)
{
    const TEdge edge(from, to);
    std::stringstream str;
    str << " ... to be redirected to #" << redirTo;
    edgeLabels_[edge] += str.str();
}

std::string RewriteCapture::locLabel(TLocIdx loc) const
{
    const TLocLabels::const_iterator it = locLabels_.find(loc);
    return (it == locLabels_.end()) ? "" : it->second;
}

std::string RewriteCapture::edgeLabel(TLocIdx from, TLocIdx to) const
{
    const TEdge edge(from, to);
    const TEdgeLabels::const_iterator it = edgeLabels_.find(edge);
    return (it == edgeLabels_.end()) ? "" : it->second;
}


void plotInsn(
        THeapSet                   *pHeapSet,
        PlotData                   &plot,
        const TVarMap              &varMap,
        const TLocIdx               locIdx,
        const std::string          &locLabelSuffix,
        const LocalState           &locState)
{
    if (!locState.insn)
        // an already removed insn
        return;

    const char *color = (locLabelSuffix.empty())
        ? "black"
        : "chartreuse2";

    // open cluster
    plot.out << "subgraph \"cluster" << plot.subGraphIdx << "." << locIdx
        << "\" {\n\tlabel=\"loc #" << locIdx << locLabelSuffix
        << "\";\n\tfontcolor=" << color << ";\n";

    // plot the root node
    const TInsn insn = locState.insn->clInsn();
    if (insn)
        plot.out << LOC_NODE(plot, locIdx)
            << " [label=" << QUOT(Trace::insnToLabel(insn))
            << ", tooltip=" << QUOT(insn->loc)
            << ", shape=box, color=blue, fontcolor=blue];\n";
    else
        plot.out << LOC_NODE(plot, locIdx)
            << " [label=" << QUOT(*locState.insn)
            << ", shape=box, color=blue, fontcolor=red];\n";

    const SymState &state = locState.heapList;

    const THeapIdx cntHeaps = state.size();
    for (THeapIdx shIdx = 0; shIdx < cntHeaps; ++shIdx) {
        const THeapIdent shIdent(locIdx, shIdx);
        const SymHeap &sh = state[shIdx];
        const TShapeList &shapeList = locState.shapeListByHeapIdx[shIdx];

        TIdSet contShapeIds;
        for (const Shape &shape : shapeList) {
            TObjSet contShapeObjs;
            objSetByShape(&contShapeObjs, sh, shape);
            for (const TObjId obj : contShapeObjs)
                contShapeIds.insert(static_cast<int>(obj));
        }

        std::ostringstream nameStr;
        nameStr << plot.name
            << "-loc" << STD_SETW(4) << locIdx
            << "-sh" << STD_SETW(4) << shIdx;

        // plot the shape graph
        std::string shapeName;
        plotHeap(sh, nameStr.str(), /* loc */ 0, &shapeName, &contShapeIds);

        // plot the link to shape
        plot.out << SH_NODE(shIdent) << " [label=\"sh #" << shIdx;
        const TShapeIdent csIdent(shIdent, /* XXX */ 0);
        const TVarMap::const_iterator it = varMap.find(csIdent);
        if (varMap.end() != it)
            plot.out << " C" << it->second;
        plot.out << "\", URL=" << DOT_LINK(shapeName);

        if (1U == pHeapSet->erase(shIdent))
            // template instance mached on this heap!
            plot.out << ", color=chartreuse2, penwidth=4";
        else {
            const unsigned csCnt = shapeList.size();
            if (csCnt)
                plot.out << ", color=red, penwidth=" << (2U * csCnt);
        }

        plot.out << "];\n";
    }

    // close cluster
    plot.out << "}\n";
}

void plotFncCore(
        PlotData                   &plot,
        const GlobalState          &fncState,
        const TShapeVarByShape     &varByShape  = TShapeVarByShape(),
        const RewriteCapture       &capture     = RewriteCapture(),
        THeapSet                    heapSet     = THeapSet())
{
    const TLocIdx locCnt = fncState.size();
    for (TLocIdx locIdx = 0; locIdx < locCnt; ++locIdx) {
        const LocalState &locState = fncState[locIdx];
        const std::string locLabelSuffix = capture.locLabel(locIdx);
        plotInsn(&heapSet, plot, varByShape, locIdx, locLabelSuffix, locState);

        // plot trace edges
        for (const TTraceEdgeList &tList : locState.traceOutEdges) {
            for (const TraceEdge *te : tList) {
                plot.out << SH_NODE(te->src) << " -> " << SH_NODE(te->dst);

                const unsigned cnt = te->csMap.size();
                if (cnt) {
                    // we have non-empty container shape mapping for this edge
                    plot.out << " [color=red, fontcolor=red";
                    if (1U < cnt)
                        plot.out << ", label=" << cnt << ", penwidth=" << cnt;
                    plot.out << "]";
                }

                plot.out << ";\n";
            }
        }

        const unsigned cntTargets = locState.cfgOutEdges.size();
        for (unsigned i = 0; i < cntTargets; ++i) {
            const CfgEdge &edge = locState.cfgOutEdges[i];
            const TLocIdx tgt = edge.targetLoc;
            const std::string edgeLabelSuffix = capture.edgeLabel(locIdx, tgt);

            const char *color = "blue";
            if (edge.closesLoop)
                // loop-closing edge
                color = "chartreuse2";

            std::string label;
            if (2U == cntTargets) {
                // assume CL_INSN_COND
                label = (!i) ? "T" : "F";
                if (!edgeLabelSuffix.empty())
                    label += " ... ";
            }

            plot.out << LOC_NODE(plot, locIdx)
                << " -> " << LOC_NODE(plot, tgt)
                << " [label=" << QUOT(label + edgeLabelSuffix)
                << ", color=" << color
                << ", fontcolor=" << color << "];\n";
        }
    }
}

// XXX
AdtOp::OpCollection adtOps;

void plotFixedPointOfFnc(PlotData &plot, const GlobalState &fncState)
{
    if (!GlConf::data.detectContainers) {
        plotFncCore(plot, fncState);
        return;
    }

    // back up the original CFG and create a writer for the resulting one
    GlobalState cfgOrig;
    exportControlFlow(&cfgOrig, fncState);
    RecordRewriter recorder;

    MultiRewriter writer;
    writer.appendWriter(recorder);

    RewriteCapture capture;
    writer.appendWriter(capture);

    // match templates
    using namespace AdtOp;
    TMatchList matchList;
    matchTemplates(&matchList, adtOps, fncState);
    selectApplicableMatches(&matchList, fncState);

    TOpList opList;
    if (!collectOpList(&opList, matchList))
        CL_ERROR("[ADT] failed to detect container operations");

    // assign shape variables
    TShapeVarByShape varByShape;
    if (!assignShapeVariables(&varByShape, &writer, matchList, opList, adtOps,
                fncState))
        CL_WARN("[ADT] failed to assign shape variables");

    // replace container operations
    if (!replaceAdtOps(&writer, matchList, opList, adtOps, varByShape,
                fncState))
        CL_ERROR("[ADT] failed to replace container operations");

    // remove matched heaps not representing any instructions to be replaced
    for (FootprintMatch &fm : matchList) {
        CL_BREAK_IF(fm.matchedHeaps.empty());
        fm.matchedHeaps.pop_back();
    }

    // compute set of matched heaps
    THeapSet heapSet;
    for (FootprintMatch &fm : matchList) {
        CL_BREAK_IF(fm.matchedHeaps.empty());
        for (const THeapIdent &heap : fm.matchedHeaps)
            heapSet.insert(heap);
    }

    GlobalState cfgResult;
    exportControlFlow(&cfgResult, fncState);
    StateRewriter rewriter(&cfgResult);
    recorder.flush(&rewriter);

    // plot the annotated input CFG
    plotFncCore(plot, fncState, varByShape, capture, std::move(heapSet));

    if (cfgOrig.size() < 16) {
        // plot the original CFG-only subgraph
        ++plot.subGraphIdx;
        plotFncCore(plot, cfgOrig);

        // plot the resulting CFG
        ++plot.subGraphIdx;
        plotFncCore(plot, cfgResult);
    }

    // plot the resulting CFG after removing dead code
    ++plot.subGraphIdx;
    removeDeadCode(&cfgResult);
    plotFncCore(plot, cfgResult);
}

void plotFnc(const TFnc fnc, StateByInsn::TStateMap &stateByInsn)
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
    plotFixedPointOfFnc(plot, *fncState);
    delete fncState;

    // close graph
    out << "}\n";
    if (!out)
        CL_ERROR("unable to write file '" << fileName << "'");
    out.close();
}

void StateByInsn::plotAll()
{
    if (d->visitedFncs.empty())
        // nothing to plot
        return;

    // obtain a reference to CodeStorage::Storage
    TStorRef stor = *d->visitedFncs.begin()->second->stor;

    if (GlConf::data.detectContainers)
        AdtOp::loadDefaultOperations(&adtOps, stor);

    for (TFncMap::const_reference fncItem : d->visitedFncs) {
        const TFnc fnc = fncItem.second;
        const TLoc loc = locationOf(*fnc);
        CL_NOTE_MSG(loc, "plotting fixed-point of " << nameOf(*fnc) << "()...");

        plotFnc(fnc, d->stateByInsn);
    }
}

} // namespace FixedPoint
