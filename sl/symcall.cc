/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "symcall.hh"

#include "cl_msg.hh"
#include "location.hh"
#include "storage.hh"
#include "symheap.hh"
#include "symproc.hh"
#include "symstate.hh"

#include <vector>

#include <boost/foreach.hpp>

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCtx
struct SymCallCtx::Private {
    IBtPrinter                  *bt;
    SymHeap                     heap;
    const CodeStorage::Fnc      *fnc;
    const struct cl_operand     *dst;
    SymHeapUnion                rawResults;

    void assignReturnValue();
    void destroyStackFrame();
};

SymCallCtx::SymCallCtx():
    d(new Private)
{
}

SymCallCtx::~SymCallCtx() {
    delete d;
}

bool SymCallCtx::needExec() const {
    // TODO: optimization
    return true;
}

const SymHeap& SymCallCtx::entry() const {
    // TODO: optimization
    return d->heap;
}

SymHeapUnion& SymCallCtx::rawResults() {
    return d->rawResults;
}

void SymCallCtx::Private::assignReturnValue() {
    const cl_operand &op = *this->dst;
    if (CL_OPERAND_VOID == op.code)
        // we're done for a function returning void
        return;

    // go through results and perform assignment of the return value
    BOOST_FOREACH(SymHeap &res, this->rawResults) {
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

void SymCallCtx::Private::destroyStackFrame() {
    using namespace CodeStorage;
    const Fnc &ref = *this->fnc;

#if DEBUG_SE_STACK_FRAME
    const LocationWriter lw(&ref.def.loc);
    CL_DEBUG_MSG(lw, "<<< destroying stack frame of "
            << nameOf(ref) << "():");

    int hCnt = 0;
#endif

    BOOST_FOREACH(SymHeap &res, this->rawResults) {
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

                const TObjId obj = res.objByCVar(var.uid);
                if (obj < 0)
                    TRAP;

                proc.setLocation(lw);
                proc.destroyObj(obj);
            }
        }

        // We need to look for junk since there can be a function returning an
        // allocated object. The ignoring the return value on the caller's side
        // can trigger a memory leak.
        // TODO: write a test-case for this!
        proc.destroyObj(OBJ_RETURN);
    }
}

void SymCallCtx::flushCallResults(SymHeapUnion &dst) {
    // TODO: optimization
    d->assignReturnValue();
    d->destroyStackFrame();
    dst.insert(d->rawResults);
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymCallCache
struct SymCallCache::Private {
    IBtPrinter                  *bt;
    LocationWriter              lw;
    SymHeap                     *heap;
    SymHeapProcessor            *proc;
    const CodeStorage::Fnc      *fnc;

    /* XXX */ std::vector<SymCallCtx *> cont;

    void createStackFrame();
    void setCallArgs(const CodeStorage::TOperandList &opList);
};

SymCallCache::SymCallCache(IBtPrinter *bt):
    d(new Private)
{
    d->bt = bt;
}

SymCallCache::~SymCallCache() {
    BOOST_FOREACH(SymCallCtx *ctx, d->cont) {
        delete ctx;
    }
    delete d;
}

void SymCallCache::Private::createStackFrame() {
    using namespace CodeStorage;
    const Fnc &ref = *this->fnc;

#if DEBUG_SE_STACK_FRAME
    CL_DEBUG_MSG(this->lw,
            ">>> creating stack frame for " << nameOf(ref) << "()");
#endif

    BOOST_FOREACH(const int uid, ref.vars) {
        const Var &var = ref.stor->vars[uid];
        if (isOnStack(var)) {
#if DEBUG_SE_STACK_FRAME
            LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, ">>> creating stack variable: #" << var.uid
                    << " (" << var.name << ")" );
#endif

            this->heap->objCreate(var.clt, var.uid);
        }
    }
}

void SymCallCache::Private::setCallArgs(const CodeStorage::TOperandList &opList)
{
    // get called fnc's args
    const CodeStorage::TArgByPos &args = this->fnc->args;
    if (args.size() + 2 != opList.size())
        TRAP;

    // set args' values
    int pos = /* dst + fnc */ 2;
    BOOST_FOREACH(int arg, args) {
        const struct cl_operand &op = opList[pos++];
        this->proc->setLocation(this->lw);

        const TValueId val = this->proc->heapValFromOperand(op);
        if (VAL_INVALID == val)
            TRAP;

        const TObjId lhs = this->heap->objByCVar(arg);
        if (OBJ_INVALID == lhs)
            TRAP;

        // set arg's value
        this->proc->heapSetVal(lhs, val);
    }
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

    // initialize local variables of the called fnc
    d->createStackFrame();
    d->setCallArgs(opList);

    // TODO: prune heap
    // TODO: cache lookup

    // XXX
    SymCallCtx *ctx = new SymCallCtx;
    ctx->d->bt   = d->bt;
    ctx->d->fnc  = d->fnc;
    ctx->d->heap = *d->heap;
    ctx->d->dst  = &dst;
    d->cont.push_back(ctx);
    return *ctx;
}
