/*
 * Copyright (C) 2014-2022 Kamil Dudka <kdudka@redhat.com>
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

#include "fixed_point_rewrite.hh"

#include <cl/cldebug.hh>
#include <cl/cl_msg.hh>
#include <cl/killer.hh>

namespace FixedPoint {

void MultiRewriter::insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn)
{
    for (IStateRewriter *slave : slaveList_)
        slave->insertInsn(src, dst, insn);
}

void MultiRewriter::replaceInsn(TLocIdx at, GenericInsn *insn)
{
    for (IStateRewriter *slave : slaveList_)
        slave->replaceInsn(at, insn);
}

void MultiRewriter::dropInsn(TLocIdx at)
{
    for (IStateRewriter *slave : slaveList_)
        slave->dropInsn(at);
}

void MultiRewriter::dropEdge(TLocIdx src, TLocIdx dst)
{
    for (IStateRewriter *slave : slaveList_)
        slave->dropEdge(src, dst);
}

void MultiRewriter::redirEdge(TLocIdx from, TLocIdx to, TLocIdx redirTo)
{
    for (IStateRewriter *slave : slaveList_)
        slave->redirEdge(from, to, redirTo);
}

void MultiRewriter::appendWriter(IStateRewriter &slave)
{
    slaveList_.push_back(&slave);
}

class IRewriteAction {
    public:
        virtual ~IRewriteAction() { }
        virtual void apply(IStateRewriter &writer) const = 0;
};

typedef std::vector<const IRewriteAction *>                 TActionList;

enum EActionKind {
    AK_BASIC,
    AK_REMOVE,
    AK_TOTAL
};

struct RecordRewriter::Private {
    TActionList actionLists[AK_TOTAL];

    ~Private()
    {
        for (int i = 0; i < AK_TOTAL; ++i)
            for (const IRewriteAction *action : this->actionLists[i])
                delete action;
    }
};

RecordRewriter::RecordRewriter():
    d(new Private)
{
}

RecordRewriter::~RecordRewriter()
{
    delete d;
}

class InsertInsnAction: public IRewriteAction {
    public:
        InsertInsnAction(TLocIdx src, TLocIdx dst, GenericInsn *insn):
            src_(src),
            dst_(dst),
            insn_(insn)
        {
        }

        ~InsertInsnAction()
        {
            delete insn_;
        }

        virtual void apply(IStateRewriter &writer) const
        {
            GenericInsn *insn = insn_->clone();
            writer.insertInsn(src_, dst_, insn);
        }

    private:
        TLocIdx                     src_;
        TLocIdx                     dst_;
        GenericInsn                *insn_;
};

void RecordRewriter::insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn)
{
    CL_BREAK_IF(src < 0);
    CL_BREAK_IF(dst < 0);
    const IRewriteAction *action = new InsertInsnAction(src, dst, insn);
    d->actionLists[AK_BASIC].push_back(action);
}

class ReplaceInsnAction: public IRewriteAction {
    public:
        ReplaceInsnAction(TLocIdx at, GenericInsn *insn):
            at_(at),
            insn_(insn)
        {
        }

        ~ReplaceInsnAction()
        {
            delete insn_;
        }

        virtual void apply(IStateRewriter &writer) const
        {
            GenericInsn *insn = insn_->clone();
            writer.replaceInsn(at_, insn);
        }

    private:
        TLocIdx                     at_;
        GenericInsn                *insn_;
};

void RecordRewriter::replaceInsn(TLocIdx at, GenericInsn *insn)
{
    CL_BREAK_IF(at < 0);
    const IRewriteAction *action = new ReplaceInsnAction(at, insn);
    d->actionLists[AK_BASIC].push_back(action);
}

class DropInsnAction: public IRewriteAction {
    public:
        DropInsnAction(TLocIdx at):
            at_(at)
        {
        }

        virtual void apply(IStateRewriter &writer) const
        {
            writer.dropInsn(at_);
        }

    private:
        TLocIdx                     at_;
};

void RecordRewriter::dropInsn(TLocIdx at)
{
    CL_BREAK_IF(at < 0);
    const IRewriteAction *action = new DropInsnAction(at);
    d->actionLists[AK_REMOVE].push_back(action);
}

void RecordRewriter::dropEdge(TLocIdx src, TLocIdx dst)
{
    // TODO
    (void) src;
    (void) dst;
    CL_BREAK_IF("please implement");
}

void RecordRewriter::redirEdge(TLocIdx from, TLocIdx to, TLocIdx redirTo)
{
    // TODO
    (void) from;
    (void) to;
    (void) redirTo;
    CL_BREAK_IF("please implement");
}

/// UNSAFE wrt. exceptions falling through IRewriteAction::apply(...)
void RecordRewriter::flush(IStateRewriter *pConsumer)
{
    for (int i = 0; i < AK_TOTAL; ++i) {
        TActionList &actionList = d->actionLists[i];

        for (const IRewriteAction *action : actionList) {
            action->apply(*pConsumer);
            delete action;
        }

        actionList.clear();
    }
}

bool RecordRewriter::empty() const
{
    for (int i = 0; i < AK_TOTAL; ++i)
        if (!d->actionLists[i].empty())
            return false;

    return true;
}

typedef std::pair<TLocIdx /* src */, TLocIdx /* dst */>     TEdge;
typedef std::map<TEdge, TLocIdx /* src */>                  TInsMap;

struct StateRewriter::Private {
    GlobalState                    &state;
    TInsMap                         insMap;

    Private(GlobalState *pState):
        state(*pState)
    {
    }
};

StateRewriter::StateRewriter(GlobalState *pState):
    d(new Private(pState))
{
}

StateRewriter::~StateRewriter()
{
    delete d;
}

void StateRewriter::insertInsn(
        TLocIdx                     src,
        const TLocIdx               dst,
        GenericInsn                *insn)
{
    const TEdge edge(src, dst);
    const TInsMap::iterator it = d->insMap.find(edge);
    if (it == d->insMap.end())
        d->insMap[edge] = /* new loc */ d->state.stateList_.size();
    else
        // inserting multiple locations --> we have to chain them
        std::swap(it->second, src);

    CL_NOTE("[ADT] inserting " << *insn
            << " between locations #" << src << " -> #" << dst);

    // allocate a new instruction
    LocalState *locState = new LocalState;
    locState->insn = insn;

    // append the instruction to the list
    const TLocIdx at = d->state.size();
    d->state.stateList_.append(locState);

    // resolve src/dst instructions
    LocalState &srcState = d->state[src];
    LocalState &dstState = d->state[dst];
    if (!srcState.insn || !dstState.insn)
        CL_BREAK_IF("invalid neighbour detected in insertInsn()");

    bool closesLoop = false;
    for (CfgEdge &oe : srcState.cfgOutEdges) {
        if (dst != oe.targetLoc)
            continue;

        closesLoop = oe.closesLoop;
        oe.targetLoc = at;
        oe.closesLoop = false;
    }

    for (CfgEdge &ie : dstState.cfgInEdges) {
        if (src != ie.targetLoc)
            continue;

        CL_BREAK_IF(closesLoop != ie.closesLoop);
        ie.targetLoc = at;
    }

    const CfgEdge ie(src);
    locState->cfgInEdges.push_back(ie);

    const CfgEdge oe(dst, closesLoop);
    locState->cfgOutEdges.push_back(oe);
}

void StateRewriter::replaceInsn(const TLocIdx at, GenericInsn *insn)
{
    CL_NOTE("[ADT] replacing insn #" << at << " by " << *insn);
    LocalState &locState = d->state[at];
    delete locState.insn;
    locState.insn = insn;
}

void StateRewriter::dropEdge(const TLocIdx src, const TLocIdx dst)
{
    CL_NOTE("[ADT] removing CFG edge #" << src << " -> #" << dst);

    LocalState &srcState = d->state[src];
    LocalState &dstState = d->state[dst];

    // remove src -> dst edges
    TCfgEdgeList outEdges;
    for (const CfgEdge &oe : srcState.cfgOutEdges)
        if (dst != oe.targetLoc)
            outEdges.push_back(oe);
    outEdges.swap(srcState.cfgOutEdges);

    // remove dst <- src edges
    TCfgEdgeList inEdges;
    for (const CfgEdge &ie : dstState.cfgInEdges)
        if (src != ie.targetLoc)
            inEdges.push_back(ie);
    inEdges.swap(dstState.cfgInEdges);
}

void StateRewriter::redirEdge(
        const TLocIdx               from,
        const TLocIdx               to,
        const TLocIdx               redirTo)
{
    // TODO: preserve loop-closing edge flag
    CL_NOTE("[ADT] redirecting CFG edge #" << from << " -> #" << to
            << " to #" << redirTo);

    // update output edges of 'from'
    LocalState &fromState = d->state[from];
    for (CfgEdge &oe : fromState.cfgOutEdges)
        if (oe.targetLoc == to)
            oe.targetLoc = redirTo;

    // update input edges of 'to'
    LocalState &toState = d->state[to];
    TCfgEdgeList toInEdges;
    for (const CfgEdge &ie : toState.cfgInEdges)
        if (from != ie.targetLoc)
            toInEdges.push_back(ie);
    toInEdges.swap(toState.cfgInEdges);

    // update input edges of 'redirTo'
    LocalState &redirToState = d->state[redirTo];
    redirToState.cfgInEdges.push_back(CfgEdge(from));
}

void StateRewriter::dropInsn(const TLocIdx at)
{
    LocalState &locState = d->state[at];

    std::string insnText;
    if (locState.insn) {
        // display the insn being removed
        std::ostringstream str;
        locState.insn->writeToStream(str);
        insnText = str.str();
    }

    CL_NOTE("[ADT] removing insn #" << at << " ... " << insnText);

    delete locState.insn;
    locState.insn = 0;

    // iterate through all incoming edges
    for (const CfgEdge &ie : locState.cfgInEdges) {
        LocalState &inState = d->state[ie.targetLoc];
        TCfgEdgeList outEdges;

        for (const CfgEdge &be : inState.cfgOutEdges) {
            if (at != be.targetLoc) {
                // keep unrelated CFG edges as they are
                outEdges.push_back(be);
                continue;
            }

            // redirect all edges previously going to 'at'
            for (CfgEdge oe : locState.cfgOutEdges) {
                oe.closesLoop |= ie.closesLoop;
                outEdges.push_back(oe);
            }
        }

        outEdges.swap(inState.cfgOutEdges);
    }

    // iterate through all outgoing edges
    for (const CfgEdge &oe : locState.cfgOutEdges) {
        LocalState &outState = d->state[oe.targetLoc];
        TCfgEdgeList inEdges;

        for (const CfgEdge &be : outState.cfgInEdges) {
            if (at != be.targetLoc) {
                // keep unrelated CFG edges as they are
                inEdges.push_back(be);
                continue;
            }

            // redirect all edges previously coming from 'at'
            for (CfgEdge ie : locState.cfgInEdges) {
                ie.closesLoop |= oe.closesLoop;
                inEdges.push_back(ie);
            }
        }

        inEdges.swap(outState.cfgInEdges);
    }

    // finally detach 'at' from the graph completely
    locState.cfgInEdges.clear();
    locState.cfgOutEdges.clear();
}

bool StateRewriter::dedupOutgoingEdges(const TLocIdx at)
{
    LocalState &locState = d->state[at];
    bool anyChange = false;

    // iterate through all outgoing edges
    for (const CfgEdge &oe : locState.cfgOutEdges) {
        LocalState &outState = d->state[oe.targetLoc];
        TCfgEdgeList inEdges;
        std::set<TLocIdx> inSet;

        for (const CfgEdge &be : outState.cfgInEdges) {
            const TLocIdx dst = be.targetLoc;
            if (dst == at && !insertOnce(inSet, dst)) {
                // duplicate edge detected
                anyChange = true;
                continue;
            }

            // keep other CFG edges as they are
            inEdges.push_back(be);
        }

        inEdges.swap(outState.cfgInEdges);
    }

    if (!anyChange)
        // nothing changed actually
        return false;

    TCfgEdgeList outEdges;
    std::set<TLocIdx> outSet;

    // iterate through all outgoing edges
    for (const CfgEdge &oe : locState.cfgOutEdges) {
        if (insertOnce(outSet, oe.targetLoc))
            outEdges.push_back(oe);
    }

    outEdges.swap(locState.cfgOutEdges);
    return true;
}

void StateRewriter::mergeInsns(const TLocIdx locDst, const TLocIdx locSrc)
{
    CL_NOTE("[ADT] merging insn #" << locDst << " with insn #" << locSrc);

    // make sure we have exactly one outgoing CFG edge from each location
    LocalState &dstState = d->state[locDst];
    LocalState &srcState = d->state[locSrc];
    CL_BREAK_IF(dstState.cfgOutEdges.size() != 1U);
    CL_BREAK_IF(srcState.cfgOutEdges.size() != 1U);

    // make sure that both locations have the same successor location
    CfgEdge &dstOutEdge = dstState.cfgOutEdges.front();
    CfgEdge &srcOutEdge = srcState.cfgOutEdges.front();
    const TLocIdx locOut = dstOutEdge.targetLoc;
    CL_BREAK_IF(locOut != srcOutEdge.targetLoc);
    LocalState &outState = d->state[locOut];

    // redirect incoming edges of locSrc to locDst
    for (CfgEdge &ie : srcState.cfgInEdges) {
        ie.closesLoop |= srcOutEdge.closesLoop;
        dstState.cfgInEdges.push_back(ie);
        for (CfgEdge &oe : d->state[ie.targetLoc].cfgOutEdges) {
            if (oe.targetLoc != locSrc)
                continue;

            oe.targetLoc = locDst;
            oe.closesLoop = ie.closesLoop;
        }
    }

    // remove all backward references to src
    for (const CfgEdge &oe : dstState.cfgOutEdges) {
        LocalState &outState = d->state[oe.targetLoc];
        TCfgEdgeList inEdges;

        for (const CfgEdge &be : outState.cfgInEdges)
            if (locSrc != be.targetLoc)
                // keep only CFG edges NOT going to src
                inEdges.push_back(be);

        inEdges.swap(outState.cfgInEdges);
    }

    // preserve the loop-closing edge flag
    if (dstOutEdge.closesLoop != srcOutEdge.closesLoop) {
        dstOutEdge.closesLoop = false;
        for (CfgEdge &e : outState.cfgInEdges)
            if (e.targetLoc == locDst)
                e.closesLoop = false;

        if (dstOutEdge.closesLoop) {
            for (CfgEdge &ie : dstState.cfgInEdges) {
                ie.closesLoop |= dstOutEdge.closesLoop;
                for (CfgEdge &oe : d->state[ie.targetLoc].cfgOutEdges)
                    if (oe.targetLoc == locSrc)
                        oe.closesLoop = ie.closesLoop;
            }
        }
    }

    // finally remove locSrc from the CFG
    srcState.cfgInEdges.clear();
    srcState.cfgOutEdges.clear();
    delete srcState.insn;
    srcState.insn = 0;
}

void ClInsn::lazyInit() const
{
    using namespace CodeStorage::VarKiller;

    if (done_)
        return;
    done_ = true;

    BlockData data;
    scanInsn(&data, insn_);

    for (const TVar var : data.gen)
        live_.insert(GenericVar(VL_CODE_LISTENER, var));

    for (const TVar var : data.kill)
        kill_.insert(GenericVar(VL_CODE_LISTENER, var));
}

const TGenericVarSet& ClInsn::liveVars() const
{
    this->lazyInit();
    return live_;
}

const TGenericVarSet& ClInsn::killVars() const
{
    this->lazyInit();
    return kill_;
}

void ClInsn::writeToStream(std::ostream &str) const
{
    str << *insn_;
}

void TextInsn::writeToStream(std::ostream &str) const
{
    str << text_;
}

} // namespace FixedPoint
