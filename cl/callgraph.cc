/*
 * Copyright (C) 2012 Kamil Dudka <kdudka@redhat.com>
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
#include "callgraph.hh"

#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "stopwatch.hh"
#include "worklist.hh"

#include <boost/foreach.hpp>

namespace CodeStorage {

namespace CallGraph {

typedef const struct cl_operand            &TOp;
typedef const Insn                         *TInsn;

Node* allocNodeIfNeeded(Graph &cg, Fnc *const fnc)
{
    Node *&node = fnc->cgNode;
    if (node)
        return node;

    node = new Node(fnc);
    cg.roots.insert(node);
    cg.leaves.insert(node);
    return node;
}

void handleCallback(Graph &cg, Node *node, const TInsn insn, TOp op)
{
    int uid;
    if (!fncUidFromOperand(&uid, &op))
        // not a function
        return;

    // resolve a call-graph node for the callee
    Fnc *const targetFnc = insn->stor->fncs[uid];
    Node *const targetNode = allocNodeIfNeeded(cg, targetFnc);

    // (node == 0) means var initializer
    Fnc *const callerFnc = (node)
        ? node->fnc
        : 0;

    // append a callback to the _target_ node
    // FIXME: this can append a single instruction multiple times
    targetNode->callbacks[callerFnc].push_back(insn);

    // update globals
    cg.roots.erase(targetNode);
    cg.hasCallback = true;
}

void handleCall(Graph &cg, Node *node, const TInsn insn)
{
    // if there is a call, it is no longer a leaf node
    cg.leaves.erase(node);

    int uid;
    if (!fncUidFromOperand(&uid, &insn->operands[/* fnc */ 1])) {
        // indirect call
        node->calls[/* indirect calls */ 0].push_back(insn);
        cg.hasIndirectCall = true;
        return;
    }

    // resolve a call-graph node for the callee
    Fnc *const targetFnc = insn->stor->fncs[uid];
    Node *const targetNode = allocNodeIfNeeded(cg, targetFnc);

    // create a bi-directional call-graph edge
    node->calls[targetFnc].push_back(insn);
    targetNode->callers[node->fnc].push_back(insn);
    cg.roots.erase(targetNode);
}

void handleFnc(Fnc *const fnc)
{
    Graph &cg = fnc->stor->callGraph;
    Node *const node = allocNodeIfNeeded(cg, fnc);

    BOOST_FOREACH(const Block *bb, fnc->cfg) {
        BOOST_FOREACH(const TInsn insn, *bb) {
            const bool isCallInsn = (CL_INSN_CALL == insn->code);
            if (isCallInsn)
                handleCall(cg, node, insn);

            const TOperandList &opList = insn->operands;
            for (unsigned i = 0; i < opList.size(); ++i) {
                if (isCallInsn && (/* fnc */ 1 == i))
                    // this is a direct call, not a call-back
                    continue;

                handleCallback(cg, node, insn, opList[i]);
            }
        }
    }
}

typedef std::pair<std::string /* file */, int /* line */>       TLocKey;
typedef std::map<TLocKey, const Fnc *>                          TLocMap;

void insertFnc(TLocMap &lm, const Fnc *fnc)
{
    const struct cl_loc *loc = locationOf(*fnc);
    const TLocKey key(loc->file, loc->line);
    lm[key] = fnc;
}

void buildTopList(Graph &cg)
{
    typedef std::queue<const Node *> TSched;
    WorkList<const Node *, TSched> wl;

    TLocMap lmRoots;
    BOOST_FOREACH(const Node *rootNode, cg.roots)
        insertFnc(lmRoots, rootNode->fnc);

    BOOST_FOREACH(TLocMap::const_reference item, lmRoots)
        wl.schedule(item./* fnc */second->cgNode);

    const Node *node;
    while (wl.next(node)) {
        TLocMap lm;
        BOOST_FOREACH(TInsnListByFnc::const_reference item, node->calls) {
            const Fnc *callee = item.first;
            if (!callee)
                // ignore indirect calls
                continue;

            insertFnc(lm, callee);
        }

        BOOST_FOREACH(TLocMap::const_reference item, lm)
            wl.schedule(item./* fnc */second->cgNode);

        const Fnc *fnc = node->fnc;
        cg.topOrder.push_back(fnc);
    }
}

void buildCallGraph(const Storage &stor)
{
    StopWatch watch;

    BOOST_FOREACH(Fnc *fnc, stor.fncs)
        handleFnc(fnc);

    Graph &cg = const_cast<Graph &>(stor.callGraph);

    // dig callbacks from var initializers
    BOOST_FOREACH(const Var &var, stor.vars)
        BOOST_FOREACH(const TInsn insn, var.initials)
            BOOST_FOREACH(const TOp op, insn->operands)
                handleCallback(cg, /* node */ 0, insn, op);

    // construct topological order
    buildTopList(cg);

    CL_DEBUG("buildCallGraph() took " << watch);
}

} // namespace CallGraph

} // namespace CodeStorage
