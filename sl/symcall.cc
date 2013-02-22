/*
 * Copyright (C) 2010-2011 Kamil Dudka <kdudka@redhat.com>
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

#include "config.h"
#include "symcall.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symabstract.hh"
#include "symbt.hh"
#include "symcmp.hh"
#include "symcut.hh"
#include "symdebug.hh"
#include "symheap.hh"
#include "symjoin.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "util.hh"

#include <algorithm>
#include <vector>

#include <boost/foreach.hpp>

LOCAL_DEBUG_PLOTTER(symcall, DEBUG_SYMCALL)

// /////////////////////////////////////////////////////////////////////////////
// call context cache per one fnc
class PerFncCache {
    private:
        typedef std::vector<SymCallCtx *> TCtxMap;

        SymHeapUnion    huni_;
        TCtxMap         ctxMap_;
#if !SE_ENABLE_CALL_CACHE
        SymCallCtx     *null_;
#endif
        int             missCntSinceLastHit_;

        int lookupCore(const SymHeap &sh);

        void cacheHit() {
            if (0 < missCntSinceLastHit_)
                missCntSinceLastHit_ = 0;
            else
                --missCntSinceLastHit_;
        }

    public:
        PerFncCache():
            missCntSinceLastHit_(0)
        {
        }

        ~PerFncCache() {
            BOOST_FOREACH(SymCallCtx *ctx, ctxMap_) {
                delete ctx;
            }
        }

        int missCntSinceLastHit() const {
            return missCntSinceLastHit_;
        }

        bool inUse() const {
            BOOST_FOREACH(const SymCallCtx *ctx, ctxMap_)
                if (ctx->inUse())
                    return true;

            return false;
        }

        void updateCacheEntry(const SymHeap &of, SymHeap by) {
#if !SE_ENABLE_CALL_CACHE
            CL_BREAK_IF("invalid call of PerFncCache::updateCacheEntry()");
            return;
#endif
            const int idx = this->lookupCore(of);
            CL_BREAK_IF(!areEqual(of, huni_[idx]));

            Trace::waiveCloneOperation(by);
            huni_.swapExisting(idx, by);
        }

        /**
         * look for the given heap; return the corresponding call ctx if found,
         * 0 otherwise
         */
        SymCallCtx*& lookup(const SymHeap &sh) {
#if SE_ENABLE_CALL_CACHE
            return ctxMap_[this->lookupCore(sh)];
#else
            (void) sh;
            return null_ = 0;
#endif
        }
};

int PerFncCache::lookupCore(const SymHeap &sh)
{
#if 1 < SE_ENABLE_CALL_CACHE
#if SE_STATE_ON_THE_FLY_ORDERING
#error "SE_STATE_ON_THE_FLY_ORDERING is incompatible with join-based call cache"
#endif
    EJoinStatus     status;
    SymHeap         result(sh.stor(), new Trace::TransientNode("PerFncCache"));
    const int       cnt = huni_.size();
    int             idx;

    // try join
    for(idx = 0; idx < cnt; ++idx) {
        const SymHeap &shIn = huni_[idx];
        if (!joinSymHeaps(&status, &result, shIn, sh))
            // join failed with this heap, try the next one
            continue;

        switch (status) {
            case JS_USE_ANY:
            case JS_USE_SH1:
                // already covered by the cached ctx --> cache hit!
                this->cacheHit();
                return idx;

            case JS_USE_SH2:
            case JS_THREE_WAY:
                // not covered, yet still possible to join with the cache entry
                break;
        }

        SymCallCtx *&ctx = ctxMap_[idx];
        if (ctx->inUse())
            // context in use by the current backtrace, keep going...
            continue;

        // destroy the current context
        delete ctx;
        ctx = 0;

        // update the cache entry
        if (JS_THREE_WAY == status)
            huni_.swapExisting(idx, result);
        else {
            CL_BREAK_IF(JS_USE_SH2 != status);
            SymHeap shDup(sh);
            Trace::waiveCloneOperation(shDup);
            huni_.swapExisting(idx, shDup);
        }

        this->cacheHit();
        return idx;
    }

#else // 1 == SE_ENABLE_CALL_CACHE means "graph isomorphism only"
    int idx = huni_.lookup(sh);
    if (-1 != idx) {
        this->cacheHit();
#   if 1 < SE_STATE_ON_THE_FLY_ORDERING
        rotate(ctxMap_.begin(), ctxMap_.begin() + idx, ctxMap_.end());
        return 0;
#   else
        return idx;
#   endif
    }
#endif

    // cache miss
    idx = ctxMap_.size();
    huni_.insertNew(sh);
    ctxMap_.push_back((SymCallCtx *) 0);
    CL_BREAK_IF(huni_.size() != ctxMap_.size());

    ++missCntSinceLastHit_;
    return idx;
}


// /////////////////////////////////////////////////////////////////////////////
// SymCallCache internal data
struct SymCallCache::Private {
    typedef const CodeStorage::Fnc                     &TFncRef;
    typedef CodeStorage::TVarSet                        TFncVarSet;
    typedef std::map<int /* uid */, PerFncCache>        TCache;
    typedef std::vector<SymCallCtx *>                   TCtxStack;

    TCache                      cache;
    TCtxStack                   ctxStack;
    SymBackTrace                bt;

    void importGlVar(SymHeap &sh, const CVar &cv);
    void resolveHeapCut(TCVarList &cut, SymHeap &sh, TFncRef fnc);
    SymCallCtx* getCallCtx(const SymHeap &entry, TFncRef fnc);

    Private(TStorRef stor):
        bt(stor)
    {
    }
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCtx
struct SymCallCtx::Private {
    SymCallCache::Private       *cd;
    const CodeStorage::Fnc      *fnc;
    SymHeap                     entry;
    SymHeap                     callFrame;
    const struct cl_operand     *dst;
    SymHeapUnion                rawResults;
    int                         nestLevel;
    bool                        computed;
    bool                        flushed;

    void assignReturnValue(SymHeap &sh);
    void destroyStackFrame(SymHeap &sh);

    // cppcheck-suppress uninitMemberVar
    Private(SymCallCache::Private *cd_):
        cd(cd_),
        fnc(0),
        entry(cd_->bt.stor(),
                new Trace::TransientNode("SymCallCtx::Private::entry")),
        callFrame(cd_->bt.stor(),
                new Trace::TransientNode("SymCallCtx::Private::callFrame")),
        computed(false),
        flushed(false)
    {
    }
};

SymCallCtx::SymCallCtx(SymCallCache::Private *cd):
    d(new Private(cd))
{
}

SymCallCtx::~SymCallCtx()
{
    delete d;
}

bool SymCallCtx::needExec() const
{
    return !d->computed;
}

bool SymCallCtx::inUse() const
{
    if (!d->flushed)
        return true;

    CL_BREAK_IF(!d->computed);
    return false;
}

const SymHeap& SymCallCtx::entry() const
{
    return d->entry;
}

SymState& SymCallCtx::rawResults()
{
    return d->rawResults;
}

void SymCallCtx::Private::assignReturnValue(SymHeap &sh)
{
    const cl_operand &op = *this->dst;
    if (CL_OPERAND_VOID == op.code)
        // we're done for a function returning void
        return;

    // wait, we're crossing stack frame boundaries here!  We need to use one
    // backtrace instance for source operands and another one for destination
    // operands.  The called function already appears on the given backtrace, so
    // that we can get the source backtrace by removing it from there locally.
    SymBackTrace callerSiteBt(this->cd->bt);
    callerSiteBt.popCall();
    SymProc proc(sh, &callerSiteBt);
    proc.setLocation(&op.data.var->loc);

    const FldHandle fldDst = proc.fldByOperand(op);
    const FldHandle fldSrc(sh, OBJ_RETURN, op.type);
    TValId val;
    if (fldSrc.isValidHandle()) {
        val = fldSrc.value();
    }
    else
        val = sh.valCreate(VT_UNKNOWN, VO_STACK);

    // assign the return value in the current symbolic heap
    proc.setValueOf(fldDst, val);
}

void SymCallCtx::Private::destroyStackFrame(SymHeap &sh)
{
    SymProc proc(sh, &this->cd->bt);

    // We need to look for junk since there can be a function returning an
    // allocated object.  Then ignoring the return value on the caller's
    // side can trigger a memory leak.  See test-0090.c for a use case.
    proc.objDestroy(OBJ_RETURN);

    TObjList live;
    sh.gatherObjects(live, isProgramVar);
    BOOST_FOREACH(const TObjId obj, live) {
        const EStorageClass code = sh.objStorClass(obj);
        if (SC_STATIC == code)
            // gl variable
            continue;

        // local variable
        const CVar cv(sh.cVarByObject(obj));
        if (!hasKey(this->fnc->vars, cv.uid) || cv.inst != this->nestLevel)
            // a local variable that is not here-local
            continue;

        const struct cl_loc *loc = 0;
        std::string varString = varToString(sh.stor(), cv.uid, &loc);
#if DEBUG_SE_STACK_FRAME
        CL_DEBUG_MSG(loc, "FFF destroying variable: " << varString);
#else
        (void) varString;
#endif
        proc.setLocation(loc);
        proc.objDestroy(obj);
    }
}

bool isGlVar(EStorageClass code)
{
    return (SC_STATIC == code);
}

void joinHeapsWithCare(
        SymHeap                        &sh,
        SymHeap                         callFrame,
        const CodeStorage::Fnc         *fnc)
{
    using namespace Trace;

    LDP_INIT(symcall, "join");
    LDP_PLOT(symcall, sh);
    LDP_PLOT(symcall, callFrame);

    // create a new trace graph node
    NodeHandle trResult(sh.traceNode());
    NodeHandle trFrame(callFrame.traceNode()->parent());

    // first off, we need to make sure that a gl variable from callFrame will
    // not overwrite the result of just completed function call since the var
    // could have already been imported from there
    TCVarList preserveGlVars;

    TObjList liveGlVars;
    callFrame.gatherObjects(liveGlVars, isGlVar);
    BOOST_FOREACH(const TObjId obj, liveGlVars) {
        const CVar cv = callFrame.cVarByObject(obj);
        CL_BREAK_IF(cv.inst);

        // check whether the var from 'callFrame' is alive in 'sh'
        if (isVarAlive(sh, cv))
            preserveGlVars.push_back(cv);
    }

    if (!preserveGlVars.empty()) {
        // conflict resolution: yield the gl vars from just completed fnc call
        SymHeap arena(callFrame.stor(), new TransientNode("preserveGlVars"));
        callFrame.swap(arena);
        splitHeapByCVars(&arena, preserveGlVars, &callFrame);
    }

    bool isFrameAlive = !liveGlVars.empty();
    if (!isFrameAlive) {
        TValList liveVars;
        callFrame.gatherObjects(liveGlVars, isProgramVar);
        isFrameAlive = !liveVars.empty();
    }

    Node *trDone = (isFrameAlive)
        ? new CallDoneNode(trResult.node(), trFrame.node(), fnc)
        : new CallDoneNode(trResult.node(), fnc);

    joinHeapsByCVars(&sh, &callFrame);
    sh.traceUpdate(trDone);
    LDP_PLOT(symcall, sh);
}

void SymCallCtx::flushCallResults(SymState &dst)
{
    using namespace Trace;

    // are we really ready for this?
    CL_BREAK_IF(d->flushed);

    // leave ctx stack
    CL_BREAK_IF(this != d->cd->ctxStack.back());
    d->cd->ctxStack.pop_back();

    // go through the results and make them of the form that the caller likes
    const unsigned cnt = d->rawResults.size();
    for (unsigned i = 0; i < cnt; ++i) {
        if (1 < cnt) {
            CL_DEBUG("*** SymCallCtx::flushCallResults() is processing heap #"
                     << i << " of " << cnt << " heaps total");
        }

        // clone the heap from the result currently being processed
        const SymHeap &origin = d->rawResults[i];
        SymHeap sh(origin);
        waiveCloneOperation(sh);

        if (d->computed) {
            // call cache hit --> tag the raw result as cached
            Node *trEntry = d->entry.traceNode();
            Node *trOrig = origin.traceNode();
            sh.traceUpdate(new CallCacheHitNode(trEntry, trOrig, d->fnc));
        }

        // first join the heap with its original callFrame
        joinHeapsWithCare(sh, d->callFrame, d->fnc);

        LDP_INIT(symcall, "post-processing");
        LDP_PLOT(symcall, sh);

        // perform all necessary action wrt. our function call convention
        d->assignReturnValue(sh);
        d->destroyStackFrame(sh);
        LDP_PLOT(symcall, sh);

#if SE_ABSTRACT_ON_CALL_DONE
        // after the final merge and cleanup, chances are that the abstraction
        // may be useful
        abstractIfNeeded(sh);
        LDP_PLOT(symcall, sh);
#endif
        // flush the result
        dst.insert(sh);
    }

    // mark as done
    d->computed = true;
    d->flushed = true;

    // leave backtrace
    d->cd->bt.popCall();
}

void SymCallCtx::invalidate()
{
#if SE_ENABLE_CALL_CACHE
#   if SE_CALL_CACHE_MISS_THR
    typedef SymCallCache::Private::TCache TCache;
    TCache &cache = d->cd->cache;
    const CodeStorage::Fnc &fnc = *d->fnc;
    const int uid = uidOf(fnc);
    const TCache::iterator it = cache.find(uid);
    CL_BREAK_IF(it == cache.end());

    const PerFncCache &pfc = it->second;
    const int missCnt = pfc.missCntSinceLastHit();
    if (missCnt < (SE_CALL_CACHE_MISS_THR))
        return;

    const struct cl_loc *loc = locationOf(fnc);
    CL_DEBUG_MSG(&loc, "SE_CALL_CACHE_MISS_THR reached for "
            << nameOf(fnc) << "(): " << missCnt);

    if (pfc.inUse()) {
        CL_DEBUG_MSG(&loc, "... but PerFncCache is still being used!");
        return;
    }

    cache.erase(it);
#   endif
#else
    delete this;
#endif
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCache
SymCallCache::SymCallCache(TStorRef stor):
    d(new Private(stor))
{
}

SymCallCache::~SymCallCache()
{
    delete d;
}

SymBackTrace& SymCallCache::bt()
{
    return d->bt;
}

void pullGlVar(SymHeap &result, SymHeap origin, const CVar &cv)
{
    // do not try to combine things, it causes problems
    CL_BREAK_IF(!areEqual(result, SymHeap(origin.stor(), origin.traceNode())));

    if (!isVarAlive(origin, cv)) {
        // not found in origin, create a fresh instance
        initGlVar(result, cv);
        return;
    }

    // cut by one gl var
    const TCVarList cut(1, cv);
    splitHeapByCVars(&origin, cut);
    result.swap(origin);
}

void pushGlVar(SymHeap &dst, const SymHeap &glSubHeap, const CVar &cv)
{
    // make sure the gl var is alive in 'glSubHeap'
    CL_BREAK_IF(!isVarAlive(const_cast<SymHeap &>(glSubHeap), cv));

    if (isVarAlive(dst, cv)) {
#ifndef NDEBUG
        // the gl var is alive in 'dst' --> check there is no conflict there
        SymHeap real(dst.stor(), dst.traceNode());
        pullGlVar(real, dst, cv);
        CL_BREAK_IF(!areEqual(real, glSubHeap));
#endif
        return;
    }

    // should be safe to join now
    joinHeapsByCVars(&dst, &glSubHeap);
}

void SymCallCache::Private::importGlVar(SymHeap &entry, const CVar &cv)
{
    const int cnt = this->ctxStack.size();
    if (!cnt) {
        // empty ctx stack --> no heap to import the var from
        initGlVar(entry, cv);
        return;
    }

    TStorRef stor = entry.stor();
    const struct cl_loc *loc = 0;
    std::string varString = varToString(stor, cv.uid, &loc);
    CL_DEBUG_MSG(loc, "<G> importGlVar() imports variable " << varString);

    // create a trace node for this call of importGlVar()
    entry.traceUpdate(new Trace::ImportGlVarNode(entry.traceNode(), varString));

    // seek the gl var going through the ctx stack backward
    int idx;
    for (idx = cnt - 1; 0 < idx; --idx) {
        SymCallCtx *ctx = this->ctxStack[idx];
        SymHeap &sh = ctx->d->callFrame;

        if (isVarAlive(sh, cv))
            break;
    }

    // 'origin' is the heap that we are importing the gl var from
    const SymHeap &origin = this->ctxStack[idx]->d->callFrame;

    // pull the designated gl var from 'origin'
    SymHeap glSubHeap(stor, new Trace::TransientNode("importGlVar()"));
    pullGlVar(glSubHeap, origin, cv);

    // go through all heaps above the 'origin' up to the current call level
    for (; idx < cnt; ++idx) {
        SymCallCtx *ctx = this->ctxStack[idx];

        // import gl variable at the current level
        const SymHeap src(ctx->d->entry);
        SymHeap &dst = ctx->d->entry;
        pushGlVar(dst, glSubHeap, cv);

        // update the corresponding cache entry
        const int uid = uidOf(*ctx->d->fnc);
        PerFncCache &pfc = this->cache[uid];
        pfc.updateCacheEntry(src, dst);
        CL_DEBUG_MSG(loc, "<G> importGlVar() updates a call cache entry for "
                << nameOf(*stor.fncs[uid]) << "()");
    }

    // finally import the gl var to the current call level
    pushGlVar(entry, glSubHeap, cv);
}

void SymCallCache::Private::resolveHeapCut(
        TCVarList                       &cut,
        SymHeap                         &sh,
        TFncRef                          fnc)
{
    const TFncVarSet &fncVars = fnc.vars;
    const int nestLevel = bt.countOccurrencesOfTopFnc();
#if SE_ENABLE_CALL_CACHE
    TStorRef stor = sh.stor();

    // start with all gl variables that are accessible from this function
    BOOST_FOREACH(const int uid, fncVars) {
        const CodeStorage::Var &var = stor.vars[uid];
        if (isOnStack(var))
            continue;

        const CVar cv(uid, /* gl var */ 0);
        if (!isVarAlive(sh, cv))
            // the var we need does not exist at this level yet --> lazy import
            this->importGlVar(sh, cv);

        cut.push_back(cv);
    }
#endif

    TObjList live;
    sh.gatherObjects(live, isProgramVar);
    BOOST_FOREACH(const TObjId obj, live) {
        const CVar cv(sh.cVarByObject(obj));

        const EStorageClass code = sh.objStorClass(obj);
        if (SC_STATIC == code) {
#if !SE_ENABLE_CALL_CACHE
            cut.push_back(cv);
#endif
            continue;
        }

        if (hasKey(fncVars, cv.uid) && cv.inst == nestLevel)
            cut.push_back(cv);
    }
}

void setCallArgs(
        SymProc                         &proc,
        const CodeStorage::Fnc          &fnc,
        const CodeStorage::Insn         &insn)
{
    // check insn validity
    using namespace CodeStorage;
    const TOperandList &opList = insn.operands;
    CL_BREAK_IF(CL_INSN_CALL != insn.code || opList.size() < 2);

    // get called fnc's args
    const TArgByPos &args = fnc.args;
    if (args.size() + 2 < opList.size()) {
        CL_DEBUG_MSG(&insn.loc,
                "too many arguments given (vararg fnc involved?)");

        CL_DEBUG_MSG(locationOf(fnc), "note: fnc was declared here");
    }

    // wait, we're crossing stack frame boundaries here!  We need to use one
    // backtrace instance for source operands and another one for destination
    // operands.  The called function already appears on the given backtrace, so
    // that we can get the source backtrace by removing it from there locally.
    const SymBackTrace &bt = *proc.bt();
    SymBackTrace callerSiteBt(bt);
    callerSiteBt.popCall();

    SymHeap &sh = proc.sh();
    SymProc srcProc(sh, &callerSiteBt);
    srcProc.setLocation(&insn.loc);

    // set args' values
    unsigned pos = /* dst + fnc */ 2;
    BOOST_FOREACH(int arg, args) {

        // cVar lookup
        const int nestLevel = bt.countOccurrencesOfFnc(uidOf(fnc));
        const CVar cVar(arg, nestLevel);
        const TObjId obj = sh.regionByVar(cVar, /* createIfNeeded */ true);

        // object instantiation
        TStorRef stor = *fnc.stor;
        const TObjType clt = stor.vars[arg].type;

        if (opList.size() <= pos) {
            // no value given for this arg
            const struct cl_loc *loc;
            std::string varString = varToString(stor, arg, &loc);
            CL_DEBUG_MSG(loc, "no fnc arg given for " << varString);
            continue;
        }

        // read the given argument's value
        const struct cl_operand &op = opList[pos++];
        const TValId val = srcProc.valFromOperand(op);
        CL_BREAK_IF(VAL_INVALID == val);

        // set the value of lhs accordingly
        const FldHandle argObj(sh, obj, clt);
        proc.setValueOf(argObj, val);
    }

    srcProc.killInsn(insn);
}

SymCallCtx* SymCallCache::Private::getCallCtx(const SymHeap &entry, TFncRef fnc)
{
    // cache lookup
    const int uid = uidOf(fnc);
    PerFncCache &pfc = this->cache[uid];
    SymCallCtx *&ctx = pfc.lookup(entry);
    if (!ctx) {
        // cache miss
        ctx = new SymCallCtx(this);
        ctx->d->fnc     = &fnc;
        ctx->d->entry   = entry;
        Trace::waiveCloneOperation(ctx->d->entry);

        // enter ctx stack
        this->ctxStack.push_back(ctx);
        return ctx;
    }

    const struct cl_loc *loc = locationOf(fnc);

    // cache hit, perform some sanity checks
    if (!ctx->d->computed) {
        // oops, we are not ready for this!
        CL_ERROR_MSG(loc, "call cache entry found, but result not "
                "computed yet; perhaps a recursive function call?");
        return 0;
    }
    if (!ctx->d->flushed) {
        // oops, we are not ready for this!
        CL_ERROR_MSG(loc, "call cache entry found, but result not "
                "flushed yet; perhaps a recursive function call?");
        return 0;
    }

    // enter ctx stack
    this->ctxStack.push_back(ctx);

    // all OK, return the cached ctx
    return ctx;
}

SymCallCtx* SymCallCache::getCallCtx(
        SymHeap                         entry,
        const CodeStorage::Fnc          &fnc,
        const CodeStorage::Insn         &insn)
{
    const struct cl_loc *loc = &insn.loc;
    CL_DEBUG_MSG(loc, "SymCallCache is looking for " << nameOf(fnc) << "()...");

    // build two new nodes of the trace graph
    Trace::waiveCloneOperation(entry);
    Trace::Node *trCall = entry.traceNode();
    Trace::Node *trEntry = new Trace::CallEntryNode(trCall, &insn);
    Trace::Node *trFrame = new Trace::CallFrameNode(trCall, &insn);

    // enlarge the backtrace
    const int uid = uidOf(fnc);
    d->bt.pushCall(uid, loc);

    // create SymProc and update the location info
    SymProc proc(entry, &d->bt);
    proc.setLocation(loc);

    // check recursion depth (if any)
    const int nestLevel = d->bt.countOccurrencesOfFnc(uid);
    if (1 != nestLevel)
        CL_DEBUG_MSG(loc, "recursive function call, depth = " << nestLevel);

    // initialize local variables of the called fnc
    LDP_INIT(symcall, "pre-processing");
    LDP_PLOT(symcall, entry);
    setCallArgs(proc, fnc, insn);
    LDP_PLOT(symcall, entry);

    // resolve heap cut
    TCVarList cut;
    d->resolveHeapCut(cut, entry, fnc);
    LDP_PLOT(symcall, entry);

    // prune heap
    LDP_INIT(symcall, "split");
    LDP_PLOT(symcall, entry);

    SymHeap callFrame(entry.stor(), trFrame);
    splitHeapByCVars(&entry, cut, &callFrame);
    callFrame.objInvalidate(OBJ_RETURN);
    entry.traceUpdate(trEntry);

    LDP_PLOT(symcall, entry);
    LDP_PLOT(symcall, callFrame);
    
    // get either an existing ctx, or create a new one
    SymCallCtx *ctx = d->getCallCtx(entry, fnc);
    if (!ctx)
        return 0;

    // not flushed yet
    ctx->d->flushed     = false;

    // keep some properties later required to process the results
    ctx->d->dst         = &insn.operands[/* dst */ 0];
    ctx->d->nestLevel   = nestLevel;
    ctx->d->callFrame   = callFrame;

    // update trace graph
    Trace::waiveCloneOperation(ctx->d->callFrame);
    ctx->d->entry.traceUpdate(trEntry);

    return ctx;
}
