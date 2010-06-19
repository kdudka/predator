/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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
#include <cl/location.hh>
#include <cl/storage.hh>

#include "symbt.hh"
#include "symcut.hh"
#include "symheap.hh"
#include "symproc.hh"
#include "symstate.hh"

#include <vector>

#include <boost/foreach.hpp>

#ifndef DEBUG_SE_STACK_FRAME
#   define DEBUG_SE_STACK_FRAME 0
#endif

#ifndef SE_BYPASS_CALL_CACHE
#   define SE_BYPASS_CALL_CACHE 0
#endif

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCtx
struct SymCallCtx::Private {
    SymBackTrace                *bt;
    const CodeStorage::Fnc      *fnc;
    SymHeap                     heap;
    SymHeap                     surround;
    const struct cl_operand     *dst;
    SymHeapUnion                rawResults;
    int                         nestLevel;
    bool                        computed;
    bool                        flushed;

    void assignReturnValue(SymHeapUnion &state);
    void destroyStackFrame(SymHeapUnion &state);
};

SymCallCtx::SymCallCtx():
    d(new Private)
{
    d->computed = false;
    d->flushed = false;
}

SymCallCtx::~SymCallCtx() {
    delete d;
}

bool SymCallCtx::needExec() const {
    return !d->computed;
}

const SymHeap& SymCallCtx::entry() const {
    return d->heap;
}

SymHeapUnion& SymCallCtx::rawResults() {
    return d->rawResults;
}

void SymCallCtx::Private::assignReturnValue(SymHeapUnion &state) {
    const cl_operand &op = *this->dst;
    if (CL_OPERAND_VOID == op.code)
        // we're done for a function returning void
        return;

    // go through results and perform assignment of the return value
    BOOST_FOREACH(SymHeap &res, state) {
        SymHeapProcessor proc(res, this->bt);
        proc.setLocation(&op.loc);

        const TObjId obj = proc.heapObjFromOperand(op);
        if (OBJ_INVALID == obj)
            TRAP;

        const TValueId val = res.valueOf(OBJ_RETURN);
        if (VAL_INVALID == val)
            TRAP;

        // assign the return value in the current symbolic heap
        res.objSetValue(obj, val);
    }
}

void SymCallCtx::Private::destroyStackFrame(SymHeapUnion &state) {
    using namespace CodeStorage;
    const Fnc &ref = *this->fnc;

#if DEBUG_SE_STACK_FRAME
    const LocationWriter lw(&ref.def.loc);
    CL_DEBUG_MSG(lw, "<<< destroying stack frame of "
            << nameOf(ref) << "()"
            << ", nestLevel = " << this->nestLevel);

    int hCnt = 0;
#endif

    BOOST_FOREACH(SymHeap &res, state) {
#if DEBUG_SE_STACK_FRAME
        if (1 < this->rawResults.size()) {
            CL_DEBUG_MSG(lw, "*** destroying stack frame in result #"
                    << (++hCnt));
        }
#endif
        SymHeapProcessor proc(res, this->bt);

        BOOST_FOREACH(const int uid, ref.vars) {
            const Var &var = ref.stor->vars[uid];
            if (isOnStack(var)) {
                const LocationWriter lw(&var.loc);
#if DEBUG_SE_STACK_FRAME
                CL_DEBUG_MSG(lw, "<<< destroying stack variable: #"
                        << var.uid << " (" << var.name << ")" );
#endif

                const CVar cVar(var.uid, this->nestLevel);
                const TObjId obj = res.objByCVar(cVar);
                if (obj < 0)
                    TRAP;

                proc.setLocation(lw);
                proc.destroyObj(obj);
            }
        }

        // We need to look for junk since there can be a function returning an
        // allocated object.  Then ignoring the return value on the caller's
        // side can trigger a memory leak.
        // TODO: write a test-case for this!
        proc.destroyObj(OBJ_RETURN);
    }
}

void SymCallCtx::flushCallResults(SymHeapUnion &dst) {
    // mark as done
    d->computed = true;
    d->flushed = true;

    // polish the results
    SymHeapUnion results(d->rawResults);
    d->assignReturnValue(results);
    d->destroyStackFrame(results);

    // now merge the results with the original surround
    // TODO: some verbose output
    BOOST_FOREACH(SymHeap &heap, results) {
        joinHeapsByCVars(&heap, &d->surround);
    }

    // flush results
    dst.insert(results);
}

// /////////////////////////////////////////////////////////////////////////////
// call context cache per one fnc
// FIXME: suboptimal interaction with SymHeapUnion during lookup/insert
class PerFncCache {
    private:
        typedef std::vector<SymCallCtx *> TCtxMap;

        SymHeapUnion    huni_;
        TCtxMap         ctxMap_;

    public:
        ~PerFncCache() {
            BOOST_FOREACH(SymCallCtx *ctx, ctxMap_) {
                delete ctx;
            }
        }

        /**
         * look for the given heap; return the corresponding call ctx if found,
         * 0 otherwise
         */
        SymCallCtx* lookup(const SymHeap &heap) {
#if SE_BYPASS_CALL_CACHE
            return 0;
#endif
            int idx = huni_.lookup(heap);
            if (-1 == idx)
                return 0;

            return ctxMap_.at(idx);
        }

        /**
         * store the given heap with its corresponding call ctx into the cache
         */
        void insert(const SymHeap &heap, SymCallCtx *ctx) {
#if SE_BYPASS_CALL_CACHE
            return;
#endif
            huni_.insert(heap);
            ctxMap_.push_back(ctx);
            if (huni_.size() != ctxMap_.size())
                // integrity of PerFncCache broken, perhaps called unexpectedly?
                TRAP;
        }
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCache
struct SymCallCache::Private {
    typedef std::map<int /* uid */, PerFncCache> TCache;

    TCache                      cache;
    SymBackTrace                *bt;
    LocationWriter              lw;
    SymHeap                     *heap;
    SymHeapProcessor            *proc;
    const CodeStorage::Fnc      *fnc;
    int                         nestLevel;

    SymHeap::TContCVar          glVars;

    void createStackFrame();
    void setCallArgs(const CodeStorage::TOperandList &opList,
                     SymHeap::TContCVar &args);
    SymCallCtx* getCallCtx(int uid, const SymHeap &heap);
};

SymCallCache::SymCallCache(SymBackTrace *bt):
    d(new Private)
{
    using namespace CodeStorage;
    d->bt = bt;

    const Storage &stor = bt->stor();
    BOOST_FOREACH(const Var &var, stor.vars) {
        if (VAR_GL == var.code) {
            const CVar cVar(var.uid, /* gl variable */ 0);
            d->glVars.push_back(cVar);
        }
    }

    const unsigned found = d->glVars.size();
    if (found)
        CL_DEBUG("(g) SymCallCache found " << found << " gl variable(s)");
}

SymCallCache::~SymCallCache() {
    delete d;
}

void SymCallCache::Private::createStackFrame() {
    using namespace CodeStorage;
    const Fnc &ref = *this->fnc;

#if DEBUG_SE_STACK_FRAME
    CL_DEBUG_MSG(this->lw,
            ">>> creating stack frame for " << nameOf(ref) << "()"
            << ", nestLevel = " << this->nestLevel);
#endif

    BOOST_FOREACH(const int uid, ref.vars) {
        const Var &var = ref.stor->vars[uid];
        if (isOnStack(var)) {
#if DEBUG_SE_STACK_FRAME
            LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, ">>> creating stack variable: #" << var.uid
                    << " (" << var.name << ")" );
#endif

            const CVar cVar(var.uid, this->nestLevel);
            this->heap->objCreate(var.clt, cVar);
        }
    }
}

void SymCallCache::Private::setCallArgs(const CodeStorage::TOperandList &opList,
                                        SymHeap::TContCVar &cut)
{
    // get called fnc's args
    const CodeStorage::TArgByPos &args = this->fnc->args;
    if (args.size() + 2 != opList.size())
        TRAP;

    // wait, we're crossing stack frame boundaries here!  We need to use one
    // backtrace instance for source operands and another one for destination
    // operands.  The called function already appears on the given backtrace, so
    // that we can get the source backtrace by removing it from there locally.
    SymBackTrace callerSiteBt(*this->bt);
    callerSiteBt.popCall();
    SymHeapProcessor srcProc(*this->heap, &callerSiteBt);

    // set args' values
    int pos = /* dst + fnc */ 2;
    BOOST_FOREACH(int arg, args) {
        const struct cl_operand &op = opList[pos++];
        srcProc.setLocation(this->lw);
        this->proc->setLocation(this->lw);

        const TValueId val = srcProc.heapValFromOperand(op);
        if (VAL_INVALID == val)
            TRAP;

        // store arg as program variable (later used for heap pruning)
        const CVar cVar(arg, this->nestLevel);
        cut.push_back(cVar);

        const TObjId lhs = this->heap->objByCVar(cVar);
        if (OBJ_INVALID == lhs)
            TRAP;

        // set arg's value
        this->proc->heapSetVal(lhs, val);
    }
}

SymCallCtx* SymCallCache::Private::getCallCtx(int uid, const SymHeap &heap) {
    // cache lookup
    PerFncCache &pfc = this->cache[uid];
    SymCallCtx *ctx = pfc.lookup(heap);
    if (!ctx) {
        // cache miss
        ctx = new SymCallCtx;
        ctx->d->bt      = this->bt;
        ctx->d->fnc     = this->fnc;
        ctx->d->heap    = heap;
        pfc.insert(heap, ctx);
        return ctx;
    }

    // cache hit, perform some sanity checks
    if (!ctx->d->computed) {
        // oops, we are not ready for this!
        CL_ERROR("SymCallCache: cache entry found, but result not computed yet"
                 ", perhaps a recursive function call?");
        return 0;
    }
    if (!ctx->d->flushed) {
        // oops, we are not ready for this!
        CL_ERROR("SymCallCache: cache entry found, but result not flushed yet"
                 ", perhaps a recursive function call?");
        return 0;
    }

    // all OK, return the cached ctx
    return ctx;
}

SymCallCtx& SymCallCache::getCallCtx(SymHeap heap,
                                     const CodeStorage::Insn &insn)
{
    using namespace CodeStorage;

    // check insn validity
    const TOperandList &opList = insn.operands;
    if (CL_INSN_CALL != insn.code || opList.size() < 2)
        TRAP;

    // create SymHeapProcessor and update the location info
    SymHeapProcessor proc(heap, d->bt);
    proc.setLocation((d->lw = &insn.loc));
    d->heap = &heap;
    d->proc = &proc;

    // look for lhs
    const struct cl_operand &dst = opList[/* dst */ 0];
    if (CL_OPERAND_VOID != dst.code) {
        // create an object for return value
        heap.objDestroy(OBJ_RETURN);
        heap.objDefineType(OBJ_RETURN, dst.type);
    }

    // look for Fnc ought to be called
    const int uid = proc.fncFromOperand(opList[/* fnc */ 1]);
    d->fnc = insn.stor->fncs[uid];
    if (!d->fnc || CL_OPERAND_VOID == d->fnc->def.code)
        // this should have been handled elsewhere
        TRAP;

    CL_DEBUG_MSG(d->lw, "SymCallCache is looking for "
            << nameOf(*d->fnc) << "()...");

    // check recursion depth (if any)
    d->nestLevel = d->bt->countOccurrencesOfFnc(uid);
    if (1 != d->nestLevel) {
        CL_WARN_MSG(d->lw, "support of call recursion is not stable yet");
        CL_NOTE_MSG(d->lw, "nestLevel is " << d->nestLevel);
    }

    // start with gl variables as the cut
    SymHeap::TContCVar cut(d->glVars);

    // initialize local variables of the called fnc
    d->createStackFrame();
    d->setCallArgs(opList, cut);

    // prune heap
    SymHeap surround;
    CL_DEBUG_MSG(d->lw, "|C| pruning heap by " << cut.size() << " variable(s)");
    splitHeapByCVars(&heap, cut, &surround);
    
    // get either an existing ctx, or create a new one
    SymCallCtx *ctx = d->getCallCtx(uid, heap);
    if (!ctx)
        TRAP;

    // not flushed yet
    ctx->d->flushed     = false;

    // keep some properties later required to process the results
    ctx->d->dst         = &dst;
    ctx->d->nestLevel   = d->nestLevel;
    ctx->d->surround    = surround;
    return *ctx;
}
