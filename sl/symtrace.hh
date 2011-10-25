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

#ifndef H_GUARD_SYM_TRACE_H
#define H_GUARD_SYM_TRACE_H

/**
 * @file symtrace.hh
 * @todo some dox
 */

#include "config.h"

#include <vector>

namespace CodeStorage {
    struct Fnc;
}

/// directed acyclic graph of the symbolic execution trace
namespace Trace {

typedef const CodeStorage::Fnc                     *TFnc;

class Node;
typedef std::vector<Node *>                         TNodeList;

/// an abstract node of the symbolic execution trace graph
class Node {
    private:
        TNodeList children_;

    protected:
        TNodeList parents_;

        /// this is an abstract class, its instantiation is @b not allowed
        Node() { }

        /// constructor for nodes with exactly one parent
        Node(Node *ref):
            parents_(1, ref)
        {
        }

        /// constructor for nodes with exactly two parents
        Node(Node *ref1, Node *ref2):
            parents_(1, ref1)
        {
            parents_.push_back(ref2);
        }

    public:
        virtual ~Node();
        const TNodeList& parents()      const { return parents_;    }
        const TNodeList& children()     const { return children_;   }

    public:
        /// birth notification from a child node
        void notifyBirth(Node *child);

        /// death notification from a child node
        void notifyDeath(Node *child);
};

/// useful to prevent a trace sub-graph from being destroyed too early
class NodeHandle: public Node {
    public:
        NodeHandle(Node *ref):
            Node(ref)
        {
            ref->notifyBirth(this);
        }

        Node* node() const {
            return parents_.front();
        }

        void notifyBirth(Node *);
        void notifyDeath(Node *);
};

// TODO: remove this
class NullNode: public Node {
};

/// root node of the trace graph (a call of the root function)
class RootNode: public Node {
    private:
        const TFnc rootFnc_;

    public:
        RootNode(const TFnc rootFnc):
            rootFnc_(rootFnc)
        {
        }

    public:
        // TODO
};

// FIXME: these nodes should not be created by default but only when debugging
class CloneNode: public Node {
    public:
        CloneNode(Node *ref):
            Node(ref)
        {
            ref->notifyBirth(this);
        }
};

class LinearNode: public Node {
    // ref
    // first insn
    // last insn
};

class DecisionNode: public Node {
    // ref
    // branch (true/false)
};

// TODO: move this one to symjoin.hh (missing declaration of EJoinStatus)
class JoinNode: public Node {
    // ref1
    // ref2
    // status (EJoinStatus)
};

class CallCacheHitNode: public Node {
    // ref - cache entry
    // ref - cached result (a particular heap)
};

class MsgNode: public Node {
    // ref
    // kind of message (warning, error)
    // insn
};

} // namespace Trace

#endif /* H_GUARD_SYM_TRACE_H */
