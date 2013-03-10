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
 * directed acyclic graph of the symbolic execution trace, see namespace Trace
 */

#include "config.h"

#include "id_mapper.hh"
#include "symbt.hh"                 // needed for EMsgLevel
#include "symheap.hh"               // needed for EObjKind

#include <vector>
#include <string>

struct cl_loc;

class SymState;

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

// TODO: should we use a more generic ID type?
typedef IdMapper<TObjId, OBJ_INVALID, OBJ_MAX_ID>   TIdMapper;

/// an abstract base for Node and NodeHandle (externally not much useful)
class NodeBase {
    protected:
        /// list of all (0..n) parent nodes
        TNodeList parents_;

        /// this is an abstract class, its instantiation is @b not allowed
        NodeBase() { }

        /// construct Node with exactly one parent, can be extended later
        NodeBase(Node *node):
            parents_(1, node)
        {
        }

    public:
        /// force virtual destructor
        virtual ~NodeBase();

        /// this can be called only on nodes with exactly one parent
        Node* parent() const;

        /// reference to list of parents (containing 0..n pointers)
        const TNodeList& parents() const { return parents_; }
};

/// an abstract node of the symbolic execution trace graph
class Node: public NodeBase {
    private:
        /// birth notification from a child node
        void notifyBirth(NodeBase *child);

        /// death notification from a child node
        void notifyDeath(NodeBase *child);

        friend class NodeBase;
        friend class NodeHandle;

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

        /// serialize this node to the given plot (externally not much useful)
        void virtual plotNode(TracePlotter &) const = 0;

        friend void plotTraceCore(TracePlotter &);

    public:
        /// used to store a list of child nodes
        typedef std::vector<NodeBase *> TBaseList;

        /// reference to list of child nodes (containing 0..n pointers)
        const TBaseList& children() const { return children_; }

        /// print the node in a human-readable format if considered interesting
        virtual Node* /* selected predecessor */ printNode() const {
            return this->parent();
        }

    public:
        /// return the ID mapping describing the operation behind the trace node
        TIdMapper& idMapper()             { return idMapper_; }

        /// return the ID mapping describing the operation behind the trace node
        const TIdMapper& idMapper() const { return idMapper_; }

    private:
        // copying NOT allowed
        Node(const Node &);
        Node& operator=(const Node &);

    protected:
        TIdMapper idMapper_;

    private:
        TBaseList children_;
};

/// useful to prevent a trace sub-graph from being destroyed too early
class NodeHandle: public NodeBase {
    public:
        /// initialize the handle with the given node, can be reset later
        NodeHandle(Node *ref):
            NodeBase(ref)
        {
            ref->notifyBirth(this);
        }

        /// return the node stored within this handle
        Node* node() const {
            return this->parent();
        }

        /// release the old node and re-initialize the handle with the new one
        void reset(Node *);

        /// overridden copy constructor keeping the semantics of a handle
        NodeHandle(const NodeHandle &tpl):
            NodeBase(tpl.node())
        {
            this->parent()->notifyBirth(this);
        }

        /// overridden assignment operator keeping the semantics of a handle
        NodeHandle& operator=(const NodeHandle &tpl) {
            this->reset(tpl.node());
            return *this;
        }
};

/// used to explicitly highlight trace graph nodes that should not be reachable
class TransientNode: public Node {
    public:
        /// @param origin describe where the unreachable node originates from
        TransientNode(const char *origin):
            origin_(origin)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;

    private:
        const char *origin_;
};

/// root node of the trace graph (usually a call of the root function)
class RootNode: public Node {
    private:
        const TFnc rootFnc_;

    public:
        /// @param rootFnc a CodeStorage::Fnc object used for the root call
        RootNode(const TFnc rootFnc):
            rootFnc_(rootFnc)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a non-terminal instruction
class InsnNode: public Node {
    private:
        const TInsn insn_;
        const bool  isBuiltin_;

    public:
        /**
         * @param ref a reference to a trace leading to this instruction
         * @param insn a CodeStorage::Insn object representing the instruction
         * @param isBuiltin true, if the instruction is recognized as a built-in
         */
        InsnNode(Node *ref, TInsn insn, const bool isBuiltin):
            Node(ref),
            insn_(insn),
            isBuiltin_(isBuiltin)
        {
            idMapper_.setNotFoundAction(TIdMapper::NFA_RETURN_IDENTITY);
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a conditional insn being traversed
class CondNode: public Node {
    private:
        const TInsn inCmp_;
        const TInsn inCnd_;
        const bool determ_;
        const bool branch_;

    public:
        /**
         * @param ref a reference to a trace leading to this instruction
         * @param inCmp a comparison instruction occurring prior to inCnd
         * @param inCnd a conditional jump instruction being traversed
         * @param determ true if the branch being taken was known in advance
         * @param branch true if the 'then' branch was taken, false for 'else'
         */
        CondNode(Node *ref, TInsn inCmp, TInsn inCnd, bool determ, bool branch):
            Node(ref),
            inCmp_(inCmp),
            inCnd_(inCnd),
            determ_(determ),
            branch_(branch)
        {
            idMapper_.setNotFoundAction(TIdMapper::NFA_RETURN_IDENTITY);
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a @b single abstraction step
class AbstractionNode: public Node {
    private:
        const EObjKind              kind_;
        std::string                 name_;

    public:
        /**
         * @param ref a trace leading to this abstraction step
         * @param kind the kind of abstraction step being performed
         */
        AbstractionNode(Node *ref, EObjKind kind):
            Node(ref),
            kind_(kind)
        {
        }

        /// @param name name of the corresponding debug plot (empty if unused)
        void setPlotName(const std::string &name) {
            name_ = name;
        }

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a @b single concretization step
class ConcretizationNode: public Node {
    private:
        const EObjKind              kind_;
        const std::string           name_;

    public:
        /**
         * @param ref a trace leading to this concretization step
         * @param kind the kind of concretization step being performed
         * @param name name of the corresponding debug plot (empty if unused)
         */
        ConcretizationNode(Node *ref, EObjKind kind, const std::string &name):
            Node(ref),
            kind_(kind),
            name_(name)
        {
        }

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a @b single splice-out operation
class SpliceOutNode: public Node {
    private:
        const bool              len_;

    public:
        /**
         * @param ref a trace leading to this splice-out operation
         * @param len count of successfully spliced-out segments
         */
        SpliceOutNode(Node *ref, const int len = 1):
            Node(ref),
            len_(len)
        {
            idMapper_.setNotFoundAction(TIdMapper::NFA_RETURN_IDENTITY);
        }

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a @b single join operation
class JoinNode: public Node {
    public:
        /// takes references to both traces being joined by this operation
        JoinNode(Node *ref1, Node *ref2):
            Node(ref1, ref2)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph nodes inserted automatically per each SymHeap clone operation
class CloneNode: public Node {
    public:
        CloneNode(Node *ref):
            Node(ref)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph node representing a call entry point
class CallEntryNode: public Node {
    private:
        const TInsn insn_;

    public:
        /**
         * @param ref trace representing the call entry as seen by the @b caller
         * @param insn a CodeStorage::Insn object representing the call
         */
        CallEntryNode(Node *ref, const TInsn insn):
            Node(ref),
            insn_(insn)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph node representing a cached call result
class CallCacheHitNode: public Node {
    private:
        const TFnc fnc_;

    public:
        /**
         * @param entry trace representing the call cache entry
         * @param result trace representing a cached call result (without frame)
         * @param fnc a CodeStorage::Fnc fld representing the called function
         */
        CallCacheHitNode(Node *entry, Node *result, const TFnc fnc):
            Node(entry, result),
            fnc_(fnc)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph node representing a call frame
class CallFrameNode: public Node {
    private:
        const TInsn insn_;

    public:
        /**
         * @param ref trace representing the call entry as seen by the @b caller
         * @param insn a CodeStorage::Insn object representing the call
         */
        CallFrameNode(Node *ref, const TInsn insn):
            Node(ref),
            insn_(insn)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph node representing a call result
class CallDoneNode: public Node {
    private:
        const TFnc fnc_;

    public:
        /**
         * @param result trace representing the result as seen by the @b callee
         * @param fnc a CodeStorage::Fnc fld representing the called function
         */
        CallDoneNode(Node *result, const TFnc fnc):
            Node(result),
            fnc_(fnc)
        {
        }

        /**
         * @param result trace representing the result as seen by the @b callee
         * @param callFrame trace representing the call frame of the call
         * @param fnc a CodeStorage::Fnc fld representing the called function
         */
        CallDoneNode(Node *result, Node *callFrame, const TFnc fnc):
            Node(result, callFrame),
            fnc_(fnc)
        {
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// a trace graph node that represents a @b single call of importGlVar()
class ImportGlVarNode: public Node {
    private:
        const std::string           varString_;

    public:
        /**
         * @param ref a trace leading to this concretization step
         * @param varString describing the global variable being imported
         */
        ImportGlVarNode(Node *ref, const std::string &varString):
            Node(ref),
            varString_(varString)
        {
        }

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph node representing an error/warning message
class MsgNode: public Node {
    private:
        const EMsgLevel     level_;
        const TLoc          loc_;

    public:
        /**
         * @param ref a trace leading to this error/warning message
         * @param level classification of the message (error, warning, ...)
         * @param loc a location info the message was emitted with
         */
        MsgNode(Node *ref, const EMsgLevel level, const TLoc loc):
            Node(ref),
            level_(level),
            loc_(loc)
        {
            idMapper_.setNotFoundAction(TIdMapper::NFA_RETURN_IDENTITY);
        }

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// trace graph node representing something important for the user
class UserNode: public Node {
    private:
        const TInsn         insn_;
        const std::string   label_;

    public:
        /**
         * @param ref a trace leading to this use node
         * @param insn a CodeStorage::Insn object that caused the node to appear
         * @param label a label describing what this node actually means
         */
        UserNode(Node *ref, const TInsn insn, const char *label):
            Node(ref),
            insn_(insn),
            label_(label)
        {
            idMapper_.setNotFoundAction(TIdMapper::NFA_RETURN_IDENTITY);
        }

        virtual Node* printNode() const;

    protected:
        void virtual plotNode(TracePlotter &) const;
};

/// plot a trace graph named "name-NNNN.dot" leading to the given node
bool plotTrace(Node *endPoint, const std::string &name, std::string *pName = 0);

/// print a human-readable trace using the Code Listener messaging API
void printTrace(Node *endPoint);

/// this runs in the debug build only
bool chkTraceGraphConsistency(Node *const from);

/// a container maintaining a set of trace graph end-points
class EndPointConsolidator {
    public:
        EndPointConsolidator();
        ~EndPointConsolidator();

        /// insert a new trace graph end-point
        bool /* any change */ insert(Node *);

        /// plot a common graph leading to all end-points inside the container
        bool plotAll(const std::string &name);

    private:
        // copying NOT allowed
        EndPointConsolidator(const EndPointConsolidator &);
        EndPointConsolidator& operator=(const EndPointConsolidator &);

    private:
        struct Private;
        Private *d;
};

/// a container maintaining a set of trace graphs being built on the fly
class GraphProxy {
    public:
        GraphProxy();
        ~GraphProxy();

        /// insert a new trace graph end-point into the specified trace graph
        bool /* any change */ insert(Node *, const std::string &graphName);

        /// plot the specified graph
        bool plotGraph(const std::string &name);

        /// plot all graphs being maintained by this proxy
        bool plotAll();

    private:
        // copying NOT allowed
        GraphProxy(const GraphProxy &);
        GraphProxy& operator=(const GraphProxy &);

    private:
        struct Private;
        Private *d;
};

/// a singleton holding global GraphProxy (may be extended later)
class Globals {
    private:
        GraphProxy              glProxy_;
        static Globals         *inst_;

    public:
        static bool alive() {
            return !!inst_;
        }

        static Globals* instance() {
            return (alive())
                ? (inst_)
                : (inst_ = new Globals);
        }

        GraphProxy* glProxy() {
            return &glProxy_;
        }

        static void cleanup() {
            delete inst_;
            inst_ = 0;
        }
};

/// mark the just completed @b clone operation as @b intended and unimportant
void waiveCloneOperation(SymHeap &sh);

/// mark the just completed @b clone operation as @b intended and unimportant
void waiveCloneOperation(SymState &);

} // namespace Trace

#endif /* H_GUARD_SYM_TRACE_H */
