/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_STORAGE_H
#define H_GUARD_STORAGE_H

#include "code_listener.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#ifndef BUILDING_DOX
#   define STD_VECTOR(type) std::vector<type>
#else
    // this would cause a lot of syntax errors with any parser that takes C++
    // seriously but doxygen does not complain and produces way better output
#   define STD_VECTOR(type) type[]
#endif

/**
 * @file storage.hh
 * object model that describes the analyzed code on the input
 */

/**
 * object model that describes the analyzed code on the input
 */
namespace CodeStorage {

struct Insn;

/**
 * high-level variable (operand) classification
 */
enum EVar {
    VAR_VOID,           ///< this should be used only internally
    VAR_GL,             ///< global (may be also static) variable
    VAR_LC,             ///< local variable (not valid beyond a function)
    VAR_FNC_ARG         ///< fnc argument (sort of local variable also)
};

/**
 * high-level variable representation
 */
struct Var {
    EVar                        code;   ///< high-level kind of variable
    struct cl_loc               loc;    ///< location of its declaration

    /**
     * type of the variable
     * @note This often differs from type of the operand given to constructor!
     */
    const struct cl_type        *type;

    /**
     * unique ID of variable
     * @attention not guaranteed to be unique beyond the scope of variable
     */
    int                         uid;

    /**
     * name of the variable, empty string for anonymous variables
     */
    std::string                 name;

    /**
     * initializer (optional)
     */
    std::vector<const Insn *>   initials;

    /**
     * if true, the variable is initialized; in particular it means that whole
     * contents of the variable is nullified unless said otherwise (and yes,
     * even if initials is an empty list)
     *
     * @todo a better API for initializers?
     */
    bool                        initialized;

    /**
     * true if the variable is external (defined in another module)
     */
    bool                        isExtern;

    /**
     * true if there is at least one instruction in the program that takes an
     * address of the variable (or some part of it)
     */
    bool                        mayBePointed;

    /**
     * dummy constructor
     * @note known to be useful for internal purposes only
     */
    Var();
    ~Var();

    /**
     * wrap low-level operand to Var object
     * @param code kind of variable
     * @param op Pointer to low-level operand to be used for Var construction.
     */
    Var(EVar code, const struct cl_operand *op);
};

bool isOnStack(const Var &);

/**
 * lookup container for set of Var objects
 */
class VarDb {
    private:
        typedef STD_VECTOR(Var) TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        VarDb();
        ~VarDb();

        /**
         * look for a Var object by ID, add one if not found
         * @param uid ID of variable to look for
         * @return reference to either a found or just created Var object
         */
        Var& operator[](int uid);

        /**
         * look for a Var object by ID, crash if not found
         * @attention It is not safe to look for non-existing ID, it will jump
         * to debugger in that case.
         * @param uid ID of variable to look for
         * @return reference to the found Var object
         */
        const Var& operator[](int uid) const;

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator begin() const { return vars_.begin(); }

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator end()   const { return vars_.end();   }

        /**
         * return count of variables stored in the container
         */
        size_t size()          const { return vars_.size();  }

    private:
        /// @b not allowed to be copied
        VarDb(const VarDb &);

        /// @b not allowed to be copied
        VarDb& operator=(const VarDb &);

    private:
        TList vars_;
        struct Private;
        Private *d;
};

/**
 * type lookup table
 * @attention Type objects are not cloned for now (sort of exception).
 * @todo Check if there is a front-end which really needs types to be cloned.
 */
class TypeDb {
    private:
        typedef STD_VECTOR(const struct cl_type *) TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        TypeDb();
        ~TypeDb();

        /**
         * index given type for lookup
         * @note useful only for builder
         * @return true if the type was really added, false if it has been
         * already there
         */
        bool insert(const struct cl_type *);

        /**
         * type lookup by ID
         * @attention The pointer returned is the same as formerly given to
         * insert(). There is no cloning performed since it hasn't been
         * considered useful for now.
         */
        const struct cl_type* operator[](int) const;

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator begin() const { return types_.begin(); }

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator end()   const { return types_.end();   }

        /**
         * return count of types stored in the container
         */
        size_t size()          const { return types_.size();  }

        /**
         * value of sizeof(void (*)()) in the analysed program, -1 if such
         * information is not available
         */
        int codePtrSizeof() const;

        /**
         * value of sizeof(void *) in the analysed program, -1 if such
         * information is not available
         */
        int dataPtrSizeof() const;

        /// a (void *) type if available; if not, any data pointer; 0 otherwise
        const struct cl_type* genericDataPtr() const;

    private:
        /// @b not allowed to be copied
        TypeDb(const TypeDb &);

        /// @b not allowed to be copied
        TypeDb& operator=(const TypeDb &);

    private:
        TList types_;
        struct Private;
        Private *d;
};

/**
 * Add the given type into TypeDb instance, then descent into the type and add
 * all the referred types recursively.
 * @param db TypeDb object to add all types to
 * @param clt an arbitrary code listener type to be read
 */
void readTypeTree(TypeDb &db, const struct cl_type *clt);

/**
 * name to UID mapping for global/static symbols
 */
struct NameDb {
    typedef std::map<std::string, int /* uid */>    TNameMap;
    typedef std::map<std::string, TNameMap>         TFileMap;

    TNameMap        glNames;
    TFileMap        lcNames;
};

class Block;
class ControlFlow;
struct Storage;

/**
 * generic STL-based list of cl_operand objects.
 * They may or may not be deep-cloned, it depends on the particular purpose.
 */
typedef STD_VECTOR(struct cl_operand) TOperandList;

/**
 * generic STL-based list of Block pointers (useful to build CFG from Block
 * objects)
 */
typedef STD_VECTOR(const Block *) TTargetList;

/**
 * value type describing a single 'kill variable' hint
 */
struct KillVar {
    /// CodeStorage uid of the variable to kill
    int             uid;

    /// if true, killing the variable is safe only if nobody points at/inside it
    bool            onlyIfNotPointed;

    KillVar(int uid_, bool onlyIfNotPointed_):
        uid(uid_),
        onlyIfNotPointed(onlyIfNotPointed_)
    {
    }
};

/// FIXME: inserting <id,0> and <id,1> could break the consistency of std::set
struct ltKillVar {
    bool operator()(const KillVar &a, const KillVar &b) {
        return a.uid < b.uid;
    }
};

/// list of variables to kill (a @b kill @b list)
typedef std::set<KillVar, ltKillVar>                TKillVarList;

/// list of kill lists (one per each target of a terminal instruction)
typedef std::vector<TKillVarList>                   TKillPerTarget;

/**
 * high-level representation of an intermediate code instruction
 */
struct Insn {
    /**
     * instance of Storage which owns the Insn object
     */
    Storage                     *stor;

    /**
     * reference to a basic block that owns the Insn object
     */
    Block                       *bb;

    /**
     * kind of instruction, see #cl_insn_e documentation
     * @note now there can be also CL_INSN_CALL and CL_INSN_SWITCH
     */
    enum cl_insn_e              code;

    /**
     * some extra instructions partitioning, for now used by CL_INSN_UNOP and
     * CL_INSN_BINOP
     * @note this suffers from a little type info lost since it represent both
     * enumeral types #cl_unop_e and #cl_binop_e by an integer.
     */
    int                         subCode;

    /**
     * corresponding location in the original source code
     */
    struct cl_loc               loc;

    /**
     * List of all operands used by the instruction.
     * Their particular semantic is highly dependent on @b type @b of @b the
     * @b instruction. Let's go to summarize it:
     *
     * - (@b 0 operands) @b CL_INSN_JMP
     *
     * - (@b 1 operands) @b CL_INSN_COND:
     *      - [@b 0] - value to branch by
     *
     * - (@b 1 operands) @b CL_INSN_RET:
     *      - [@b 0] - value to return from fnc
     *
     * - (@b 0 operands) @b CL_INSN_ABORT
     *
     * - (@b 2 operands) @b CL_INSN_UNOP:
     *      - [@b 0] - destination
     *      - [@b 1] - source
     *
     * - (@b 3 operands) @b CL_INSN_BINOP:
     *      - [@b 0] - destination
     *      - [@b 1] - source 1
     *      - [@b 2] - source 2
     *
     * - (@b 2+ operands) @b CL_INSN_CALL:
     *      - [@b 0] - destination
     *      - [@b 1] - what are we going to call (mostly a fnc)
     *      - [@b 2..@b n] - call arguments
     *
     * - (@b 1+ operands) @b CL_INSN_SWITCH:
     *      - [@b 0] - value to switch by
     *      - [@b 1..@b n] - case values
     *
     * - (@b 1 operand) @b CL_INSN_LABEL:
     *      - [@b 0] - name of the label
     */
    TOperandList                operands;

    /// list of variables you can safely kill @b after execution of the insn
    TKillVarList                varsToKill;

    /**
     * similar to varsToKill, but refined for @b terminal instructions with more
     * than one target.  List of kill lists, one kill list per each target of
     * the terminal instruction.  If a variable is already killed by varsToKill,
     * it does not appear in killPerTarget again.
     */
    TKillPerTarget              killPerTarget;

    /**
     * List of all target blocks - useful only for @b terminal @b instructions.
     * Their particular semantic is highly dependent on @b type @b of @b the
     * @b instruction. Let's go to summarize it:
     *
     * - (@b 1 target) @b CL_INSN_JMP:
     *      - [@b 0] - target of jump
     *
     * - (@b 2 targets) @b CL_INSN_COND:
     *      - [@b 0] - where to jump if condition holds
     *      - [@b 1] - where to jump if condition does not hold
     *
     * - (@b 0 targets) @b CL_INSN_RET
     *
     * - (@b 0 targets) @b CL_INSN_ABORT
     *
     * - (@b 1+ targets) @b CL_INSN_SWITCH:
     *      - [@b 0] - where to jump by default
     *      - [@b 1..@b n] - where to jump in corresponding cases
     */
    TTargetList                 targets;

    /// list of indexes of targets that are closing a loop at the CFG level
    std::vector<unsigned>       loopClosingTargets;
};

/**
 * Basic block - a single node in ControlFlow graph. Once the basic block is
 * ready, it contains (possibly empty) sequence of non-terminating instructions
 * and exactly one terminating instruction.
 */
class Block {
    private:
        typedef STD_VECTOR(const Insn *) TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        /**
         * constructor useful to place objects into std::vector, but do @b not
         * try to call append() on objects constructed this way. It would crash
         * on a NULL pointer dereference.
         */
        Block():
            cfg_(0)
        {
        }

        /**
         * created a named basic block
         * @param cfg pointer to control flow graph where the block belongs to
         * @param name name of the basic block being constructed (zero ended C
         * string)
         */
        Block(ControlFlow *cfg, const char *name):
            cfg_(cfg),
            name_(name)
        {
        }

        // NOTE: there is no destructor ... given objects are NOT destroyed

        /**
         * return name of the basic block
         */
        const std::string& name() const {
            return name_;
        }

        /**
         * append a given instruction to end of the block
         * @param insn Instruction to append.
         * @note Given objects are not cloned nor destroyed!
         */
        void append(Insn *insn);

        void appendPredecessor(Block *);

        /**
         * return list of all direct successors
         */
        const TTargetList& targets() const;

        /**
         * return list of all direct predecessors
         */
        const TTargetList& inbound()         const { return inbound_; }

        /**
         * return STL-like iterator to go through all the instructions inside
         */
        const_iterator begin()               const { return insns_.begin(); }

        /**
         * return STL-like iterator to go through all the instructions inside
         */
        const_iterator end()                 const { return insns_.end();   }

        /**
         * return the first instruction in the basic block
         */
        const Insn* front() const;

        /**
         * return the last instruction in the basic block
         * @note This should be a @b terminal instruction once the CodeStorage
         * model has been built.
         */
        const Insn* back() const;

        /**
         * return count of instructions inside the basic block
         */
        size_t size()                        const { return insns_.size();  }

        /**
         * direct access to instruction by its index.
         * @param idx Index of instruction to access (staring with zero).
         * @attention There is no range check performed.
         */
        const Insn* operator[](unsigned idx) const { return insns_[idx];    }

        /**
         * return the ControlFlow object which the Block belongs to
         */
        const ControlFlow* cfg() const             { return cfg_;           }

        /// return true, if a loop at the level of CFG starts with this block
        bool isLoopEntry() const;

    private:
        TList insns_;
        TTargetList inbound_;
        ControlFlow *cfg_;
        std::string name_;
};

/**
 * Control flow graph - an easy to analyse representation of the intermediate
 * code. Nodes of the graph are basic blocks - instances of Block
 * @todo support for (non-recursive) call graph unfolding?
 */
class ControlFlow {
    private:
        typedef STD_VECTOR(Block *) TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        ControlFlow();
        ~ControlFlow();
        ControlFlow(const ControlFlow &);               ///< shallow copy
        ControlFlow& operator=(const ControlFlow &);    ///< shallow copy

        /**
         * return entry basic block
         */
        const Block *entry() const;

        /**
         * look for a basic block by name, create one if not found
         * @param name name of the basic block to look for
         * @return referenced pointer to either a found or just created Block
         * @attention created objects will @b not be destroyed automatically
         */
        Block*& operator[](const char *name);

        /**
         * look for a basic block by name, crash if not found
         * @attention It is not safe to look for a non-existing basic block, it
         * will jump to debugger in that case.
         * @param name name of the basic block to look for
         * @return pointer to the found Var object
         */
        const Block* operator[](const char *name) const;

        /**
         * return STL-like iterator to go through all basic blocks inside
         */
        const_iterator begin()                const { return bbs_.begin(); }

        /**
         * return STL-like iterator to go through all basic blocks inside
         */
        const_iterator end()                  const { return bbs_.end();   }

        /**
         * return count of basic blocks inside the control flow graph
         */
        size_t size()                         const { return bbs_.size();  }

    private:
        TList bbs_;
        struct Private;
        Private *d;
};

typedef std::vector<int>        TArgByPos;
typedef std::set<int>           TVarSet;

namespace CallGraph {
    struct Node;
}

struct Fnc;

namespace PointsTo {

class Node;

typedef enum {
    PT_ITEM_VAR = 0,
    PT_ITEM_RET,
    PT_ITEM_MALLOC
} ItemCodeE;

class Item {
    public:
        Item(ItemCodeE);
        Item(const Var *v);

        bool isGlobal() const;
        int uid() const;
        const char *name() const;
        /**
         * return CodeStorage::Var pointer if the item represents PT_ITEM_VAR
         * variable, otherwise return NULL.
         */
        const Var *var() const;

    public:
        ItemCodeE code;

        union {
            // NULL for black-hole
            const Var *var;
            // valid only for ITEM_RET
            const Fnc *fnc;
            // valid only for PT_ITEM_MALLOC
            int mallocId;
        } data;

    private:
        Item();
};

typedef std::vector<const Item *>       TItemList;
typedef std::set<Node *>                TNodeList;

class Node {
    public:
        Node();
       ~Node();

    public:
        /// list of variables assigned to the node
        TItemList                       variables;
        /// out-edges edges are stored inside
        TNodeList                       outNodes;
        /// in-edges are just 'synced' pointers
        TNodeList                       inNodes;
        /// there should be only one black-hole / graph
        bool                            isBlackHole;
};

// In some types of PT-graphs (e.g. graph constructed by FICS algorithm) we can
// expect that one variable may be placed at most in one node of graph
// structure -- actual implementation covers this situation.
// FIXME: this should be generalized for graphs where the same variable may be
//        placed in more than one node of points-to graph.
typedef std::map<int, Node *> TMap;

class Graph {
    public:
        Graph(Fnc *fnc_ = 0) :
            fnc(fnc_),
            blackHole(0)
        {
        }

    public:
        /// map variable uid to Item object
        std::map<int, const Item *>     uidToItem;
        /// map item uid to concrete node
        TMap                            map;
        /// backward reference to FncDb (NULL for global graph)
        Fnc                            *fnc;
        /// shortcuts to global variables occurring in this graph
        TItemList                       globals;
        /// has this graph only one all-variables eating node?
        const Node                     *blackHole;
};

class GlobalData {
    public:
        /// graph corresponding to global variables
        Graph                   gptg;
        /// points-to build status
        bool                    dead;

    public:
        GlobalData() :
            dead(false)
        {
        }
};

bool isDead(const GlobalData &);

} // namespace PoinstTo

/**
 * function definition
 */
struct Fnc {
    struct cl_operand           def;    ///< definition as low-level operand
    Storage                    *stor;   ///< owning Storage object
    TVarSet                     vars;   ///< uids of variables used by the fnc
    TArgByPos                   args;   ///< args uid addressed by arg position
    ControlFlow                 cfg;    ///< fnc code as control flow graph
    PointsTo::Graph             ptg;    ///< points-to graph
    CallGraph::Node            *cgNode; ///< pointer to call-graph node or NULL

    Fnc():
        stor(0),
        ptg(this),
        cgNode(0)
    {
        def.code = CL_OPERAND_VOID;
    }
};

/**
 * return the name of given Fnc object (if any)
 */
const char* nameOf(const Fnc &);

/**
 * return the location info associated with the given Fnc object (if any)
 */
const struct cl_loc* locationOf(const Fnc &);

/**
 * return the uid of given Fnc object
 */
int uidOf(const Fnc &);

/**
 * return true if the function is defined (means we have its CFG and the like)
 */
bool isDefined(const Fnc &);

/**
 * lookup container for set of Fnc objects
 */
class FncDb {
    private:
        typedef STD_VECTOR(Fnc *) TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef const_iterator iterator;

    public:
        FncDb();
        ~FncDb();
        FncDb(const FncDb &);                       ///< shallow copy
        FncDb& operator=(const FncDb &);            ///< shallow copy

        /**
         * look for a function by ID, create one if not found
         * @param uid ID of the function to look for
         * @return referenced pointer to either a found or just created Fnc obj
         * @attention created objects will @b not be destroyed automatically
         */
        Fnc*& operator[](int uid);

        /**
         * look for a function by ID, crash if not found
         * @attention It is not safe to look for a non-existing function, it
         * will jump to debugger in that case.
         * @param uid ID of the function to look for
         * @return pointer to the found Fnc object
         */
        const Fnc* operator[](int uid) const;

        /**
         * return STL-like iterator to go through all functions inside
         */
        const_iterator begin() const { return fncs_.begin(); }

        /**
         * return STL-like iterator to go through all functions inside
         */
        const_iterator end()   const { return fncs_.end();   }

        /**
         * return count of functions inside the container
         */
        size_t size()          const { return fncs_.size();  }

    private:
        TList fncs_;
        struct Private;
        Private *d;
};

typedef std::vector<const Fnc *>                    TFncList;
typedef std::vector<const Insn *>                   TInsnList;
typedef std::map<Fnc *, TInsnList>                  TInsnListByFnc;

namespace CallGraph {

    /// call-graph node (there is a 1:1 relation with Fnc, if the CG is built)
    struct Node {
        Fnc                        *fnc;

        /// list of calls sorted by called fnc, zero key means an indirect call
        TInsnListByFnc              calls;

        /// list of direct call instructions that call this function
        TInsnListByFnc              callers;

        /// insns that take address of this function, zero key means initializer
        TInsnListByFnc              callbacks;

        Node(Fnc *fnc_):
            fnc(fnc_)
        {
        }
    };

    typedef std::set<Node *>                        TNodeList;

    struct Graph {
        TNodeList                   roots;
        TNodeList                   leaves;

        bool                        hasIndirectCall;
        bool                        hasCallback;

        TFncList                    topOrder;

        Graph():
            hasIndirectCall(false),
            hasCallback(false)
        {
        }
    };

} // namespace CallGraph

/**
 * a value type representing the @b whole @b serialised @b model of code
 */
struct Storage {
    TypeDb                      types;      ///< type info lookup container
    VarDb                       vars;       ///< variables lookup container
    FncDb                       fncs;       ///< functions lookup container
    NameDb                      varNames;   ///< var names lookup container
    NameDb                      fncNames;   ///< fnc names lookup container
    CallGraph::Graph            callGraph;  ///< call graph globals
    PointsTo::GlobalData        ptd;        ///< global PT-info
};

/// return the pointer to the Fnc object that the cfg instance is @b wrapped by
const Fnc* fncByCfg(const ControlFlow *cfg);

} // namespace CodeStorage

#endif /* H_GUARD_STORAGE_H */
