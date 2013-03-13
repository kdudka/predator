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

#include "util.hh"
#include "worklist.hh"
#include "builtins.hh"

#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "clplot.hh"
#include "pointsto.hh"
#include "pointsto_fics.hh"

#include <boost/foreach.hpp>

template <class T>
class FixPoint: public WorkList<T, std::queue<T> >
{
    private:
        typedef WorkList<T, std::queue<T> > TBase;
        int accessCounter_;

    public:
        FixPoint():
            accessCounter_(0)
        {
        }

        bool next(T &dst) {
            if (!TBase::next(dst))
                return false;

            accessCounter_++;
            return true;
        }

        int steps() const {
            return accessCounter_;
        }
};

namespace CodeStorage {
namespace PointsTo {

typedef CodeStorage::Storage           &TStorRef;

#define PLOT_PROGRESS(ctx)                                                  \
    do {                                                                    \
        if (ctx.plot.progress)                                              \
            plotGraph(ctx.stor, ctx.plot.progress);                         \
    } while (0)

typedef enum {
    PTFICS_RET_NO_CHANGE = 0,
    PTFICS_RET_CHANGE,
    PTFICS_RET_FAIL
} RetVal;

// these definitions are for bindPair() function
typedef enum  {
    BINDPAIR_VAR = 0,
    BINDPAIR_RET,
    BINDPAIR_HEAP
} EBindPairType;

typedef struct {
    EBindPairType code;

    // all important info known from callers site
    struct {
        const cl_operand               *operand;
    } caller;
    struct {
        // uid of fnc or parameter -- depends on 'code'
        int                             uid;
    } callee;
} TBindPair;

typedef std::vector<TBindPair> TBindPairs;

#define FALLBACK(msg)                                                       \
    do {                                                                    \
        PT_ERROR(msg);                                                      \
        goto fallback;                                                      \
    } while(0)

// Template for creating data-per-node databases in algorithms.
template <class TData>
class DataManager {
    public:
        DataManager() { }

        ~DataManager() {
            BOOST_FOREACH(TRef it, cont_)
                delete it.second;
        }

        TData* alloc(const Node *n) {
            CL_BREAK_IF(hasKey(cont_, n));
            TData *data = new TData;
            cont_[n] = data;
            return data;
        }

        TData* getData(const Node *n) const {
            typename TMap::const_iterator it = cont_.find(n);
            CL_BREAK_IF(cont_.end() == it);
            return it->second;
        }

        bool empty() const {
            return cont_.empty();
        }

        template <class TDst>
        void getAll(TDst &dst) const {
            BOOST_FOREACH(TRef it, cont_)
                dst.push_back(it.second);
        }
        bool hasData(const Node *n) const {
            return hasKey(cont_, n);
        }

    private:
        DataManager(const DataManager &);
        DataManager& operator=(const DataManager &);

    private:
        typedef std::map<const Node *, TData *> TMap;
        typedef typename TMap::const_reference  TRef;
        TMap cont_;
};

bool isNull(const cl_operand &op)
{
    if (op.code != CL_OPERAND_CST)
        return false;

    if (!isDataPtr(op.type))
        return false;

    return (op.data.cst.data.cst_int.value == 0);
}

inline bool isPtrRelatedType(const cl_type *type)
{
    if (!type)
        return false;
    if (type->code == CL_TYPE_STRUCT)
        return true;
    return isDataPtr(type);
}

inline bool isPtrRelated(const cl_operand &op)
{
    if (op.code == CL_OPERAND_VOID)
        return false;

    if (op.code == CL_OPERAND_CST && op.data.cst.code == CL_TYPE_STRING)
        // we don't care about constant strings
        return false;

    return isPtrRelatedType(op.type);
}

const char *fncNameFromInsn(const Insn *insn)
{
    CL_BREAK_IF(insn->code != CL_INSN_CALL);
    const cl_operand *fncOp = &insn->operands[1 /* fnc */];
    const char *name;
    if (!fncNameFromCst(&name, fncOp))
        // white-list only constants?
        return NULL;
    return name;
}

int generateMallocUid(const Insn *insn)
{
    static int i = 0;
    static std::map<const Insn *, int> ids;
    if (hasKey(ids, insn))
        return ids[insn];

    ids[insn] = ++i;
    return i;
}

bool isKnownModel(const Insn *insn, TBindPairs &pairs)
{
    const char *name = fncNameFromInsn(insn);
    if (!name)
        // not known call..
        return false;

    const TOperandList &opList = insn->operands;
    if (STREQ(name, "malloc")) {
        CL_BREAK_IF(opList.size() != 3);
        TBindPair pair;

        CL_BREAK_IF(opList[0].code == CL_OPERAND_VOID);

        pair.code = BINDPAIR_HEAP;
        pair.callee.uid = -generateMallocUid(insn);
        pair.caller.operand = &opList[0];

        pairs.push_back(pair);
        return true;
    }

    return false;
}

// TODO: push this to separate file (it would be nice to provide some general
//       models for known functions).

bool isWhiteListedName(const char *name)
{
    return STREQ(name, "malloc")
        || STREQ(name, "free")
        || STREQ(name, "___sl_error")
        || STREQ(name, "__VERIFIER_plot");
}

bool isWhiteListed(const Insn *insn)
{
    const char *name = fncNameFromInsn(insn);
    if (!name)
        // white-list only constants?
        return false;

    return isWhiteListedName(name);
}

bool isWhiteListed(const Fnc *fnc)
{
    const char *name = nameOf(*fnc);
    return isWhiteListedName(name);
}

void dumpPairs(const TBindPairs &pairs)
{
    BOOST_FOREACH(const TBindPair &pair, pairs) {
        PT_DEBUG(0, "paircode: " << pair.code);
    }
}

/**
 * Find the pairs of operands from caller's site bounded to arguments of callee
 * site.  Bind also return value with callers definition.
 *
 * return true when some error occurred
 */
bool bindPairs(const Insn *insn, TBindPairs &pairs)
{
    TStorRef stor = *insn->stor;
    const TOperandList &opList = insn->operands;
    const cl_operand &retOp = opList[0];
    const cl_operand &calleeOp = opList[1];

    int calleeUid;
    const Fnc *callee;

    if (!fncUidFromOperand(&calleeUid, &calleeOp))
        FALLBACK("TODO: indirect call");

    callee = stor.fncs[calleeUid];

    // if we know that this call may _NOT_ ever create some real PT-relations in
    // callers site, do not add anything to "pairs"
    if (isKnownModel(insn, pairs))
        return false;

    if (isWhiteListed(insn))
        return false;

    if (opList.size() - 2 > callee->args.size()) {
        FALLBACK("TODO: bad number of parameters: " << *insn
                << " (" << callee->args.size() << " expected)");
    }

    // cover only pointer related return values
    if (retOp.code != CL_OPERAND_VOID && isPtrRelated(retOp)) {
        CL_BREAK_IF(retOp.code == CL_OPERAND_CST);

        TBindPair p;
        p.code                  = BINDPAIR_RET;
        p.caller.operand        = &retOp;

        p.callee.uid = calleeUid;
        pairs.push_back(p);
    }

    /// handle all pointer related operands
    for (unsigned i = 2; i < opList.size(); ++i) {
        const cl_operand &op = opList[i];
        if (!isPtrRelated(op))
            continue;
        if (isNull(op))
            continue;

        TBindPair p;
        p.code              = BINDPAIR_VAR;
        p.callee.uid        = callee->args[i - 2];
        p.caller.operand    = &op;
        pairs.push_back(p);
    }

    return false;

fallback:
    // problem found
    return true;
}

bool isNeutralInsnPhase1(const Insn &insn)
{
    switch (insn.code) {
        case CL_INSN_JMP:
        case CL_INSN_COND:
        case CL_INSN_LABEL:
        case CL_INSN_SWITCH:
        case CL_INSN_ABORT:
            return true;
        case CL_INSN_UNOP:
            switch (insn.subCode) {
                case CL_UNOP_ASSIGN:
                    return false;
                default:
                    return true;
            }
            break;
        case CL_INSN_BINOP:
            switch (insn.subCode) {
                case CL_BINOP_EQ:
                case CL_BINOP_NE:
                case CL_BINOP_LT:
                case CL_BINOP_LE:
                case CL_BINOP_GT:
                case CL_BINOP_GE:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

// return true if the insn is pointer-related assignment
RetVal phase1handleInsn(
        BuildCtx                        &ctx,
        const Insn                      &insn)
{
    const TOperandList &opList = insn.operands;
    Graph &ptg = *ctx.ptg;

    if (isNeutralInsnPhase1(insn))
        return PTFICS_RET_NO_CHANGE;

    if (insn.code == CL_INSN_CALL)
        // this phase skips calls silently!
        return PTFICS_RET_NO_CHANGE;

    /// HANDLE return statement
    if (insn.code == CL_INSN_RET) {
        if (opList.size() != 1) {
            PT_DEBUG(3, "void return");
            return PTFICS_RET_NO_CHANGE;
        }

        const cl_operand &opParam = opList[0];
        if (!isPtrRelated(opParam))
            return PTFICS_RET_NO_CHANGE;

        bool referenced;
        Node *left = getNode(ptg, ctx.ptg->fnc->def);
        Node *right = nodeAccessS(ctx.stor, ptg, opParam, &referenced);

        if (left == right)
            return PTFICS_RET_NO_CHANGE;

        if (referenced) {
            appendNodeS(ctx, ptg, left, right);
        }
        else {
            left = preventEndingS(left);
            right = preventEndingS(right);
            ctx.joinTodo.push_back(TNodePair(left, right));
        }
        joinFixPointS(ctx, ptg);

        return PTFICS_RET_CHANGE;
    }

    const cl_operand &opLeft = opList[0];
    const cl_operand &opRight = opList[1];
    Node *left, *right;

    if (insn.code != CL_INSN_UNOP || insn.subCode != CL_UNOP_ASSIGN)
        // FIXME handled only assignment for now
        FALLBACK("unhandled instruction type: " << insn);

    if (opLeft.code != CL_OPERAND_VAR || opRight.code != CL_OPERAND_VAR)
        return PTFICS_RET_NO_CHANGE;

    if (!isPtrRelated(opLeft) || !isPtrRelated(opRight)) {
        if (isPtrRelated(opLeft) != isPtrRelated(opRight))
            FALLBACK("bad assignment!");

        // this is not and pointer related assignment
        return PTFICS_RET_NO_CHANGE;
    }

    bool referenced;
    left  = nodeAccessS(ctx.stor, ptg, opLeft);
    right = nodeAccessS(ctx.stor, ptg, opRight, &referenced);

    if (referenced)
        // address taken -- just append this operand as a following node
        appendNodeS(ctx, ptg, left, right);
    else {
        preventEndingS(left);
        preventEndingS(right);

        ctx.joinTodo.push_back(std::make_pair(
                *left->outNodes.begin(),
                *right->outNodes.begin()));
    }

    // go down through accessors to find following join-candidates
    joinFixPointS(ctx, ptg);
    return PTFICS_RET_CHANGE;

fallback:
    ctx.stor.ptd.dead = 1;
    return PTFICS_RET_FAIL;
}

void makeBlackHole(Fnc &fnc)
{
    // should be still unvisited
    CL_BREAK_IF(fnc.ptg.map.size() != 0);

    Graph &ptg = fnc.ptg;
    VarDb &vars = fnc.stor->vars;

    Node *blackHole = new Node;
    addEdge(blackHole, blackHole); // self loop

    // all parameters are in one node
    BOOST_FOREACH(int uid, fnc.args) {
        const Var *v = &vars[uid];
        Item *i = new Item(v);

        bindItem(ptg, blackHole, i);
    }

    BOOST_FOREACH(const Var &v, vars) {
        if (v.code != VAR_GL)
            continue;
        Item *i = new Item(&v);
        bindItem(ptg, blackHole, i);
    }

    setBlackHole(ptg, blackHole);
}

bool ficsPhase1(BuildCtx &ctx)
{
    const Storage & stor = ctx.stor;

    PT_DEBUG(1, "> phase 1 <");
    if (!(ctx.debug.phases & FICS_PHASE_1)) {
        PT_DEBUG(1, "skipping");
        return true;
    }

    BOOST_FOREACH(const Fnc *pFnc, stor.callGraph.topOrder) {
        Fnc &fnc = *const_cast<Fnc *>(pFnc);
        ctx.ptg = &fnc.ptg;
        if (isBuiltInFnc(fnc.def))
            // just skip built-ins
            continue;

        if (!isDefined(fnc)) {
            if (isWhiteListed(&fnc))
                continue;

            PT_DEBUG(2, "creating black hole for function: '"
                        << nameOf(fnc) << "'");
            makeBlackHole(fnc);
            PLOT_PROGRESS(ctx);
            continue;
        }

        PT_DEBUG(2, "function: '" << nameOf(fnc) << "'");

        BOOST_FOREACH(const Block *bb, fnc.cfg) {
            PT_DEBUG(3, "block: " << bb->name());
            BOOST_FOREACH(const Insn *insn, *bb) {

                int rc = phase1handleInsn(ctx, *insn);
                if (PTFICS_RET_NO_CHANGE == rc)
                    continue;

                if (PTFICS_RET_CHANGE == rc) {
                    PLOT_PROGRESS(ctx);
                    continue;
                }
                // error occurred
                return false;
            }
        }
    }

    return true;
}

typedef enum {
    BINDDATA_ARG = 0,
    BINDDATA_GLOB
} TBindDataType;

struct TBindData {
    TBindDataType code;

    struct {
        // corresponding callers operand bound to 'startFrom' node.  Note that
        // we need to remember pointer to whole operand for better looking for
        // correct node using accessors.  Structure Var is not enough.
        union {
            // valid only for BINDDATA_ARG
            const cl_operand           *operand;
            // valid only for BINDDATA_GLOB
            const Var                  *glob;
        } data;

    } caller;

    struct {
        // start point of of PT-graph on the callee site
        const Node                     *start;
        // number of steps to come to join-able node from ^^^ startFrom node
        int                             steps;
        // FICS algorithm says that in this phase we shouldn't join targets of
        // globals only.  The target must be somehow connected with parameters
        // of called function.
        // This flag just says that the node which has this flag set to true can
        // point to some node of the graph which is pointed to by parameter.
        // This flag is valid only for root nodes.
        bool                            joinable;
    } callee;

    TBindData() :
        code(BINDDATA_ARG)
    {
        memset(&caller, 0, sizeof(caller));
        memset(&callee, 0, sizeof(callee));
    }
};

typedef DataManager<TBindData> TDMBind;

bool joinable(TDMBind &dm, const Node *n)
{
    TBindData *data = dm.getData(n);
    if (data->callee.joinable)
        return true;

    return dm.getData(data->callee.start)->callee.joinable;
}

bool joinAtDepth(
        BuildCtx                       &ctx,
        Graph                          &ptg,
        Node                           *leftStart,
        int                             leftDepth,
        Node                           *rightStart,
        int                             rightDepth)
{
    Node *left  = goDownS(leftStart,  leftDepth);
    Node *right = goDownS(rightStart, rightDepth);

    if (left == right)
        // just continue .. this nodes were probably joined before
        return false;

    CL_BREAK_IF(ctx.joinTodo.size() > 0);

    // do the job..
    ctx.joinTodo.push_back(TNodePair(left, right));
    joinFixPointS(ctx, ptg);
    return true; // graph was changed
}

/**
 * this is bind() helper.
 *
 * return true when the ptg structure changed
 */
bool joinTgtVisited(
        BuildCtx                       &ctx,
        TDMBind                        &dm,
        Graph                          &ptg,
        const Node                     *cldVisited,
        Node                           *right,
        int                             rDepth)
{
    if (!joinable(dm, cldVisited))
        // no parameter points to this node..
        return false;

    TBindData *visitedD = dm.getData(cldVisited);
    Node *left = 0;
    int lDepth = visitedD->callee.steps;

    switch (visitedD->code) {
        case BINDDATA_GLOB:
            left = getNode(ptg, visitedD->caller.data.glob);
            break;

        case BINDDATA_ARG: {
            bool lIsRef;
            const cl_operand &op = *visitedD->caller.data.operand;
            left = nodeAccessS(ctx.stor, ptg, op, &lIsRef);
            if (lIsRef)
                lDepth --;
        } break;
    }

    if (joinAtDepth(ctx, ptg, left, lDepth, right, rDepth))
        return true;
    return false;
}

RetVal bindHeap(BuildCtx &ctx, Graph &ptg, TBindPairs &pairs)
{
    CL_BREAK_IF(pairs.empty());
    TBindPair &p = pairs[0];
    CL_BREAK_IF(p.code != BINDPAIR_HEAP);

    // find or create malloc output in caller
    Node *mallocNode = findNode(ptg, p.callee.uid);
    if (!mallocNode) {
        Item *it = new Item(PT_ITEM_MALLOC);
        it->data.mallocId = p.callee.uid;

        mallocNode = allocNodeForItem(ptg, it);
    }

    int uid = varIdFromOperand(p.caller.operand);
    Node *parent = findNode(ptg, uid);
    if (!parent) {
         const Var *v = &ctx.stor.vars[uid];
         parent = getNode(ptg, v);
    }

    // follow the target
    parent = preventEndingS(parent);

    if (mallocNode != parent) {
        appendNodeS(ctx, ptg, parent, mallocNode);
        joinFixPointS(ctx, ptg);
        return PTFICS_RET_CHANGE;
    }

    return PTFICS_RET_NO_CHANGE;
}

/**
 * Shape the callers graph based on informations from graph in callee function
 * (the shaping starts from arguments of call-instructions).  This phase does
 * not map the memory addresses (local variables) of callee to caller -- it just
 * shapes the graph.
 *
 * @return true if th caller's Graph was changed.
 */
RetVal bind(
        BuildCtx                        &ctx,
        const Insn                      *insn,
        Fnc                             *caller,
        const Fnc                       *callee)
{
    // TODO: referenced parameters, return value, variable length of
    // parameters

    TStorRef stor = ctx.stor;
    VarDb &vars = ctx.stor.vars;
    Graph &ptg = caller->ptg; // changed is going to be only caller's graph now

    WorkList<const Node *> wl;
    TDMBind dm;
    bool change = false;

    TBindPairs pairs;
    if (isWhiteListed(insn)) {
        if (isKnownModel(insn, pairs))
            return bindHeap(ctx, caller->ptg, pairs);
        else
            return PTFICS_RET_NO_CHANGE;
    }

    if (bindPairs(insn, pairs))
        return PTFICS_RET_FAIL;

    //// schedule of processing all PT-related parameters of called function
    BOOST_FOREACH(TBindPair &p, pairs) {
        const Node *argNode = existsUid(callee->ptg, p.callee.uid);
        if (!argNode)
            // this variable may not bee bound
            continue;

        if (!dm.hasData(argNode)) {
            // first inspection of this node - the joining is not possible yet
            wl.schedule(argNode);

            TBindData *d = dm.alloc(argNode);

            // note that here may occur global variable also!
            d->code                 = BINDDATA_ARG;

            // here is safe to remember the 'argNode' because callee's graph is
            // not going to be changed
            d->callee.start         = argNode;
            d->callee.steps         = 0;
            d->callee.joinable      = true;

            d->caller.data.operand  = p.caller.operand;
            continue;
        }

        // already seen node.  At least two function arguments are situated in
        // the same node -> join corresponding nodes in caller.
        TBindData *argData = dm.getData(argNode);

        // corresponding node from caller (based on previously explored operand)
        bool lIsRef;
        Node *lNode = nodeAccessS(stor, ptg, *argData->caller.data.operand, &lIsRef);
        // find/create corresponding node for actually handled parameter
        bool rIsRef;
        Node *rNode = nodeAccessS(stor, ptg, *p.caller.operand, &rIsRef);

        if (lIsRef == rIsRef) {
            if (joinAtDepth(ctx, ptg, lNode, 0, rNode,  0))
                change = true;
            continue;
        }

        if (lIsRef)
            appendNodeS(ctx, ptg, rNode, lNode);
        else
            appendNodeS(ctx, ptg, lNode, rNode);
        joinFixPointS(ctx, ptg);
        change = true;
    }

    // do the DFS starting from parameter related nodes
    const Node *actualNode;
    while (wl.next(actualNode)) {
        TBindData *actD = dm.getData(actualNode);
        const Node *targetNode = hasOutputS(actualNode);
        if (!targetNode)
            // no output edge - backtrack DFS for this branch of state space
            continue;

        if (!dm.hasData(targetNode)) {
            TBindData *d            = dm.alloc(targetNode);
            d->code                 = BINDDATA_ARG;
            d->callee.start         = actD->callee.start;
            d->callee.steps         = actD->callee.steps + 1;
            d->callee.joinable      = true;
            d->caller.data.operand  = actD->caller.data.operand;

            wl.schedule(targetNode);
            continue;
        }

        // the targetNode was already explored before - join corresponding nodes
        // in callers PT-graph.
        TBindData *targetData = dm.getData(targetNode);

        bool rIsRef;
        const cl_operand &rOp = *actD->caller.data.operand;
        Node *rStart = nodeAccessS(ctx.stor, ptg, rOp, &rIsRef);
        int rDepth = actD->callee.steps + 1;
        if (rIsRef)
            rDepth -= 1;

        bool lIsRef;
        const cl_operand &lOp = *targetData->caller.data.operand;
        Node *lStart = nodeAccessS(ctx.stor, ptg, lOp, &lIsRef);
        int lDepth = targetData->callee.steps;
        if (lIsRef) {
            CL_BREAK_IF(lDepth < 1);
            lDepth--;
        }

        if (joinAtDepth(ctx, ptg, lStart, lDepth, rStart, rDepth))
            change = true;
    }

    BOOST_FOREACH(int v_uid, callee->vars) {
        const Var *v = &vars[v_uid];
        if (v->code != VAR_GL)
            // skip non-globals
            continue;

        const Node *cldGlobNode = existsVar(callee->ptg, v);
        if (!cldGlobNode)
            // impossible to bind anything
            continue;

        if (dm.hasData(cldGlobNode)) {
            // Handle already seen nodes.  There are two possibilities -- either
            // this variable was visited starting from parameter (callee's
            // argument) or starting from another global variable.

            // find or create..
            Node *rStart = getNode(ptg, v);
            if (joinTgtVisited(ctx, dm, ptg, cldGlobNode, rStart, 0))
                change = true;

            continue;
        }

        /// cldGlobNode is visited firstly..

        TBindData *d = dm.alloc(cldGlobNode);
        d->code             = BINDDATA_GLOB;
        d->callee.start     = cldGlobNode;
        d->callee.steps     = 0;
        d->callee.joinable  = false; // for now is not join-able
        d->caller.data.glob = v;
        wl.schedule(cldGlobNode);

        const Node *handled;
        while (wl.next(handled)) {
            TBindData *handledD = dm.getData(handled);
            const Node *target = hasOutputS(handled);
            if (!target)
                // no join possible from this node
                continue;

            if (!dm.hasData(target)) {
                // go down
                CL_BREAK_IF(wl.seen(target));
                wl.schedule(target);
                TBindData *d = dm.alloc(target);
                d->code = handledD->code;
                d->callee.start = handledD->callee.start;
                d->callee.steps = handledD->callee.steps + 1;
                d->callee.joinable = handledD->callee.joinable;
                d->caller.data = handledD->caller.data;
                continue;
            }

            // target node was visited before
            CL_BREAK_IF(handledD->code == BINDDATA_ARG);
            Node *rStart = getNode(ptg, handledD->caller.data.glob);
            int rDepth = handledD->callee.steps + 1;
            if (joinTgtVisited(ctx, dm, ptg, target, rStart, rDepth)) {
                change = true;
                // we need to set this sub-graph starting from appropriate
                // global variable related node as joinable.
                TBindData *startD = dm.getData(handledD->callee.start);
                startD->callee.joinable = true;
            }
        }
    }

    return change ? PTFICS_RET_CHANGE : PTFICS_RET_NO_CHANGE;
}

struct NodeData {
    /// node to start traversal from
    const Node                     *startFrom;

    /// how far we should go from startFrom
    int                             steps;

    /// caller's item (if we know it)
    const Item                     *clrItem;

    NodeData() :
        startFrom(0),
        steps(0),
        clrItem(0)
    {
    }
};

/**
 * shape the global graph (ctx.stor.ptd.gptg) based on actually handled
 * function's graph (ctx.ptg).
 */
RetVal bindGlobal(BuildCtx &ctx)
{
    typedef DataManager<NodeData>   TDM;
    TDM dm;
    bool change = false;
    TStorRef stor = ctx.stor;
    Graph &gptg = stor.ptd.gptg; // this graph is going to be changed now
    Graph &ptg = *ctx.ptg;

    PT_DEBUG(2, "performing BindGlobal");

    WorkList<const Node *> wl;
    BOOST_FOREACH(const Item *i, ptg.globals) {
        CL_BREAK_IF(!i->isGlobal());

        const Node *node = existsItem(ptg, i);
        CL_BREAK_IF(!node);

        if (!wl.seen(node)) {
            wl.schedule(node);

            NodeData *d = dm.alloc(node);
            d->startFrom    = node;
            d->steps        = 0;
            d->clrItem      = i;
            continue;
        }

        // already seen node -- try to join nodes in global graph
        NodeData *prevData = dm.getData(node);

        Node *left  = nodeFromForeign(gptg, prevData->clrItem);
        Node *right = nodeFromForeign(gptg, i);
        CL_BREAK_IF(!left || !right);

        if (joinAtDepth(ctx, gptg, left, 0, right, 0))
            change = true;
    }

    const Node *actual;
    while (wl.next(actual)) {
        const Node *target = hasOutputS(actual);
        if (!target)
            // no join is possible
            continue;

        NodeData *actualData = dm.getData(actual);

        if (!dm.hasData(target)) {
            // not seen yet
            wl.schedule(target);
            NodeData *d = dm.alloc(target);
            d->startFrom    = actualData->startFrom;
            d->steps        = actualData->steps + 1;
            continue;
        }

        NodeData *targetData = dm.getData(target);

        // JOIN
        const Node *lGlN = dm.getData(target)->startFrom;
        const Node *rGlN = dm.getData(actual)->startFrom;

        NodeData *tmpD = dm.getData(lGlN);
        Node *left = nodeFromForeign(gptg, tmpD->clrItem);
        tmpD = dm.getData(rGlN);
        Node *right = nodeFromForeign(gptg, tmpD->clrItem);
        CL_BREAK_IF(!left || !right);

        if (joinAtDepth(ctx, gptg, left,  targetData->steps,
                                   right, actualData->steps + 1))
        {
            change = true;
        }
    }

    if (change)
        PLOT_PROGRESS(ctx);

    // restore back
    return change ? PTFICS_RET_CHANGE : PTFICS_RET_NO_CHANGE;
}

// definitions used by bindLoc function
struct TBindLocItem {
    struct {
        const Item                     *item;
        bool                            referenced;
    } src;

    struct Dst {
        const Var                      *v;
        Dst (): v(0) { }
    } dst;

    TBindLocItem() {
        src.item = 0;
        src.referenced = false;
    }
};
typedef std::vector<TBindLocItem> TBindLocData;

/**
 * Compute the memory locations in target graph based on source graph.  The
 * computation starts from pairs identifying source and target node properly.
 */
bool bindLocations(
        BuildCtx                       &ctx,
        const TBindLocData             &bindData,
        const Graph                    *srcPtg,
        Graph                          *tgtPtg)
{
    if (srcPtg == tgtPtg) {
        PT_DEBUG(0, "recursion detected, giving up...");
        ctx.stor.ptd.dead = true;
        return false;
    }

    const char *nameA = srcPtg->fnc ? nameOf(*srcPtg->fnc) : "globals";
    const char *nameB = tgtPtg->fnc ? nameOf(*tgtPtg->fnc) : "globals";

    PT_DEBUG(2, "processing BindLoc '" << nameA << "' to '" << nameB << "'");

    bool change = false;
    BOOST_FOREACH(const TBindLocItem &item, bindData) {
        const Node *startSrcNode = existsItem(*srcPtg, item.src.item);
        if (!startSrcNode)
            continue;
        if (!item.src.referenced) {
            startSrcNode = hasOutputS(startSrcNode);
            if (!startSrcNode)
                continue;
        }

        WorkList<const Node *> wl;
        wl.schedule(startSrcNode);

        Node *tgtNode;
        if (!item.dst.v) {
            // malloced
            const Item *srcItem = item.src.item;
            CL_BREAK_IF(srcItem->code != PT_ITEM_MALLOC);
            tgtNode = findNode(*tgtPtg, srcItem->uid());
            if (!tgtNode) {
                Item *tmpIt = new Item(srcItem->code);
                tmpIt->data.mallocId = srcItem->uid();
                tgtNode = allocNodeForItem(*tgtPtg, tmpIt);
            }
        }
        else 
            tgtNode = getNode(*tgtPtg, item.dst.v);

        const Node *srcNode;
        for (int depth = 0; wl.next(srcNode); depth++) {
            Node *dstNode = goDownS(tgtNode, depth + 1);
            if (bindVarList(ctx, *tgtPtg, dstNode, srcNode->variables))
                change = true;
            CL_BREAK_IF(existsError(ctx.stor));

            const Node *srcNextNode = hasOutputS(srcNode);
            if (!srcNextNode)
                break;

            wl.schedule(srcNextNode);
        }
    }

    CL_BREAK_IF(existsError(ctx.stor));
    return change;
}

/**
 * bindLocations() wrapper
 *
 * this wrapper starts automatically from argument-related nodes
 *
 * return true if the target node was changed
 */
bool bindLocationsArgs(
        BuildCtx                       &ctx,
        const TBindPairs               &pairs,
        const Graph                    *srcPtg,
        Graph                          *tgtPtg)
{
    TBindLocData bindData;
    BOOST_FOREACH(const TBindPair &pair, pairs) {
        if (pair.code == BINDPAIR_RET)
            continue;

        const cl_operand &op = *pair.caller.operand;
        CL_BREAK_IF(op.code != CL_OPERAND_VAR);

        const int srcUid = varIdFromOperand(&op);
        if (!hasKey(srcPtg->uidToItem, srcUid))
            // this may not be bound
            continue;

        TBindLocItem item;
        item.src.referenced = seekRefAccessor(op.accessor);
        // now it must exist
        item.src.item = srcPtg->uidToItem.find(srcUid)->second;
        item.dst.v = &ctx.stor.vars[pair.callee.uid];
        bindData.push_back(item);
    }

    return bindLocations(ctx, bindData, srcPtg, tgtPtg);
}

/**
 * bindLocations() wrapper
 *
 * - starts automatically from globals-related nodes
 *
 * @return true if the target node was changed
 */
bool bindLocationsGlob(
        BuildCtx                       &ctx,
        const Graph                    *srcPtg,
        Graph                          *tgtPtg)
{
    TBindLocData bindData;
    BOOST_FOREACH(const Item *item, srcPtg->globals) {
        CL_BREAK_IF(!item->isGlobal());

        TBindLocItem li;
        li.src.referenced = false;
        li.src.item = item;

        li.dst.v = item->code == PT_ITEM_VAR 
                 ? item->data.var
                 : NULL;
        bindData.push_back(li);
    }

    return bindLocations(ctx, bindData, srcPtg, tgtPtg);
}

template <class TWl>
void scheduleTopologically(TWl &dst, const CallGraph::Graph &cg)
{
    BOOST_FOREACH(const Fnc *fnc, cg.topOrder) {
        if (isBuiltInFnc(fnc->def) || isWhiteListed(fnc))
            continue;

        dst.schedule(const_cast<Fnc *>(fnc));
    }
}

bool ficsPhase2(BuildCtx &ctx)
{
    TStorRef stor = ctx.stor;
    FixPoint<Fnc *> fp;

    PT_DEBUG(1, "> phase 2 <");
    if (!(ctx.debug.phases & FICS_PHASE_2)) {
        PT_DEBUG(1, "skipping");
        return true;
    }

    // pre-plan to explore all functions
    scheduleTopologically(fp, stor.callGraph);

    Fnc *caller;
    while (fp.next(caller)) {
        PT_DEBUG(1, "processing '" << nameOf(*caller) << "'");
        bool change = false;
        ctx.ptg = &caller->ptg;
        CallGraph::Node *cgNode = caller->cgNode;

        // all calling functions should be shaped based on 'caller' function
        BOOST_FOREACH(TInsnListByFnc::const_reference item, cgNode->calls) {
            const Fnc *callee = item.first;
            if (!callee)
                // indirect function call
                FALLBACK("TODO: indirect call");

            if (isBuiltInFnc(callee->def))
                continue;

            // all calls of 'callee' may change shape of callee's graph
            const TInsnList &calls = item.second;
            BOOST_FOREACH(const Insn *insn, calls) {
                // so let's shape the graph of 'callee'
                RetVal rc = bind(ctx, insn, caller, callee);
                if (PTFICS_RET_FAIL == rc)
                    FALLBACK("bind failed");
                if (PTFICS_RET_CHANGE == rc)
                    change = true;
            }
        }

        // shape the Global points-to graph based on 'caller's one
        RetVal rv = bindGlobal(ctx);
        if (PTFICS_RET_FAIL == rv)
            FALLBACK("BindGlobal failed");
        if (PTFICS_RET_CHANGE == rv)
            change = true;

        // push the global aliases from caller into global points-to graph
        if (bindLocationsGlob(ctx, ctx.ptg /* src */, &stor.ptd.gptg /* tgt */))
            change = true;

        if (!change)
            continue;

        // plan successors to process again when something changed in caller
        BOOST_FOREACH(TInsnListByFnc::reference item, cgNode->calls) {
            Fnc *cld = item.first;
            if (isBuiltInFnc(cld->def) || isWhiteListed(cld))
                continue;
            fp.schedule(cld);
        }
    }

    PT_DEBUG(1, "fixpoint reached in " << fp.steps() << " steps");
    return true; // success

fallback:
    stor.ptd.dead = true;
    return false;
}

bool ficsPhase3(BuildCtx &ctx)
{
    TStorRef stor = ctx.stor;
    FixPoint<Fnc *> fp;

    PT_DEBUG(1, "> phase 3 <");
    if (!(ctx.debug.phases & FICS_PHASE_3)) {
        PT_DEBUG(1, "skipping");
        return true;
    }

    // plan to explore all functions we are interested in
    scheduleTopologically(fp, stor.callGraph);

    Fnc *callee;
    while (!stor.ptd.dead && fp.next(callee)) {
        bool change = false;
        // push the alias info from global PT-graph to graph of handled fnc
        if (bindLocationsGlob(ctx, &stor.ptd.gptg, &callee->ptg))
            change = true;
        CallGraph::Node *cgNode = callee->cgNode;

        // go through all functions calling the 'callee' one
        BOOST_FOREACH(TInsnListByFnc::const_reference item, cgNode->callers) {
            Fnc *caller = item.first;

            BOOST_FOREACH(const Insn *insn, item.second) {
                TBindPairs pairs;
                if (bindPairs(insn, pairs))
                    FALLBACK("binding operands -> parameters");

                if (bindLocationsArgs(ctx, pairs, &caller->ptg, &callee->ptg))
                    change = true;
            }
        }

        if (change) {
            // plan to re-visit all successors
            BOOST_FOREACH(TInsnListByFnc::reference item, cgNode->calls) {
                Fnc *cld = item.first;
                if (isBuiltInFnc(cld->def) || isWhiteListed(cld))
                    continue;
                fp.schedule(cld);
            }
        }
    }

    PT_DEBUG(2, "fixpoint reached in " << fp.steps() << " steps");
    return true;

fallback:
    return false;
}

bool runFICS(BuildCtx &ctx)
{
    // all phases should success to provide correct points-to graph
    return ficsPhase1(ctx) && ficsPhase2(ctx) && ficsPhase3(ctx);
}

} /* namespace PointsTo */
} /* namespace CodeStorage */
