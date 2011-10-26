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

struct cl_loc;

namespace CodeStorage {
    struct Fnc;
    struct Insn;
}

/// directed acyclic graph of the symbolic execution trace
namespace Trace {

class Node;

struct TracePlotter;

typedef const struct cl_loc                        *TLoc;
typedef const CodeStorage::Fnc                     *TFnc;
typedef const CodeStorage::Insn                    *TInsn;

typedef std::vector<Node *>                         TNodeList;

/// an abstract node of the symbolic execution trace graph
class Node {
    public:
        /// birth notification from a child node
        void notifyBirth(Node *child);

        /// death notification from a child node
        void notifyDeath(Node *child);

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
            ref->notifyBirth(this);
        }

        /// constructor for nodes with exactly two parents
        Node(Node *ref1, Node *ref2):
            parents_(1, ref1)
        {
            parents_.push_back(ref2);
            ref1->notifyBirth(this);
            ref2->notifyBirth(this);
        }

    public:
        virtual ~Node();
        void virtual plotNode(TracePlotter &) const = 0;

    public:
        const TNodeList& parents()      const { return parents_;    }
        const TNodeList& children()     const { return children_;   }
};

/// useful to prevent a trace sub-graph from being destroyed too early
class NodeHandle: public Node {
    public:
        NodeHandle(Node *ref):
            Node(ref)
        {
        }

        Node* node() const {
            return parents_.front();
        }

        void reset(Node *);

    private:
        // do not call these methods on NodeHandle (no children are allowed)
        void virtual plotNode(TracePlotter &) const;
        void notifyBirth(Node *);
        void notifyDeath(Node *);
};

// TODO: remove this
class NullNode: public Node {
    public:
        NullNode(const char *origin):
            origin_(origin)
        {
        }

        void virtual plotNode(TracePlotter &) const;

    private:
        const char *origin_;
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

        void virtual plotNode(TracePlotter &) const;
};

class InsnNode: public Node {
    private:
        const TInsn insn_;

    public:
        InsnNode(Node *ref, TInsn insn):
            Node(ref),
            insn_(insn)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

// FIXME: these nodes should not be created by default but only when debugging
class CloneNode: public Node {
    public:
        CloneNode(Node *ref):
            Node(ref)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class CallEntryNode: public Node {
    public:
        CallEntryNode(Node *ref):
            Node(ref)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class CallSurroundNode: public Node {
    public:
        CallSurroundNode(Node *ref):
            Node(ref)
        {
        }

        void virtual plotNode(TracePlotter &) const;
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

/// plot a trace graph named "symtrace-NNNN.dot" leading to the given node
bool plotTrace(Node *endPoint, TLoc loc = 0);

} // namespace Trace

#endif /* H_GUARD_SYM_TRACE_H */
