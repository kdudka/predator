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

#ifndef H_GUARD_CL_PT_H
#define H_GUARD_CL_PT_H

#include "config_cl.h"

#include <cl/storage.hh>
#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>

extern int pt_dbg_level;

#define PT_DEBUG(level, ...) do {                                           \
    if ((level) <= pt_dbg_level)                                            \
        CL_DEBUG("PT: " << __VA_ARGS__);                                    \
} while (0)

// this does not force invocation of gcc error
#define PT_ERROR(...) PT_DEBUG(0, "ERROR: " << __VA_ARGS__)

namespace CodeStorage {
namespace PointsTo {

    typedef std::pair<Node*, Node*>     TNodePair;
    typedef std::vector<TNodePair>      TNodeJoinTodo;

// TODO: incorporate this in parameter parsing to allow easily turn some of the
// FICS phases off
#define FICS_PHASE_1 0x01
#define FICS_PHASE_2 0x02
#define FICS_PHASE_3 0x04

    // structure used for keeping building context among functions
    class BuildCtx {
        public:
            TNodeJoinTodo               joinTodo;
            CodeStorage::Storage       &stor;
            Graph                      *ptg;

            struct plot {
                // set this variable when you want to plot all points-to graphs
                // when some of them is changed.  Content of this variable will
                // be used as a base-name and extended by unique extension for
                // each graph.
                const char             *progress;
            } plot;

            // debugging-only info
            struct debug {
                // which phases are going to be processed (all by default)
                int phases;
            } debug;

            BuildCtx(Storage &stor_) :
                stor(stor_),
                ptg(NULL)
            {
                plot.progress = NULL; // disable by default
                debug.phases = FICS_PHASE_1 | FICS_PHASE_2 | FICS_PHASE_3;
            }
    };

    // Methods called by name ending with 'S' character are single output
    // methods (there is at most one output edge per node).  This shape of graph
    // is useful especially for FICS algorithm.  Note that other algorithms
    // (like Andersen's) can have more than one output edge per node.

    /**
     * Join two nodes: nodeA = nodeA JOIN nodeB
     *
     * This must always KEEP nodeA on the same place as-is (it may be referenced
     * by others) but nodeB is going to be deleted completely (there must be
     * guaranteed that nobody is pointing it before this operation).
     */
    void joinNodesS(
            BuildCtx                   &ctx,
            Graph                      &ptg,
            Node                       *nodeA,
            Node                       *nodeB);

    /**
     * Start the merging of nodes based on ctx.joinTodo information.  This
     * function uses joinNodesS() internally.
     */
    void joinFixPointS(
            BuildCtx                   &ctx,
            Graph                      &ptg);

    /**
     * allocate and append one (empty) successor for node if it has not other
     * output.
     */
    Node *preventEndingS(Node *node);

    /**
     * Append (even existing) node to other one as a successor.  Note that this
     * just add pair to ctx.joinTodo -- this requires to call joinFixPointS to
     * do the real joining part.
     */
    void appendNodeS(
            BuildCtx                   &ctx,
            Graph                      &ptg,
            Node                       *parent,
            Node                       *which);

    /**
     * Find or create node of graph based on whole information provided by some
     * operand.  Dereferences, field accesses, etc. are taken into account.
     *
     * @param referenced is set to true when the '&' reference operator was
     * detected in operand.
     */
    Node *nodeAccessS(
            const CodeStorage::Storage &stor,
            Graph                      &ptg,
            const cl_operand           &op,
            bool                       *referenced = NULL);

    /**
     * Similar to preventEndingS() except that it would fail if the single
     * output already existed.
     */
    Node *appendEmptyS(Node *root);

    /**
     * Return non-modifiable output node pointer if the output of 'node' exists.
     * Return NULL if the given node does not have an output.
     */
    const Node *hasOutputS(const Node *node);

    /**
     * Return modifiable output node pointer if the output of 'node' exists.
     * Return NULL if the given node does not have an output.
     */
    Node *getOutputS(Node *node);

    /**
     * go 'steps'-times downwards in node's graph.  Pre-allocate non-existing
     * nodes when needed.  Note that the cycles are not detected -- when you go
     * down in a loop no node will be allocated.  Return the desired node
     * pointer.
     */
    Node *goDownS(Node *start, int steps);

    /**
     * This is _blind_ function -- it does exactly what we want from it.  Even
     * if the given item 'i' already existed somewhere in the graph it will
     * bind the item (as a referenced duplicate) into the node 'n'
     */
    void bindItem(Graph &ptg, Node *n, const Item *);

    /**
     * bindVarList is more sophisticated function than 'bindVar'.  Before it
     * assigns some variable item to the node it checks whether this item does
     * not already exist somewhere else in target graph.  If yes, it performs
     * joining (joinFixPointS()) with the concurrent node.
     */
    bool bindVarList(
            BuildCtx                   &ctx,
            Graph                      &ptg,
            Node                       *target,
            const TItemList            &nl);

    /**
     * return true if some variable (lhs) follows another one (rhs)
     */
    bool follows(const Graph &ptg, const Var *varA, const Var *varB);
    bool followsGlobal(
            const CodeStorage::Storage &stor,
            const Var                  *lVar,
            const Var                  *rVar);

    /**
     * return true if given variable 'v' is pointed in given graph 'ptg'
     */
    bool isPointed(const Graph *ptg, const Var *v);

    /**
     * return true if the given variable 'v' is pointed in any points-to graph
     */
    bool isPointedGlob(const CodeStorage::Storage &stor, const Var *v);

    /**
     * find (or _create_ if doesn't exist!) node for given variable
     */
    Node *getNode(Graph &ptg, const Var *v);
    Node *getNode(Graph &ptg, const cl_operand &op);
    Node *getNode(Graph &ptg, const Item *i);

    /**
     * find or create node based on item from foreign graph
     */
    Node *nodeFromForeign(Graph &ptg, const Item *i);

    /**
     * allocate new node and bind i with this new node
     */
    Node *allocNodeForItem(Graph &, const Item *i);

    /**
     * return the node pointer according to passed 'uid' of variable or
     * function.
     */
    const Node *existsUid(const Graph &graph, int uid);

    /**
     * return pointer to node with given variable 'v'
     */
    const Node *existsVar(const Graph &graph, const Var *v);

    const Node *existsItem(const Graph &graph, const Item *i);


    /**
     * return pointer to (modifiable) node object according to given variable or
     * node item pointer.  Return NULL when the node doesn't exist.
     */
    Node *findNode(Graph &, int);
    Node *findNode(Graph &, const Var *);
    Node *findNode(Graph &, const Item *);

    /**
     * Setup the given node to be black-hole -- self-pointed node that eats each
     * other note that is set as its successor.
     */
    void setBlackHole(Graph &, Node *);

    /**
     * Interconnect two given nodes with edge
     */
    void addEdge(Node *from, Node *to);

    /**
     * return true when some problem occurs
     */
    bool existsError(const Storage &stor);
} /* namespace PointsTo */

void pointsToAnalyse(Storage &stor, const std::string &conf);

} /* namespace CodeStorage */

#endif /* H_GUARD_CL_PT_H */
