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

#include "util.hh"
#include "stopwatch.hh"
#include "builtins.hh"

#include "pointsto.hh"
#include "pointsto_fics.hh"
#include "pointsto_assert.hh"

#include "worklist.hh"
#include "clplot.hh"

#include <cl/clutil.hh>

#include <algorithm>

#include <boost/foreach.hpp>

int pt_dbg_level = CL_DEBUG_POINTS_TO;

namespace CodeStorage {

namespace PointsTo {

// there may occur multiple out-edges
void addEdge(Node *from, Node *to)
{
    insertOnce(from->outNodes, to);
    insertOnce(to->inNodes, from);
}

void clearEdgeS(Node *from, Node *to)
{
    CL_BREAK_IF(!hasKey(from->outNodes, to));
    CL_BREAK_IF(!hasKey(to->inNodes, from));

    from->outNodes.erase(to);
    to->inNodes.erase(from);
    // no out node may remain for Single-Output graph
    CL_BREAK_IF(from->outNodes.size() != 0);
}

bool bindVarList(
        BuildCtx                       &ctx,
        Graph                          &ptg,
        Node                           *target,
        const TItemList                &nl)
{
    CL_BREAK_IF(existsError(ctx.stor));

    bool changed = false;
    BOOST_FOREACH(const Item *i, nl) {
        if (!hasKey(ptg.map, i->uid())) {
            // this variable is still not in target graph
            bindItem(ptg, target, i);
            changed = true;
            continue;
        }

        if (hasItem(target->variables, i))
            // this variable is in target node already
            continue;

        // this item is in other node now..
        PT_DEBUG(0, "may this even happen?");
        CL_BREAK_IF(ctx.joinTodo.size() > 0);

        Node *dupl = findNode(ptg, i); // 'i' must exist in pgt
        CL_BREAK_IF(!dupl);

        // here must be set firstly node 'target' !
        ctx.joinTodo.push_back(TNodePair(target, dupl));
        joinFixPointS(ctx, ptg);
        changed = true;
    }

    return changed;
}

Node *preventEndingS(Node *node)
{
    Node *target = getOutputS(node);
    if (!target) {
        target = new Node;
        addEdge(node, target);
    }

    return target;
}

// assign or move Item into node within graph
void bindItem(Graph &ptg, Node *n, const Item *i)
{
    CL_BREAK_IF(!n || !i);

    int uid = i->uid();

    if (!hasItem(n->variables, i))
        n->variables.push_back(i);
    ptg.map[uid] = n;

    if (i->isGlobal() && !hasItem(ptg.globals, i))
        ptg.globals.push_back(i);

    if (hasKey(ptg.uidToItem, uid))
        CL_BREAK_IF(ptg.uidToItem[uid] != i);
    else
        ptg.uidToItem[uid] = i;
}

void joinFixPointS(BuildCtx &ctx, Graph &ptg)
{
    CL_BREAK_IF(existsError(ctx.stor));

    TNodeJoinTodo &todo = ctx.joinTodo;
    bool changed = 0;
    while (!todo.empty()) {
        TNodePair pair = todo.back();
        todo.pop_back();

        joinNodesS(ctx, ptg, pair.first, pair.second);
        changed = true;
    }

    // verbose plotting
    if (changed && ctx.plot.progress)
        plotGraph(ctx.stor, ctx.plot.progress);
}

void joinNodesS(
        BuildCtx                       &ctx,
        Graph                          &ptg,
        Node                           *nodeLeft,
        Node                           *nodeRight)
{
    CL_BREAK_IF(existsError(ctx.stor));

    if (nodeLeft == nodeRight)
        // just skip -- do not fail
        return;

    BOOST_FOREACH(const Item *i, nodeRight->variables)
        // re-map nodeB's variables to nodeA
        bindItem(ptg, nodeLeft, i);

    // harvest all existing rightNode-related edges
    Node *leftTarget, *rightTarget;
    std::vector<TNodePair> pairs;
    TNodeList &inNodes = nodeRight->inNodes;
    while (inNodes.size() != 0) {
        Node *inNodeRight = *inNodes.begin();
        pairs.push_back(TNodePair(inNodeRight, nodeRight));
        clearEdgeS(inNodeRight, nodeRight);
    }
    CL_BREAK_IF(inNodes.size() > 0);

    rightTarget = getOutputS(nodeRight);
    if (rightTarget) {
        pairs.push_back(TNodePair(nodeRight, rightTarget));
        clearEdgeS(nodeRight, rightTarget);
    }
    CL_BREAK_IF(nodeRight->outNodes.size() > 0);

    // FIXME: this causes a USE_AFTER_FREE in bindVarList() on the line where
    //        we check hasKey(target->variables, i) in case we are called from
    //        there via joinFixPointS(), see pt-1204.c for a reproducer
    delete nodeRight;

    // the graph should be OK again
    CL_BREAK_IF(existsError(ctx.stor));

    rightTarget = NULL;
    BOOST_FOREACH(TNodePair &p, pairs) {
        if (p.first != nodeRight) {
            // handle in-coming to nodeRight
            CL_BREAK_IF(p.second != nodeRight);
            // simply add reference from p->first to nodeLeft even if it would
            // be a nodeLeft itself.
            addEdge(p.first, nodeLeft);
            continue;
        }

        // handle out-coming (at most one out-coming edge
        CL_BREAK_IF(rightTarget);
        rightTarget = p.second;
    }

    if (!rightTarget)
        // we are done for now..
        return;

    leftTarget = getOutputS(nodeLeft);

    if (rightTarget == nodeRight) {
        // there was self loop before
        if (!leftTarget) {
            // no output on the left node, just add the self-loop edge
            addEdge(nodeLeft, nodeLeft);
            return;
        }

        // note that following does nothing if leftNode == leftTarget
        ctx.joinTodo.push_back(std::make_pair(nodeLeft, leftTarget));
        return;
    }

    if (!leftTarget) {
        addEdge(nodeLeft, rightTarget);
        return;
    }

    // well, we have successfully joined two nodes, but we have to look if the
    // following pair of nodes is not possible to join also
    ctx.joinTodo.push_back(std::make_pair(leftTarget, rightTarget));
}

const Node *existsUid(const Graph &graph, int uid)
{
    const TMap &map = graph.map;
    TMap::const_iterator it = map.find(uid);

    if (it == map.end())
        return NULL;

    return it->second;
}

const Node *existsVar(const Graph &graph, const Var *v)
{
    return existsUid(graph, v->uid);
}

const Node *existsItem(const Graph &graph, const Item *i)
{
    return existsUid(graph, i->uid());
}

Node *findNode(Graph &ptg, int uid)
{
    if (existsUid(ptg, uid))
        return ptg.map[uid];
    return 0;
}

Node *findNode(Graph &ptg, const Var *v)
{
    if (existsVar(ptg, v))
        return ptg.map[v->uid];
    return 0;
}

Node *findNode(Graph &ptg, const Item *i)
{
    int uid = i->uid();
    if (existsUid(ptg, uid))
        return ptg.map[uid];
    return NULL;
}

Node *allocNodeForItem(Graph &ptg, const Item *i)
{
    int uid = i->uid();
    CL_BREAK_IF(hasKey(ptg.uidToItem, uid));

    ptg.uidToItem[uid] = i;

    Node *node = new Node();
    bindItem(ptg, node, i);

    return node;
}

// use this only for fnc. parameters
Node *getNode(Graph &ptg, const cl_operand &opFnc)
{
    int fncUid;
    if (!fncUidFromOperand(&fncUid, &opFnc))
        CL_BREAK_IF("bad operand");

    Node *node = findNode(ptg, fncUid);
    if (node)
        return node;

    Item *i = new Item(PT_ITEM_RET);
    i->data.fnc = ptg.fnc->stor->fncs[fncUid];

    return allocNodeForItem(ptg, i);
}

Node *getNode(Graph &ptg, const Var *v)
{
    Node *node = findNode(ptg, v);
    if (node)
        return node;
    Item *i = new Item(v);

    return allocNodeForItem(ptg, i);
}

Node *getNode(Graph &ptg, const Item *i)
{
    CL_BREAK_IF(!i);
    int uid = i->uid();
    CL_BREAK_IF(!hasKey(ptg.map, i->uid()));

    return ptg.map[uid];
}

Node *nodeFromForeign(Graph &ptg, const Item *ref)
{
    int uid = ref->uid();
    Node *n = findNode(ptg, uid);
    if (n)
        return n;

    // this uid is not yet in ptg

    Item *it = new Item(ref->code);
    it->data = ref->data;

    return allocNodeForItem(ptg, it);
}

void setBlackHole(Graph &ptg, Node *n)
{
    ptg.blackHole = n;
    n->isBlackHole = true;
}

void appendNodeS(BuildCtx &ctx, Graph&, Node *parent, Node *what)
{
    Node *target = getOutputS(parent);
    if (target)
        // plan the joining of parent's output if there exists output edge
        ctx.joinTodo.push_back(std::make_pair(target, what));
    else
        // it is safe to just append 'which' (do not plan anything)
        addEdge(parent, what);
}

Node *getOutputS(Node *node)
{
    int outCnt = node->outNodes.size();
    assert(outCnt <= 1);
    return outCnt ? *node->outNodes.begin() : NULL;
}

const Node *hasOutputS(const Node *node)
{
    return getOutputS(const_cast<Node *>(node));
}

Node *appendEmptyS(Node *source)
{
    CL_BREAK_IF(source->outNodes.size() > 0);
    Node *target = new Node;
    addEdge(source, target);
    return target;
}

Node *nodeAccessS(
        const CodeStorage::Storage     &stor,
        Graph                          &ptg,
        const cl_operand               &op,
        bool                           *referenced)
{
    const Var *v = &stor.vars[op.data.var->uid];
    Node *root = getNode(ptg, v);
    if (referenced)
        *referenced = false;

    // process all possible accessors of given operand to find/create correct
    // node in Graph
    struct cl_accessor * ac = op.accessor;
    for (; ac; ac = ac->next) {
        size_t size;

        switch (ac->code) {
            case CL_ACCESSOR_DEREF:
                size = root->outNodes.size();
                CL_BREAK_IF(size > 1);
                if (size != 0)
                    // there is output edge already
                    root = *root->outNodes.begin();
                else
                    root = appendEmptyS(root);
                break;
            case CL_ACCESSOR_ITEM:
            case CL_ACCESSOR_OFFSET:
            case CL_ACCESSOR_DEREF_ARRAY:
                // just skip this accessor and continue with the next one
                continue;
            case CL_ACCESSOR_REF:
                if (referenced)
                    *referenced = true;
                return root;
                break;
        }
    }

    return root;
}

const char *getOpStrCstValue(const cl_operand &op)
{
    CL_BREAK_IF(op.code != CL_OPERAND_CST);
    CL_BREAK_IF(op.data.cst.code != CL_TYPE_STRING);
    return op.data.cst.data.cst_string.value;
}

bool ptHandlerPlot(BuildCtx &ctx, const Insn &insn)
{
    if (insn.operands.size() == 2) {
        plotGraph(ctx.stor);
        return true;
    }

    CL_BREAK_IF(insn.operands.size() != 3);

    const cl_operand &param = insn.operands[2];
    plotGraph(ctx.stor, getOpStrCstValue(param));
    return true;
}

bool follows(const Node *a, const Node *b)
{
    typedef WorkList<const Node *> TWL;
    TWL wl(a);

    const Node *processed;
    while (wl.next(processed)) {
        BOOST_FOREACH(const Node *next, processed->outNodes) {
            if (next == b)
                return true;
            wl.schedule(next);
        }
    }

    return false;
}

bool follows(const Graph &ptg, const Var *varA, const Var *varB)
{
    const Node *left = existsVar(ptg, varA);
    const Node *right = existsVar(ptg, varB);
    if (!left || !right)
        return false;

    return follows(left, right);
}

bool followsGlobal(
        const CodeStorage::Storage     &stor,
        const Var                      *lVar,
        const Var                      *rVar)
{
    BOOST_FOREACH(Fnc *fnc, stor.fncs)
        if (follows(fnc->ptg, lVar, rVar))
            return true;

    // search global PT-graph
    return follows(stor.ptd.gptg, lVar, rVar);
}

bool isPointed(const Graph &ptg, const Var *v)
{
    const Node *target = existsVar(ptg, v);
    if (!target)
        return false;

    BOOST_FOREACH(const Node *pointer, target->inNodes) {
        if (pointer == target) {
            // self loop on this node -- is there any other variable in this
            // node?
            BOOST_FOREACH(const Item *other, pointer->variables) {
                switch (other->code) {
                    case PT_ITEM_RET:
                        return true;
                    case PT_ITEM_VAR:
                        if (other->data.var != v)
                            return true;
                        continue;
                    case PT_ITEM_MALLOC:
                        return true;
                }
            }
            continue;
        }

        // there exists another node pointing to us
        return true;
    }

    // no pointer to this variable exists
    return false;
}

bool isPointedGlob(const CodeStorage::Storage &stor, const Var *var)
{
    BOOST_FOREACH(const Fnc *fnc, stor.fncs)
        if (isPointed(fnc->ptg, var))
            return true;
    return isPointed(stor.ptd.gptg, var);
}

Node *goDownS(Node *start, int steps)
{
    Node *node = start;
    while (steps > 0) {
        if (!hasOutputS(node))
            appendEmptyS(node);

        CL_BREAK_IF(node->outNodes.size() != 1);
        node = getOutputS(node);
        steps--;
    }

    return node;
}

static void ptParseOpts(BuildCtx &ctx, const char *ptr)
{
    CL_BREAK_IF(!ptr);
    char *copy = strdup(ptr);

    while (copy && *copy != 0) {
        const char *option = copy;
        copy = strchr(copy, ',');
        if (copy) {
            *copy = 0;
            copy++;
        }

        if (STREQ(option, "plot-changes")) {
            PT_DEBUG(0, "Request for plotting PT-graph when graph changed.");
            ctx.plot.progress = "points-to-progress";
        }
        else
            PT_ERROR("Bad argument '" << option << "'");
    }
}

void chkOutNodes(const Graph &, const Node *n)
{
    const TNodeList &outNodes = n->outNodes;

    int size = outNodes.size();
    CL_BREAK_IF(size > 1);

    if (size == 0)
        return;

    const Node *outNode = *outNodes.begin();
    CL_BREAK_IF(!outNode);

    bool found = false;
    BOOST_FOREACH(const Node *inNode, outNode->inNodes) {
        if (inNode != n)
            continue;

        if (found)
            CL_BREAK_IF("double in-node");

        found = true;
    }

    CL_BREAK_IF(!found);
}

void chkInNodes(const Graph &, const Node *n)
{
    const TNodeList &inNodes = n->inNodes;

    BOOST_FOREACH(const Node *inNode, inNodes) {
        assert(hasOutputS(inNode));
        assert(hasOutputS(inNode) == n);
        (void)inNode;
    }
}

void chkVariables(const Graph &g, const TItemList &itemList, const Node *node)
{
    BOOST_FOREACH(const Item *it, itemList) {
        assert(hasKey(g.map, it->uid()));
        assert(existsUid(g, it->uid()) == node);
        (void)it;
        (void)node;
    }
    (void)g;
}

/**
 * Just check if the graph consistency is OK
 */
void chkGraphS(const Graph &g)
{
    WorkList<const Node *> wl;
    BOOST_FOREACH(TMap::const_reference pair, g.map) {
        wl.schedule(pair.second);
    }

    const Node *handled;
    while (wl.next(handled)) {

        CL_BREAK_IF(handled->isBlackHole != 0 && handled->isBlackHole != 1);

        chkVariables(g, handled->variables, handled);
        chkOutNodes(g, handled);
        chkInNodes(g, handled);

        const Node *outNode = hasOutputS(handled);
        if (outNode)
            wl.schedule(outNode);
    }
}

/**
 * this is made just to allow easier debugging
 */
bool existsError(const Storage &stor)
{
    BOOST_FOREACH(const Fnc *fnc, stor.fncs) {
        chkGraphS(fnc->ptg);
    }

    chkGraphS(stor.ptd.gptg);

    return false;
}

} /* namespace PointsTo */

void pointsToAnalyse(Storage &stor, const std::string &conf)
{
    StopWatch watch;

    PointsTo::BuildCtx ctx(stor);
    ptParseOpts(ctx, conf.c_str());

    if (stor.callGraph.hasCallback || stor.callGraph.hasIndirectCall) {
        stor.ptd.dead = true;
        PT_ERROR("points-to analyse requires correct call graph");
        goto done;
    }
    // FICS only for now
    if (!PointsTo::runFICS(ctx)) {
        stor.ptd.dead = true;
    }

done:
    CL_DEBUG("pointsToAnalyse() took " << watch);
}

} // namespace CodeStorage
