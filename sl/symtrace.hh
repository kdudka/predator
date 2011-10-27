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

#include "symbt.hh"                 // needed for EMsgLevel
#include "symheap.hh"               // needed for EObjKind

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

/// an abstract base for Node and NodeHandle (externally not much useful)
class NodeBase {
    protected:
        TNodeList parents_;

        /// this is an abstract class, its instantiation is @b not allowed
        NodeBase() { }

        NodeBase(Node *node):
            parents_(1, node)
        {
        }

    public:
        virtual ~NodeBase();

        /// this can be called only on nodes with exactly one parent
        Node* parent() const;

        /// reference to list of parents (containing 0..n pointers)
        const TNodeList& parents() const { return parents_; }
};

/// an abstract node of the symbolic execution trace graph
class Node: public NodeBase {
    public:
        /// birth notification from a child node
        void notifyBirth(NodeBase *child);

        /// death notification from a child node
        void notifyDeath(NodeBase *child);

    protected:
        /// this is an abstract class, its instantiation is @b not allowed
        Node() { }

        /// constructor for nodes with exactly one parent
        Node(Node *ref):
            NodeBase(ref)
        {
            ref->notifyBirth(this);
        }

        /// constructor for nodes with exactly two parents
        Node(Node *ref1, Node *ref2):
            NodeBase(ref1)
        {
            parents_.push_back(ref2);
            ref1->notifyBirth(this);
            ref2->notifyBirth(this);
        }

    public:
        /// serialize this node to the given plot (externally not much useful)
        void virtual plotNode(TracePlotter &) const = 0;

        /// used to store a list of children
        typedef std::vector<NodeBase *> TBaseList;

        /// reference to list of children (containing 0..n pointers)
        const TBaseList& children()     const { return children_;   }

    private:
        TBaseList children_;
};

/// useful to prevent a trace sub-graph from being destroyed too early
class NodeHandle: public NodeBase {
    public:
        NodeHandle(Node *ref):
            NodeBase(ref)
        {
            ref->notifyBirth(this);
        }

        Node* node() const {
            return this->parent();
        }

        void reset(Node *);
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
        const bool  isBuiltin_;

    public:
        InsnNode(Node *ref, TInsn insn, const bool isBuiltin):
            Node(ref),
            insn_(insn),
            isBuiltin_(isBuiltin)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class CondNode: public Node {
    private:
        const TInsn inCmp_;
        const TInsn inCnd_;
        const bool determ_;
        const bool branch_;

    public:
        CondNode(Node *ref, TInsn inCmp, TInsn inCnd, bool determ, bool branch):
            Node(ref),
            inCmp_(inCmp),
            inCnd_(inCnd),
            determ_(determ),
            branch_(branch)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class AbstractionNode: public Node {
    private:
        const EObjKind kind_;

    public:
        AbstractionNode(Node *ref, EObjKind kind):
            Node(ref),
            kind_(kind)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class ConcretizationNode: public Node {
    private:
        const EObjKind kind_;

    public:
        ConcretizationNode(Node *ref, EObjKind kind):
            Node(ref),
            kind_(kind)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class SpliceOutNode: public Node {
    private:
        const EObjKind          kind_;
        const bool              successful_;

    public:
        SpliceOutNode(Node *ref, const EObjKind kind, const bool successful):
            Node(ref),
            kind_(kind),
            successful_(successful)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class JoinNode: public Node {
    public:
        JoinNode(Node *ref1, Node *ref2):
            Node(ref1, ref2)
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
    private:
        const TInsn insn_;

    public:
        CallEntryNode(Node *ref, const TInsn insn):
            Node(ref),
            insn_(insn)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class CallSurroundNode: public Node {
    private:
        const TInsn insn_;

    public:
        CallSurroundNode(Node *ref, const TInsn insn):
            Node(ref),
            insn_(insn)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class CallDoneNode: public Node {
    private:
        const TFnc fnc_;

    public:
        CallDoneNode(Node *result, Node *surround, const TFnc fnc):
            Node(result, surround),
            fnc_(fnc)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

class MsgNode: public Node {
    private:
        const EMsgLevel     level_;
        const TLoc          loc_;

    public:
        MsgNode(Node *ref, const EMsgLevel level, const TLoc loc):
            Node(ref),
            level_(level),
            loc_(loc)
        {
        }

        void virtual plotNode(TracePlotter &) const;
};

/// plot a trace graph named "symtrace-NNNN.dot" leading to the given node
bool plotTrace(Node *endPoint, TLoc loc = 0);

} // namespace Trace

#endif /* H_GUARD_SYM_TRACE_H */
