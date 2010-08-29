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

#ifndef SE_ABSTRACT_ON_CALL_DONE
#   define ABSTRACT_ON_CALL_DONE 0
#endif

#ifndef SE_DISABLE_CALL_CACHE
#   define SE_DISABLE_CALL_CACHE 0
#endif

#if SE_ABSTRACT_ON_CALL_DONE
#   include "symabstract.hh"
#endif

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCtx
struct SymCallCtx::Private {
    SymBackTrace                *bt;
    const CodeStorage::Fnc      *fnc;
    SymHeap                     heap;
    SymHeap                     surround;
    const struct cl_operand     *dst;
    SymState                    rawResults;
    int                         nestLevel;
    bool                        computed;
    bool                        flushed;

    void assignReturnValue(SymState &state);
    void destroyStackFrame(SymState &state);
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

SymState& SymCallCtx::rawResults() {
    return d->rawResults;
}

void SymCallCtx::Private::assignReturnValue(SymState &state) {
    const cl_operand &op = *this->dst;
    if (CL_OPERAND_VOID == op.code)
        // we're done for a function returning void
        return;

    // wait, we're crossing stack frame boundaries here!  We need to use one
    // backtrace instance for source operands and another one for destination
    // operands.  The called function already appears on the given backtrace, so
    // that we can get the source backtrace by removing it from there locally.
    SymBackTrace callerSiteBt(*this->bt);
    callerSiteBt.popCall();

    // go through results and perform assignment of the return value
    BOOST_FOREACH(SymHeap &res, state) {
        SymProc proc(res, &callerSiteBt);
        proc.setLocation(&op.loc);

        const TObjId obj = proc.heapObjFromOperand(op);
        SE_BREAK_IF(OBJ_INVALID == obj);

        const TValueId val = res.valueOf(OBJ_RETURN);
        SE_BREAK_IF(VAL_INVALID == val);

        // assign the return value in the current symbolic heap
        proc.objSetValue(obj, val);
    }
}

void SymCallCtx::Private::destroyStackFrame(SymState &state) {
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
                    << (hCnt++));
        }
#endif
        SymProc proc(res, this->bt);

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
                SE_BREAK_IF(obj < 0);

                proc.setLocation(lw);
                proc.objDestroy(obj);
            }
        }

        // We need to look for junk since there can be a function returning an
        // allocated object.  Then ignoring the return value on the caller's
        // side can trigger a memory leak.  See data/test-0090.c for a use case.
        proc.objDestroy(OBJ_RETURN);
    }
}

namespace {

void flush(SymState &dst, const SymState src) {
    BOOST_FOREACH(SymHeap sh, src) {
#if SE_ABSTRACT_ON_CALL_DONE
        // after the final merge and cleanup, chances are that the abstraction
        // may be useful
        abstractIfNeeded(sh);
#endif
        dst.insert(sh);
    }
}

} // namespace

void SymCallCtx::flushCallResults(SymState &dst) {
    if (d->flushed)
        // are we really ready for this?
        SE_TRAP;

    // mark as done
    d->computed = true;
    d->flushed = true;

    // now merge the results with the original surround
    SymState results(d->rawResults);
    BOOST_FOREACH(SymHeap &heap, results) {
        joinHeapsByCVars(d->bt, &heap, &d->surround);
    }

    // polish the results
    d->assignReturnValue(results);
    d->destroyStackFrame(results);

    // flush the results
    flush(dst, results);
}

// /////////////////////////////////////////////////////////////////////////////
// call context cache per one fnc
class PerFncCache {
    private:
        typedef std::vector<SymCallCtx *> TCtxMap;

        SymState        huni_;
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
#if SE_DISABLE_CALL_CACHE
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
#if SE_DISABLE_CALL_CACHE
            return;
#endif
            huni_.insertNew(heap);
            ctxMap_.push_back(ctx);
            SE_BREAK_IF(huni_.size() != ctxMap_.size());
        }
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCache
struct SymCallCache::Private {
    typedef SymBackTrace::TFncSeq                       TFncSeq;
    typedef SymHeapTyped::TContCVar                     TCVars;
    typedef std::map<int /* uid */, PerFncCache>        TCacheCol;
    typedef std::map<TCVars, TCacheCol>                 TCacheRow;
    typedef std::map<TFncSeq, TCacheRow>                TCache;

    TCache                      cache;
    SymBackTrace                *bt;
    LocationWriter              lw;
    SymHeap                     *heap;
    SymProc                     *proc;
    const CodeStorage::Fnc      *fnc;
    int                         nestLevel;

    SymHeap::TContCVar          glVars;

    void createStackFrame(SymHeap::TContCVar &vars);
    void setCallArgs(const CodeStorage::TOperandList &opList);
    SymCallCtx* getCallCtx(int uid, const SymHeap &heap);
};

SymCallCache::SymCallCache(SymBackTrace *bt):
    d(new Private)
{
    using namespace CodeStorage;
    d->bt = bt;

    // gather all gl variables
    const Storage &stor = bt->stor();
    BOOST_FOREACH(const Var &var, stor.vars) {
        if (VAR_GL == var.code) {
            const CVar cVar(var.uid, /* gl variable */ 0);
            d->glVars.push_back(cVar);
        }
    }

    // count gl variables
    const unsigned found = d->glVars.size();
    if (found)
        CL_DEBUG("(g) SymCallCache found " << found << " gl variable(s)");
}

SymCallCache::~SymCallCache() {
    delete d;
}

void SymCallCache::Private::createStackFrame(SymHeap::TContCVar &cVars) {
    using namespace CodeStorage;
    const Fnc &ref = *this->fnc;

#if DEBUG_SE_STACK_FRAME
    CL_DEBUG_MSG(this->lw,
            ">>> creating stack frame for " << nameOf(ref) << "()"
            << ", nestLevel = " << this->nestLevel);
#endif

    // go through all variables that are visible from the function
    BOOST_FOREACH(const int uid, ref.vars) {
        const Var &var = ref.stor->vars[uid];
        if (isOnStack(var)) {
            // automatic variable (and/or fnc arg)
#if DEBUG_SE_STACK_FRAME
            LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, ">>> creating stack variable: #" << var.uid
                    << " (" << var.name << ")" );
#endif

            // gather stack frame in order to prune the heap afterwards
            const CVar cv(var.uid, this->nestLevel);
            cVars.push_back(cv);

            // now create the SymHeap object
            this->heap->objCreate(var.clt, cv);
        }
    }
}

void SymCallCache::Private::setCallArgs(const CodeStorage::TOperandList &opList)
{
    // get called fnc's args
    const CodeStorage::TArgByPos &args = this->fnc->args;
    if (args.size() + 2 != opList.size()) {
        CL_WARN_MSG(this->lw, "count of given arguments does not match "
                    "function's declaration: " << nameOf(*this->fnc) << "()");

        const LocationWriter lwDecl(&this->fnc->def.loc);
        CL_NOTE_MSG(lwDecl, "functions with variable "
                    "count of arguments are not supported yet");
    }

    // wait, we're crossing stack frame boundaries here!  We need to use one
    // backtrace instance for source operands and another one for destination
    // operands.  The called function already appears on the given backtrace, so
    // that we can get the source backtrace by removing it from there locally.
    SymBackTrace callerSiteBt(*this->bt);
    callerSiteBt.popCall();
    SymProc srcProc(*this->heap, &callerSiteBt);

    // set args' values
    unsigned pos = /* dst + fnc */ 2;
    BOOST_FOREACH(int arg, args) {
        if (opList.size() <= pos)
            // giving up, perhaps a variable count of args?
            break;

        const struct cl_operand &op = opList[pos++];
        srcProc.setLocation(this->lw);
        this->proc->setLocation(this->lw);

        const TValueId val = srcProc.heapValFromOperand(op);
        SE_BREAK_IF(VAL_INVALID == val);

        // cVar lookup
        const CVar cVar(arg, this->nestLevel);
        const TObjId lhs = this->heap->objByCVar(cVar);
        SE_BREAK_IF(OBJ_INVALID == lhs);

        // set arg's value
        this->proc->objSetValue(lhs, val);
    }
}

SymCallCtx* SymCallCache::Private::getCallCtx(int uid, const SymHeap &heap) {
    TFncSeq seq;
    if (this->bt->hasRecursiveCall())
        // FIXME: this seems to be a silly limitation -- we require the call
        // sequence to match as long as any there is any recursion involved
        seq = this->bt->getFncSequence();

    // 1st level cache lookup
    TCacheRow &row = this->cache[seq];

    // 2nd level cache lookup
    // FIXME: We rely on the assumption that gatherCVars() returns the variables
    //        always in the same order, however SymHeapTyped API does not
    //        guarantee anything like that.  Luckily it should cause nothing
    //        evil in the analysis, only some unnecessary bloat of cache...
    SymHeap::TContCVar cVars;
    heap.gatherCVars(cVars);
    TCacheCol &col = row[cVars];

    // 3rd level cache lookup
    PerFncCache &pfc = col[uid];
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

SymCallCtx& SymCallCache::getCallCtx(SymHeap                    heap,
                                     const CodeStorage::Fnc     &fnc,
                                     const CodeStorage::Insn    &insn)
{
    using namespace CodeStorage;

    // check insn validity
    const TOperandList &opList = insn.operands;
    SE_BREAK_IF(CL_INSN_CALL != insn.code || opList.size() < 2);

    // create SymProc and update the location info
    SymProc proc(heap, d->bt);
    proc.setLocation((d->lw = &insn.loc));
    d->heap = &heap;
    d->proc = &proc;

    // re-initialize OBJ_RETURN
    heap.objDestroy(OBJ_RETURN);

    // store Fnc ought to be called
    d->fnc = &fnc;
    CL_DEBUG_MSG(d->lw, "SymCallCache is looking for "
            << nameOf(fnc) << "()...");

    // check recursion depth (if any)
    const int uid = uidOf(fnc);
    d->nestLevel = d->bt->countOccurrencesOfFnc(uid);
    if (1 != d->nestLevel) {
        CL_WARN_MSG(d->lw, "support of call recursion is not stable yet");
        CL_NOTE_MSG(d->lw, "nestLevel is " << d->nestLevel);
    }

    // start with gl variables as the cut
    SymHeap::TContCVar cut(d->glVars);

    // initialize local variables of the called fnc
    d->createStackFrame(cut);
    d->setCallArgs(opList);

    // prune heap
    SymHeap surround;
    heap.pack();
    splitHeapByCVars(d->bt, &heap, cut, &surround);
    
    // get either an existing ctx, or create a new one
    SymCallCtx *ctx = d->getCallCtx(uid, heap);
    if (!ctx)
        SE_TRAP;

    // not flushed yet
    ctx->d->flushed     = false;

    // keep some properties later required to process the results
    ctx->d->dst         = &opList[/* dst */ 0];
    ctx->d->nestLevel   = d->nestLevel;
    ctx->d->surround    = surround;
    return *ctx;
}
