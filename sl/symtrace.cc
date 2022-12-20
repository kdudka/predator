/*
 * Copyright (C) 2011-2022 Kamil Dudka <kdudka@redhat.com>
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

namespace Trace {

typedef const Node                                     *TNode;
typedef std::set<TNode>                                 TNodeSet;

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::NodeBase

NodeBase::~NodeBase()
{
}

Node* NodeBase::parent() const
{
    CL_BREAK_IF(1 != parents_.size());
    return parents_.front();
}

void NodeBase::replaceParent(Node *parentOld, Node *parentNew)
{
    typedef TNodeList::iterator TIt;
    const TIt itToRepl = std::find(parents_.begin(), parents_.end(), parentOld);
    CL_BREAK_IF(itToRepl == parents_.end());

    parentOld->notifyDeath(this);
    *itToRepl = parentNew;
    parentNew->notifyBirth(this);
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::Node

/// this runs in the debug build only
bool hasDupChildren(const Node *node)
{
    std::set<const NodeBase *> seen;
    for (const NodeBase *child : node->children())
        if (!insertOnce(seen, child))
            return true;

    return false;
}

Node::~Node()
{
    if (!alive_)
        // this node is already being destroyed
        return;

    alive_ = false;

    // visit all parents recursively and propagate death notification
    Node *node;
    WorkList<Node *> wl(this);
    while (wl.next(node)) {
        for (Node *parent : node->parents()) {
            CL_BREAK_IF(parent->children_.empty());

            // temporarily disable node deletion to avoid stack overflow
            const bool wasAlive = parent->alive_;
            parent->alive_ = false;

            // propagate death notification
            parent->notifyDeath(node);

            // check whether this node was the last child of the parent
            if (!parent->children_.empty()) {
                parent->alive_ = wasAlive;
                continue;
            }

            // schedule the parent with no children for deletion
            wl.schedule(parent);
        }

        if (this != node) {
            // delete a node that nobody points to any more
            CL_BREAK_IF(!node->children_.empty());
            CL_BREAK_IF(node->alive_);
            delete node;
        }
    }
}

void Node::notifyBirth(NodeBase *child)
{
    CL_BREAK_IF(hasDupChildren(this));
    children_.push_back(child);
    CL_BREAK_IF(hasDupChildren(this));
}

void Node::notifyDeath(NodeBase *child)
{
    CL_BREAK_IF(hasDupChildren(this));

    // remove the dead child from the list
    children_.erase(
            std::remove(children_.begin(), children_.end(), child),
            children_.end());

    if (alive_ && children_.empty())
        delete this;
}

TIdMapperList& Node::idMapperList()
{
    CL_BREAK_IF(parents_.size() != idMapperList_.size());
    return idMapperList_;
}

const TIdMapperList& Node::idMapperList() const
{
    CL_BREAK_IF(parents_.size() != idMapperList_.size());
    return idMapperList_;
}

TIdMapper& Node::idMapper()
{
    CL_BREAK_IF(1U != this->idMapperList().size());
    return idMapperList_.front();
}

const TIdMapper& Node::idMapper() const
{
    CL_BREAK_IF(1U != this->idMapperList().size());
    return idMapperList_.front();
}

void replaceNode(Node *tr, Node *by)
{
    CL_BREAK_IF(hasDupChildren(tr));
    CL_BREAK_IF(hasDupChildren(by));

    // we intentionally deep-copy the list int order to allow its safe traversal
    const Node::TBaseList children = tr->children();
    for (NodeBase *const child : children) {
        if (child == by)
            // avoid creating a self-loop
            continue;

        child->replaceParent(tr, by);
    }

    CL_BREAK_IF(hasDupChildren(tr));
    CL_BREAK_IF(hasDupChildren(by));
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::NodeHandle

NodeHandle::~NodeHandle()
{
    this->parent()->notifyDeath(this);
}

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
// implementation of Trace::resolveIdMapping()

bool seekAncestor(TNode tr, const TNode trAncestor, const TNodeSet &blackList)
{
    WorkList<TNode> wl(tr);
    while (wl.next(tr)) {
        if (trAncestor == tr)
            // found!
            return true;

        if (hasKey(blackList, tr))
            // loop detected!
            continue;

        for (const Node *trParent : tr->parents())
            wl.schedule(trParent);
    }

    // not found
    return false;
}

int parentIdxByAncestor(
        const TNode                 tr,
        const TNode                 trAncestor,
        const TNodeSet             &blackList)
{
    const TNodeList &parents = tr->parents();
    const int cnt = parents.size();
    if (cnt <= 1)
        return cnt - 1;

    for (int i = 0; i < cnt; ++i)
        if (seekAncestor(parents[i], trAncestor, blackList))
            return i;

    return /* not found */ -1;
}

void resolveIdMapping(TIdMapper *pDst, const Node *trSrc, const Node *trDst)
{
    CL_BREAK_IF(!pDst->empty());

    // start with identity, then go through the trace and construct composition
    pDst->setNotFoundAction(TIdMapper::NFA_RETURN_IDENTITY);

    TNodeSet seen;
    TNode tr = trDst;
    while (trSrc != tr && insertOnce(seen, tr)) {
        const int idx = parentIdxByAncestor(tr, trSrc, seen);
        if (idx < 0) {
            CL_BREAK_IF("resolveIdMapping() routing failure");
            break;
        }

        pDst->composite<D_RIGHT_TO_LEFT>(tr->idMapperList()[idx]);

        tr = tr->parents()[idx];
    }
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of Trace::plotTrace()

typedef const Node                     *TNode;
static Node *const nullNode = 0;

struct TraceEdge {
    TNode               src;
    TNode               dst;
    int                 idx;

    TraceEdge():
        src(nullNode),
        dst(nullNode),
        idx(0)
    {
    }
};

inline bool operator<(const TraceEdge &a, const TraceEdge &b)
{
    RETURN_IF_COMPARED(a, b, src);
    RETURN_IF_COMPARED(a, b, dst);
    RETURN_IF_COMPARED(a, b, idx);
    return false;
}

typedef WorkList<TraceEdge>             TWorkList;

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

#define INSN_LOC_AND_BB(insn, ptr) SL_QUOTE((insn)->loc << insnToBlock(insn) \
        << " (" << (ptr) << ")")

void TransientNode::plotNode(TracePlotter &tplot) const
{
    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=box, color=red, fontcolor=red, label="
        << SL_QUOTE(origin_) << "];\n";
}

void RootNode::plotNode(TracePlotter &tplot) const
{
    // TODO
    (void) rootFnc_;

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
        << ", tooltip=" << INSN_LOC_AND_BB(insn_, this)
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
    // TODO
    (void) kind_;

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
    const char *color = "red";
    const char *label = "XXX";

    switch (status_) {
        case JS_USE_ANY:
            color = "black";
            label = "sh1 = sh2";
            break;

        case JS_USE_SH1:
            label = "sh1 > sh2";
            break;

        case JS_USE_SH2:
            label = "sh1 < sh2";
            break;

        case JS_THREE_WAY:
            label = "join";
            break;
    }

    tplot.out << "\t" << SL_QUOTE(this)
        << " [shape=circle, color=" << color
        << ", fontcolor=" << color
        << ", label=\"" << label << "\"];\n";
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
        << "\", tooltip=" << INSN_LOC_AND_BB(insn_, this) << "];\n";
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
        ", tooltip=" << INSN_LOC_AND_BB(inCnd_, this);

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
    // TODO
    (void) insn_;

    tplot.out << "\t" << SL_QUOTE(this) << " [shape=octagon, penwidth=3.0"
        ", color=green, fontcolor=black, label=\"" << label_ << "\"];\n";
}

void plotTraceCore(TracePlotter &tplot)
{
    CL_DEBUG("plotTraceCore() is traversing a trace graph...");

    TraceEdge item;
    while (tplot.wl.next(item)) {
        const TNode src = item.src;
        const TNode dst = item.dst;
        const int   idx = item.idx;
        item.dst = src;

        const TNodeList &parents = src->parents();
        const int cntParents = parents.size();
        for (item.idx = 0; item.idx < cntParents; ++item.idx) {
            item.src = parents[item.idx];
            tplot.wl.schedule(item);
        }

        src->plotNode(tplot);
        if (!dst)
            continue;

        tplot.out << "\t" << SL_QUOTE(src)
            << " -> " << SL_QUOTE(dst)
            << " [color=" << ((!idx) ? "blue" : "black")
            << "];\n";
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
    TraceEdge item;
    item.src = endPoint;
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

Node* JoinNode::parent() const
{
    switch (status_) {
        case JS_USE_SH1:
            return this->parents()[/* tr1 */ 0];

        case JS_USE_SH2:
            return this->parents()[/* tr2 */ 1];

        default:
            return Node::parent();
    }
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
bool isNodeKindReachable(Node *const from)
{
    Node *node = from;
    WorkList<Node *> wl(node);
    while (wl.next(node)) {
        CL_BREAK_IF(!node->alive_);

        if (dynamic_cast<TNodeKind *>(node))
            return true;

        for (Node *pred : node->parents())
            wl.schedule(pred);
    }

    // not reachable
    return false;
}

bool chkTraceGraphConsistency(Node *const from)
{
    if (isNodeKindReachable<CloneNode>(from)) {
        CL_WARN("CloneNode reachable from the given trace graph node");
        plotTrace(from, "symtrace-CloneNode-reachable");
    }

    if (!isNodeKindReachable<RootNode>(from)) {
        CL_ERROR("RootNode not reachable from the given trace graph node");
        plotTrace(from, "symtrace-RootNode-not-reachable");
        return false;
    }

    if (isNodeKindReachable<TransientNode>(from)) {
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
    TraceEdge item;
    for (Node *endPoint : d->nset) {
        item.src = endPoint;
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
    for (Private::TMap::const_reference item : d->gmap)
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

    for (Private::TMap::const_reference item : d->gmap) {
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
    for (SymHeap *sh : state)
        Trace::waiveCloneOperation(*sh);
}

} // namespace Trace
