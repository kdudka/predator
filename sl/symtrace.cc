/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#include "symtrace.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/storage.hh>

#include "plotenum.hh"
#include "symstate.hh"
#include "worklist.hh"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>

namespace Trace {

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::NodeBase

NodeBase::~NodeBase()
{
    BOOST_FOREACH(Node *parent, parents_)
        parent->notifyDeath(this);
}

Node* NodeBase::parent() const
{
    CL_BREAK_IF(1 != parents_.size());
    return parents_.front();
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::Node

void Node::notifyBirth(NodeBase *child)
{
    children_.push_back(child);
}

void Node::notifyDeath(NodeBase *child)
{
    // remove the dead child from the list
    children_.erase(
            std::remove(children_.begin(), children_.end(), child),
            children_.end());

    if (children_.empty())
        // FIXME: this may cause stack overflow on complex trace graphs
        delete this;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::NodeHandle

void NodeHandle::reset(Node *node)
{
    Node *&ref = parents_.front();
    if (ref == node)
        // if the node is already in, protect it against accidental deallocation
        return;

    // release the old node
    ref->notifyDeath(this);

    // register the new node
    ref = node;
    ref->notifyBirth(this);
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::plotTrace()

typedef const Node                     *TNode;
typedef std::pair<TNode, TNode>         TNodePair;
typedef WorkList<TNodePair>             TWorkList;

static Node *const nullNode = 0;

struct TracePlotter {
    std::ostream                        &out;
    TWorkList                           &wl;

    TracePlotter(std::ostream &out_, TWorkList &wl_):
        out(out_),
        wl(wl_)
    {
    }
};

std::string insnToLabel(const TInsn insn)
{
    using boost::algorithm::replace_all;

    // dump the instruction to a string stream
    std::ostringstream str;
    str << (*insn);

    // extract the string and escape all double quotes
    std::string label(str.str());
    replace_all(label,  "\\", "\\\\" );
    replace_all(label,  "\"", "\\\"" );

    return label;
}

std::string insnToBlock(const TInsn insn)
{
    CodeStorage::Block *bb = insn->bb;
    return (bb)
        ? bb->name()
        : "VAR INITIALIZER";
}

// FIXME: copy-pasted from symplot.cc
#define SL_QUOTE(what) "\"" << what << "\""

#define INSN_LOC_AND_BB(insn) SL_QUOTE((insn)->loc << insnToBlock(insn))

void TransientNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=box, color=red, fontcolor=red, label="
        << SL_QUOTE(origin_) << "];\n";
}

void RootNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=circle, color=black, fontcolor=black, label=\"start\"];\n";
}

void InsnNode::plotNode(TracePlotter &tplot) const
{
    const char *color = (isBuiltin_)
        ? "blue"
        : "black";

    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=plaintext, fontname=monospace, fontcolor=" << color
        << ", label=" << SL_QUOTE(insnToLabel(insn_))
        << ", tooltip=" << INSN_LOC_AND_BB(insn_)
        << "];\n";
}

void AbstractionNode::plotNode(TracePlotter &tplot) const
{
    const char *label;
    switch (kind_) {
        case OK_SLS:
            label = "SLS abstraction";
            break;

        case OK_DLS:
            label = "DLS abstraction";
            break;

        default:
            label = "XXX unknown abstraction";
            CL_BREAK_IF("unknown abstraction");
    }

    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=ellipse, color=red, fontcolor=red, label="
        << SL_QUOTE(label) << ", tooltip="
        << SL_QUOTE(name_) << "];\n";
}

void ConcretizationNode::plotNode(TracePlotter &tplot) const
{
    // TODO: kind_
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=ellipse, color=red, fontcolor=blue, label="
        << SL_QUOTE("concretizeObj()") << ", tooltip="
        << SL_QUOTE(name_) << "];\n";
}

void SpliceOutNode::plotNode(TracePlotter &tplot) const
{
    // TODO: kind_, successful_
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=ellipse, color=red, fontcolor=blue, label="
        << SL_QUOTE("spliceOut*(len = " << len_ << ")") << "];\n";
}

void JoinNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=circle, color=red, fontcolor=red, label=\"join\"];\n";
}

void CloneNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this) << " [shape=doubleoctagon, color=black"
        ", fontcolor=black, label=\"clone\"];\n";
}

void CallEntryNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=box, fontname=monospace, color=blue, fontcolor=blue"
        ", penwidth=3.0, label=\"--> call entry: " << (insnToLabel(insn_))
        << "\", tooltip=\"" << insn_->loc << insn_->bb->name() << "\"];\n";
}

void CallCacheHitNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=box, fontname=monospace, color=gold, fontcolor=blue"
        ", penwidth=3.0, label=\"(x) call cache hit: "
        << (nameOf(*fnc_)) << "()\"];\n";
}

void CallFrameNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=box, fontname=monospace, color=blue, fontcolor=blue"
        ", label=\"--- call frame: " << (insnToLabel(insn_))
        << "\", tooltip=" << INSN_LOC_AND_BB(insn_) << "];\n";
}

void CallDoneNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=box, fontname=monospace, color=blue, fontcolor=blue"
        ", penwidth=3.0, label=\"<-- call done: "
        << (nameOf(*fnc_)) << "()\"];\n";
}

void ImportGlVarNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this) << " [shape=ellipse, color=red"
        ", fontcolor=red, label=\"importGlVar(" << varString_ << ")\"];\n";
}

void CondNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this) << " [shape=box, fontname=monospace"
        ", tooltip=" << INSN_LOC_AND_BB(inCnd_);

    if (determ_)
        tplot.out << ", color=green";
    else
        tplot.out << ", color=red";

    tplot.out << ", fontcolor=black, label=\"" << (*inCmp_) << " ... ";

    if (determ_)
        tplot.out << "evaluated as ";
    else
        tplot.out << "assuming ";

    if (branch_)
        tplot.out << "TRUE";
    else
        tplot.out << "FALSE";

    tplot.out << "\"];\n";
}

void MsgNode::plotNode(TracePlotter &tplot) const
{
    const char *color = "red";
    const char *label;
    switch (level_) {
        case ML_DEBUG:
            color = "black";
            label = "ML_DEBUG";
            break;

        case ML_WARN:
            color = "gold";
            label = "ML_WARN";
            break;

        case ML_ERROR:
            label = "ML_ERROR";
            break;

        default:
            label = "ML_XXX";
            CL_BREAK_IF("unhandled EMsgLevel in MsgNode");
    }

    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=tripleoctagon, fontcolor=monospace, color="
        << color << ", fontcolor=red, label="
        << SL_QUOTE((*loc_) << label) << "];\n";
}

void UserNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this) << " [shape=octagon, penwidth=3.0"
        ", color=green, fontcolor=black, label=\"" << label_ << "\"];\n";
}

void plotTraceCore(TracePlotter &tplot)
{
    CL_DEBUG("plotTraceCore() is traversing a trace graph...");

    TNodePair item;
    while (tplot.wl.next(item)) {
        const TNode now = /* from */ item.first;
        const TNode to  = /* to   */ item.second;
        item.second = now;

        BOOST_FOREACH(TNode from, now->parents()) {
            item.first = from;
            tplot.wl.schedule(item);
        }

        now->plotNode(tplot);
        if (!to)
            continue;

        tplot.out << "\t" << SL_QUOTE(now)
            << " -> " << SL_QUOTE(to)
            << " [color=black, fontcolor=black];\n";
    }
}

// FIXME: copy-pasted from symplot.cc
bool plotTrace(const std::string &name, TWorkList &wl, std::string *pName = 0)
{
    PlotEnumerator *pe = PlotEnumerator::instance();
    std::string plotName(pe->decorate(name));
    std::string fileName(plotName + ".dot");

    if (pName)
        // propagate the resulting name back to the caller
        *pName = plotName;

    // create a dot file
    std::fstream out(fileName.c_str(), std::ios::out);
    if (!out) {
        CL_ERROR("unable to create file '" << fileName << "'");
        return false;
    }

    // open graph
    out << "digraph " << SL_QUOTE(plotName)
        << " {\n\tlabel=<<FONT POINT-SIZE=\"18\">" << plotName
        << "</FONT>>;\n\tlabelloc=t;\n";

    // check whether we can write to stream
    if (!out.flush()) {
        CL_ERROR("unable to write file '" << fileName << "'");
        out.close();
        return false;
    }

    // do our stuff
    TracePlotter tplot(out, wl);
    plotTraceCore(tplot);

    // close graph
    out << "}\n";
    out.close();
    CL_NOTE("trace graph dumped to '" << fileName << "'");
    return !!out;
}

bool plotTrace(Node *endPoint, const std::string &name, std::string *pName)
{
    const TNodePair item(/* from */ endPoint, /* to */ nullNode);
    TWorkList wl(item);
    return plotTrace(name, wl, pName);
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::printTrace()
Node* /* selected predecessor */ TransientNode::printNode() const
{
    CL_BREAK_IF("please implement");
    return this->parent();
}

Node* /* selected predecessor */ RootNode::printNode() const
{
    // reaching this node means we are done with tracing!
    return 0;
}

Node* /* selected predecessor */ InsnNode::printNode() const
{
    // TODO: handle selected instructions here?
    return this->parent();
}

Node* /* selected predecessor */ JoinNode::printNode() const
{
    // FIXME: deal better with join nodes
    return this->parents().front();
}

Node* /* selected predecessor */ CloneNode::printNode() const
{
    CL_BREAK_IF("please implement");
    return this->parent();
}

Node* /* selected predecessor */ CallEntryNode::printNode() const
{
    CL_NOTE_MSG(&insn_->loc, "from call of " << (*insn_));
    return this->parent();
}

Node* /* selected predecessor */ CallCacheHitNode::printNode() const
{
    // follow the result, not the entry!
    return this->parents().at(/* result */ 1);
}

Node* /* selected predecessor */ CallFrameNode::printNode() const
{
    CL_BREAK_IF("please implement");
    return this->parent();
}

Node* /* selected predecessor */ CallDoneNode::printNode() const
{
    // follow the call, not the frame!
    return this->parents().front();
}

Node* /* selected predecessor */ CondNode::printNode() const
{
    const char *action = (determ_)
        ? "evaluated as "
        : "assuming ";

    const char *result = (branch_)
        ? "TRUE"
        : "FALSE";

    CL_NOTE_MSG(&inCmp_->loc, (*inCmp_) << " ... " << action << result);
    return this->parent();
}

Node* /* selected predecessor */ UserNode::printNode() const
{
    CL_BREAK_IF("please implement");
    return this->parent();
}

void printTrace(Node *endPoint)
{
    while ((endPoint = endPoint->printNode()))
        ;
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::chkTraceGraphConsistency()

template <class TNodeKind>
bool isNodeKindReachble(Node *const from)
{
    Node *node = from;
    WorkList<Node *> wl(node);
    while (wl.next(node)) {
        if (dynamic_cast<TNodeKind *>(node))
            return true;

        BOOST_FOREACH(Node *pred, node->parents())
            wl.schedule(pred);
    }

    // not reachable
    return false;
}

bool chkTraceGraphConsistency(Node *const from)
{
    if (isNodeKindReachble<CloneNode>(from)) {
        CL_WARN("CloneNode reachable from the given trace graph node");
        plotTrace(from, "symtrace-CloneNode-reachable");
    }

    if (!isNodeKindReachble<RootNode>(from)) {
        CL_ERROR("RootNode not reachable from the given trace graph node");
        plotTrace(from, "symtrace-RootNode-not-reachable");
        return false;
    }

    if (isNodeKindReachble<TransientNode>(from)) {
        CL_ERROR("TransientNode reachable from the given trace graph node");
        plotTrace(from, "symtrace-TransientNode-reachable");
        return false;
    }

    // no problems encountered
    return true;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::EndPointConsolidator

struct EndPointConsolidator::Private {
    typedef std::set<Node *>                                    TNodeSet;
    typedef std::vector<NodeHandle>                             THandleList;

    bool                        dirty;
    TNodeSet                    nset;
    THandleList                 handles;

    Private():
        dirty(false)
    {
    }
};

EndPointConsolidator::EndPointConsolidator():
    d(new Private)
{
}

EndPointConsolidator::~EndPointConsolidator()
{
    if (d->dirty)
        CL_DEBUG("WARNING: EndPointConsolidator is destructed dirty");

    // release all handles
    d->handles.clear();

    delete d;
}

bool /* any change */ EndPointConsolidator::insert(Node *endPoint)
{
    if (!insertOnce(d->nset, endPoint))
        return false;

    // keep a handle for the newly inserted node
    d->handles.push_back(NodeHandle(endPoint));

    return ((d->dirty = true));
}

bool EndPointConsolidator::plotAll(const std::string &name)
{
    d->dirty = false;

    // schedule all end-points
    TWorkList wl;
    BOOST_FOREACH(Node *endPoint, d->nset) {
        const TNodePair item(/* from */ endPoint, /* to */ nullNode);
        wl.schedule(item);
    }

    // plot everything
    return plotTrace(name, wl);
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::GraphProxy

struct GraphProxy::Private {
    typedef std::map<std::string, EndPointConsolidator *>       TMap;
    TMap gmap;
};

GraphProxy::GraphProxy():
    d(new Private)
{
}

GraphProxy::~GraphProxy()
{
    BOOST_FOREACH(Private::TMap::const_reference item, d->gmap)
        delete /* (EndPointConsolidator *) */ item.second;

    delete d;
}

bool /* any change */ GraphProxy::insert(Node *node, const std::string &name)
{
    Private::TMap::const_iterator it = d->gmap.find(name);

    EndPointConsolidator *const epc = (d->gmap.end() == it)
        ? (d->gmap[name] = new EndPointConsolidator)
        : (it->second);

    return /* any change */ epc->insert(node);
}

bool GraphProxy::plotGraph(const std::string &name)
{
    CL_BREAK_IF(!hasKey(d->gmap, name));

    return d->gmap[name]->plotAll(name);
}

bool GraphProxy::plotAll()
{
    bool ok = true;

    BOOST_FOREACH(Private::TMap::const_reference item, d->gmap) {
        const std::string &name = item.first;
        EndPointConsolidator *const epc = item.second;

        if (!epc->plotAll(name))
            ok = false;
    }

    return ok;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::Globals

Globals *Globals::inst_;


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::waiveCloneOperation()

void waiveCloneOperation(SymHeap &sh)
{
    // just make sure the caller knows what is going on...
    Node *cnode = sh.traceNode();
    CL_BREAK_IF(!dynamic_cast<CloneNode *>(cnode));

    // bypass the parental node
    sh.traceUpdate(cnode->parent());
}

void waiveCloneOperation(SymState &state)
{
    BOOST_FOREACH(SymHeap *sh, state)
        Trace::waiveCloneOperation(*sh);
}

} // namespace Trace
