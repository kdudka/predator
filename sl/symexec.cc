/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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
#include "symexec.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symabstract.hh"
#include "symbt.hh"
#include "symcall.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <set>
#include <sstream>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

// utilities
namespace {

void createGlVar(SymHeap &heap, const CodeStorage::Var &var) {
    // create the corresponding heap object
    const struct cl_type *clt = var.clt;
    const CVar cVar(var.uid, /* gl variable */ 0);
    const TObjId obj = heap.objCreate(clt, cVar);

    // now attempt to initialize the variable since it is a global/static var
    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_INT:
        case CL_TYPE_PTR:
            heap.objSetValue(obj, VAL_NULL);
            break;

        case CL_TYPE_BOOL:
            heap.objSetValue(obj, VAL_FALSE);
            break;

        case CL_TYPE_STRUCT:
            // TODO: go through the struct recursively and initialize
            // fall through!

        default:
            // only a few types are supported in case of gl variables for now
            TRAP;
    }
}

void createGlVars(SymHeap &heap, const CodeStorage::Storage &stor) {
    using namespace CodeStorage;
    BOOST_FOREACH(const Var &var, stor.vars) {
        if (VAR_GL == var.code) {
            const LocationWriter lw(&var.loc);
            CL_DEBUG_MSG(lw, "(g) creating global variable: #" << var.uid
                    << " (" << var.name << ")");
            createGlVar(heap, var);
        }
    }
}

} // namespace

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine
class SymExecEngine {
    public:
        SymExecEngine(const SymExec &se, const SymBackTrace &bt,
                      const SymHeap &src, SymHeapUnion &dst):
            stor_(se.stor()),
            params_(se.params()),
            bt_(bt),
            dst_(dst),
            block_(0),
            insnIdx_(0),
            heapIdx_(0),
            waiting_(false)
        {
            this->initEngine(src);
        }

    public:
        bool /* complete */ run();

        // TODO: describe the interface briefly
        const SymHeap*              callEntry() const;
        const CodeStorage::Insn*    callInsn() const;
        SymHeapUnion*               callResults();

    private:
        typedef const CodeStorage::Block                   *TBlock;
        typedef std::set<TBlock>                            TBlockSet;
        typedef std::map<TBlock, SymHeapScheduler>          TStateMap;

        const CodeStorage::Storage      &stor_;
        SymExecParams                   params_;
        const SymBackTrace              &bt_;
        SymHeapUnion                    &dst_;

        TStateMap                       stateMap_;
        TBlockSet                       todo_;
        TBlock                          block_;
        unsigned                        insnIdx_;
        unsigned                        heapIdx_;
        bool                            waiting_;

        SymHeapScheduler                localState_;
        SymHeapUnion                    nextLocalState_;
        LocationWriter                  lw_;

    private:
        void initEngine(const SymHeap &init);
        void execReturn();
        void updateState(const CodeStorage::Block *ofBlock,
                         TValueId valDst = VAL_INVALID,
                         TValueId valSrc = VAL_INVALID);
        void execCondInsn();
        void execTermInsn();
        bool execNontermInsn();
        void echoInsn();
        bool execInsn();
        bool execBlock();
};

// /////////////////////////////////////////////////////////////////////////////
// SymExecEngine implementation
void SymExecEngine::initEngine(const SymHeap &init)
{
    // look for fnc name
    const CodeStorage::Fnc *fnc = bt_.topFnc();
    const std::string &fncName = nameOf(*fnc);
    lw_ = &fnc->def.loc;
    CL_DEBUG_MSG(lw_, ">>> entering " << fncName << "()");

    // look for the entry block
    TBlock entry = fnc->cfg.entry();
    if (!entry) {
        CL_ERROR_MSG(lw_, fncName << ": " << "entry block not found");
        return;
    }

    // insert initial state to the corresponding union
    SymHeapUnion &huni = stateMap_[entry];
    huni.insert(init);

    // schedule the entry block for processing
    todo_.insert(entry);
}

void SymExecEngine::execReturn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &opList = insn->operands;
    if (1 != opList.size())
        TRAP;

    SymHeap heap(localState_[heapIdx_]);

    const struct cl_operand &src = opList[0];
    if (CL_OPERAND_VOID != src.code) {
        SymProc proc(heap, &bt_);
        proc.setLocation(lw_);

        const TValueId val = proc.heapValFromOperand(src);
        if (VAL_INVALID == val)
            TRAP;

        proc.objSetValue(OBJ_RETURN, val);
    }

    // commit one of the function results
    dst_.insert(heap);
}

void SymExecEngine::updateState(const CodeStorage::Block *ofBlock,
                                TValueId valDst, TValueId valSrc)
{
    // clone the current symbolic heap, as we are going to change it eventually
    SymHeap sh(localState_[heapIdx_]);

    if (VAL_INVALID != valDst && VAL_INVALID != valSrc)
        // replace an unknown value while traversing an unambiguous condition
        sh.valReplaceUnknown(valDst, valSrc);

    // time to consider abstraction
    abstractIfNeeded(sh);

    // update *target* state
    SymHeapUnion &huni = stateMap_[ofBlock];
    const size_t last = huni.size();
    huni.insert(sh);

    const std::string &name = ofBlock->name();

    // check if anything has changed
    if (huni.size() == last) {
        CL_DEBUG_MSG(lw_, "--- block " << name << " left intact");

    } else {
        const size_t last = todo_.size();

        // schedule for next wheel (if not already)
        todo_.insert(ofBlock);

        const bool already = (todo_.size() == last);
        CL_DEBUG_MSG(lw_, ((already) ? "-+-" : "+++")
                << " block " << name
                << ((already)
                    ? " changed, but already scheduled"
                    : " scheduled for next wheel"));
    }
}

void SymExecEngine::execCondInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &oplist = insn->operands;
    const CodeStorage::TTargetList &tlist = insn->targets;
    if (2 != tlist.size() || 1 != oplist.size())
        TRAP;

    // IF (operand) GOTO target0 ELSE target1

    const SymHeap &heap = localState_[heapIdx_];
    SymProc proc(const_cast<SymHeap &>(heap), &bt_);
    proc.setLocation(lw_);

    const TValueId val = proc.heapValFromOperand(oplist[0]);
    switch (val) {
        case VAL_TRUE:
            CL_DEBUG_MSG(lw_, ".T. CL_INSN_COND got VAL_TRUE");
            this->updateState(tlist[/* then label */ 0]);
            return;

        case VAL_FALSE:
            CL_DEBUG_MSG(lw_, ".F. CL_INSN_COND got VAL_FALSE");
            this->updateState(tlist[/* else label */ 1]);
            return;

        default:
            break;
    }

    // operand value is unknown, go to both targets

    const EUnknownValue code = heap.valGetUnknown(val);
    switch (code) {
        case UV_UNKNOWN:
            CL_DEBUG_MSG(lw_, "??? CL_INSN_COND got VAL_UNKNOWN");
            break;

        case UV_UNINITIALIZED:
            CL_WARN_MSG(lw_,
                    "conditional jump depends on uninitialized value");
            bt_.printBackTrace();
            break;

        case UV_DEREF_FAILED:
            // error should have been already emitted
            CL_DEBUG_MSG(lw_, "ignored VAL_DEREF_FAILED");
            break;

        case UV_KNOWN:
        case UV_ABSTRACT:
            TRAP;
            return;
    }

    this->updateState(tlist[/* then label */ 0], val, VAL_TRUE);
    this->updateState(tlist[/* else label */ 1], val, VAL_FALSE);
}

void SymExecEngine::execTermInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TTargetList &tlist = insn->targets;

    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_RET:
            this->execReturn();
            break;

        case CL_INSN_COND:
            this->execCondInsn();
            break;

        case CL_INSN_ABORT:
            CL_DEBUG_MSG(lw_, "CL_INSN_ABORT reached");
            break;

        case CL_INSN_JMP:
            if (1 == tlist.size()) {
                this->updateState(tlist[0]);
                break;
            }
            // go through!

        default:
            TRAP;
    }
}

bool /* handled */ SymExecEngine::execNontermInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // working area for non-terminal instructions
    SymHeap workingHeap(localState_[heapIdx_]);
    SymProc proc(workingHeap, &bt_);
    proc.setLocation(lw_);

    return proc.exec(nextLocalState_, *insn, params_.fastMode);
}

namespace {

void operandToStreamCstInt(std::ostream &str, const struct cl_operand &op) {
    const struct cl_cst &cst = op.data.cst;
    const int val = cst.data.cst_int.value;

    const enum cl_type_e code = op.type->code;
    switch (code) {
        case CL_TYPE_INT:
            str << "(int)" << val;
            break;

        case CL_TYPE_BOOL:
            str << (val ? "true" : "false");
            break;

        case CL_TYPE_PTR:
            if (!val) {
                str << "NULL";
                break;
            }
            // fall through!

        default:
            TRAP;
    }
}

void operandToStreamCst(std::ostream &str, const struct cl_operand &op) {
    const struct cl_cst &cst = op.data.cst;
    const enum cl_type_e code = cst.code;
    switch (code) {
        case CL_TYPE_INT:
            operandToStreamCstInt(str, op);
            break;

        case CL_TYPE_FNC: {
            const char *name = cst.data.cst_fnc.name;
            if (!name)
                TRAP;

            str << name;
            break;
        }

        case CL_TYPE_STRING: {
            const char *text = cst.data.cst_string.value;
            if (!text)
                TRAP;

            str << "\"" << text << "\"";
            break;
        }

        default:
            TRAP;
    }
}

void operandToStreamAcs(std::ostream &str, const struct cl_accessor *ac) {
    if (!ac)
        return;

    // FIXME: copy/pasted from cl_pp.cc
    if (ac && ac->code == CL_ACCESSOR_DEREF &&
            ac->next && ac->next->code == CL_ACCESSOR_ITEM)
    {
        ac = ac->next;
        str << "->" << ac->type->items[ac->data.item.id].name;
        ac = ac->next;
    }

    for (; ac; ac = ac->next) {
        enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_DEREF_ARRAY:
                str << " [...]";
                break;

            case CL_ACCESSOR_ITEM:
                str << "." << ac->type->items[ac->data.item.id].name;
                break;

            case CL_ACCESSOR_REF:
                if (!ac->next)
                    // already handled
                    break;
                // fall through!

            default:
                TRAP;
        }
    }
}

void operandToStreamVar(std::ostream &str, const struct cl_operand &op) {
    const struct cl_accessor *ac = op.accessor;

    // FIXME: copy/pasted from cl_pp.cc
    const struct cl_accessor *is_ref = ac;
    while (is_ref && (is_ref->next || is_ref->code != CL_ACCESSOR_REF))
        is_ref = is_ref->next;
    if (is_ref)
        str << "&";

    if (ac && ac->code == CL_ACCESSOR_DEREF &&
            (!ac->next || ac->next->code != CL_ACCESSOR_ITEM))
    {
        str << "*";
        ac = ac->next;
    }

    // obtain var ID and name (if any)
    const enum cl_operand_e code = op.code;
    const char *name = NULL;
    int uid = -1;
    switch (code) {
        case CL_OPERAND_REG:
            uid = op.data.reg.id;
            break;

        case CL_OPERAND_VAR:
            uid = op.data.var.id;
            name = op.data.var.name;
            break;

        default:
            TRAP;
            break;
    }

    // print var itself
    str << "#" << uid;
    if (name)
        str << ":" << name;

    // print all accessors excecpt CL_ACCESSOR_REF, which shloud have been
    // already handled
    operandToStreamAcs(str, ac);
}

void operandToStream(std::ostream &str, const struct cl_operand &op) {
    const enum cl_operand_e code = op.code;
    switch (code) {
        case CL_OPERAND_CST:
            operandToStreamCst(str, op);
            break;

        case CL_OPERAND_REG:
        case CL_OPERAND_VAR:
            operandToStreamVar(str, op);
            break;

        case CL_OPERAND_VOID:
            // this should have been handled elsewhere
        default:
            TRAP;
    }
}

void unOpToStream(std::ostream &str, int subCode,
                  const CodeStorage::TOperandList &opList)
{
    operandToStream(str, opList[/* dst */ 0]);
    str << " = ";

    const enum cl_unop_e code = static_cast<enum cl_unop_e>(subCode);
    switch (code) {
        case CL_UNOP_ASSIGN:
            break;

        case CL_UNOP_TRUTH_NOT:
            str << "!";
            break;
    }

    operandToStream(str, opList[/* src */ 1]);
}

void binOpToStream(std::ostream &str, int subCode,
                   const CodeStorage::TOperandList &opList)
{
    const enum cl_binop_e code = static_cast<enum cl_binop_e>(subCode);
    operandToStream(str, opList[/* dst */ 0]);
    str << " = (";
    operandToStream(str, opList[/* src1 */ 1]);

    // TODO: move this to cl API
    switch (code) {
        case CL_BINOP_EQ:           str << " == "; break;
        case CL_BINOP_NE:           str << " != "; break;
        case CL_BINOP_LT:           str << " < ";  break;
        case CL_BINOP_GT:           str << " > ";  break;
        case CL_BINOP_LE:           str << " <= "; break;
        case CL_BINOP_GE:           str << " >= "; break;
        case CL_BINOP_PLUS:         str << " + ";  break;
        case CL_BINOP_MINUS:        str << " - ";  break;
        default:
            TRAP;
    }

    operandToStream(str, opList[/* src2 */ 2]);
    str << ")";
}

void callToStream(std::ostream &str, const CodeStorage::TOperandList &opList) {
    const struct cl_operand &dst = opList[/* dst */ 0];
    if (CL_OPERAND_VOID != dst.code) {
        operandToStream(str, dst);
        str << " = ";
    }
    operandToStream(str, opList[/* fnc */ 1]);
    str << " (";
    for (unsigned i = /* dst + fnc */ 2; i < opList.size(); ++i)
    {
        if (2 < i)
            str << ", ";

        operandToStream(str, opList[i]);
    }
    str << ")";
}

void retToStream(std::ostream &str, const struct cl_operand &src) {
    str << "return";

    if (CL_OPERAND_VOID == src.code)
        return;

    str << " ";
    operandToStream(str, src);
}

} // namespace

void SymExecEngine::echoInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    const CodeStorage::TOperandList &opList = insn->operands;
    const CodeStorage::TTargetList &tList = insn->targets;
    std::ostringstream str;

    const enum cl_insn_e code = insn->code;
    switch (code) {
        case CL_INSN_UNOP:
            unOpToStream(str, insn->subCode, opList);
            break;

        case CL_INSN_BINOP:
            binOpToStream(str, insn->subCode, opList);
            break;

        case CL_INSN_CALL:
            callToStream(str, opList);
            break;

        case CL_INSN_RET:
            retToStream(str, opList[/* src */ 0]);
            break;

        case CL_INSN_COND:
            str << "if (";
            operandToStream(str, opList[/* src */ 0]);
            str << ") goto " << tList[/* then label */ 0]->name();
            str <<  " else " << tList[/* else label */ 1]->name();
            break;

        case CL_INSN_JMP:
            str << "goto " << tList[/* target */ 0]->name();
            break;

        case CL_INSN_ABORT:
            str << "abort";
            break;

        default:
            TRAP;
    }

    std::string echo(str.str());
    CL_DEBUG_MSG(lw_, "!!! executing insn #" << insnIdx_ << " ... " << echo);
}

bool /* complete */ SymExecEngine::execInsn() {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);

    // true for terminal instruction
    const bool isTerm = cl_is_term_insn(insn->code);

    if (!heapIdx_) {
        // let's begin with empty resulting heap union
        // TODO: implement SymHeapUnion::clear()
        nextLocalState_ = SymHeapUnion();
        this->echoInsn();
    }

    // go through the remainder of symbolic heaps corresponding to localState_
    const unsigned hCnt = localState_.size();
    for (/* we allow resume */; heapIdx_ < hCnt; ++heapIdx_) {
        if (localState_.isDone(heapIdx_))
            // for this particular symbolic heap, we already know the result and
            // the result is already included in the resulting state, skip it
            continue;

        if (1 < hCnt)
            CL_DEBUG_MSG(lw_, "*** processing heap #" << heapIdx_);

        if (isTerm) {
            // terminal insn
            this->execTermInsn();
            continue;
        }

        if (this->execNontermInsn())
            // regular heap instruction
            continue;

        // assuming CL_INSN_CALL; just suspend self and wait for results...
        ++heapIdx_;
        return false;
    }

    // completed execution of the given insn
    heapIdx_ = 0;
    return true;
}

bool /* complete */ SymExecEngine::execBlock() {
    const std::string &name = block_->name();

    // state valid for the entry of this BB
    SymHeapScheduler &origin = stateMap_[block_];
    const int origCnt = origin.size();

    if (insnIdx_ || heapIdx_) {
        // some debugging output of the resume process
        const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
        const LocationWriter lw(&insn->loc);
        CL_DEBUG_MSG(lw_, "___ we are back in " << name
                << ", insn #" << insnIdx_
                << ", heap #" << (heapIdx_ - 1));
    }

    if (!insnIdx_)
        // fresh run, let's initialize the local state by the BB entry
        localState_ = origin;

    // go through the remainder of BB insns
    for (; insnIdx_ < block_->size(); ++insnIdx_) {

        const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
        if (0 < insn->loc.line)
            // update location info
            lw_ = &insn->loc;

        // execute current instruction
        if (!this->execInsn())
            // function call reached, we should stand by
            return false;

        // swap states in order to be ready for next insn
        localState_ = nextLocalState_;
    }

    // Mark symbolic heaps that have been processed as done. They will be
    // omitted on the next call of execBlock() for the same BB since there is no
    // chance to get different results for the same symbolic heaps on the input.
    // NOTE: We don't know whether origCnt == origin.size() at this point, but
    //       only                  origCnt <= origin.size()
    for (int h = 0; h < origCnt; ++h)
        origin.setDone(h);

    // the whole block is processed now
    CL_DEBUG_MSG(lw_, "___ completed batch for " << name);
    insnIdx_ = 0;
    return true;
}

bool /* complete */ SymExecEngine::run() {
    const CodeStorage::Fnc *fnc = bt_.topFnc();
    const LocationWriter lw(&fnc->def.loc);

    if (waiting_) {
        // we're on the way from a just completed function call...
        if (!this->execBlock())
            // ... and we've just hit another one
            return false;
    }
    else {
        // we have a fresh instance of SymExecEngine
        waiting_ = true;
        if (1 != todo_.size())
            // protocol error
            TRAP;
    }

    // main loop of SymExecEngine
    while (!todo_.empty()) {
        // FIXME: take BBs in some reasonable order instead
        TBlockSet::iterator i = todo_.begin();
        block_ = *i;
        todo_.erase(i);

        const std::string &name = block_->name();
        const CodeStorage::Insn *first = block_->operator[](0);
        const LocationWriter lw(&first->loc);
        CL_DEBUG_MSG(lw_, "___ entering " << name);
        insnIdx_ = 0;
        heapIdx_ = 0;

        // process the basic block until the first function call
        if (!this->execBlock())
            // function call reached, suspend the execution for now
            return false;
    }

    // we are done with this function
    CL_DEBUG_MSG(lw, "<<< leaving " << nameOf(*fnc) << "()");
    waiting_ = false;
    return true;
}

const SymHeap* SymExecEngine::callEntry() const {
    if (heapIdx_ < 1)
        TRAP;

    return &localState_[heapIdx_ - /* already incremented for next wheel */ 1];
}

const CodeStorage::Insn* SymExecEngine::callInsn() const {
    const CodeStorage::Insn *insn = block_->operator[](insnIdx_);
    if (CL_INSN_CALL != insn->code)
        // protocol error
        TRAP;

    return insn;
}

SymHeapUnion* SymExecEngine::callResults() {
    return &nextLocalState_;
}

// /////////////////////////////////////////////////////////////////////////////
// SymExec implementation
struct StackItem;
struct SymExec::Private {
    SymExec                     &se;
    const CodeStorage::Storage  &stor;
    SymExecParams               params;
    SymHeapUnion                stateZero;
    SymBackTrace                bt;
    SymCallCache                callCache;

    Private(SymExec &se_, const CodeStorage::Storage &stor_):
        se(se_),
        stor(stor_),
        bt(stor_),
        callCache(&bt)
    {
    }

    const CodeStorage::Fnc* resolveCallInsn(SymHeap                     entry,
                                            const CodeStorage::Insn     &insn,
                                            SymHeapUnion                &dst);

    SymExecEngine* createEngine(SymCallCtx &ctx);

    void execLoop(const StackItem &item);
};

SymExec::SymExec(const CodeStorage::Storage &stor, const SymExecParams &params):
    d(new Private(*this, stor))
{
    d->params = params;

    // create the initial state, consisting of global/static variables
    SymHeap init;
    createGlVars(init, stor);
    d->stateZero.insert(init);
}

SymExec::~SymExec() {
    delete d;
}

const CodeStorage::Storage& SymExec::stor() const {
    return d->stor;
}

const SymExecParams& SymExec::params() const {
    return d->params;
}

const CodeStorage::Fnc* SymExec::Private::resolveCallInsn(
        SymHeap                     heap,
        const CodeStorage::Insn     &insn,
        SymHeapUnion                &results)
{
    const CodeStorage::TOperandList &opList = insn.operands;
    if (CL_INSN_CALL != insn.code || opList.size() < 2)
        TRAP;

    // look for Fnc ought to be called
    SymProc proc(heap, &this->bt);
    const LocationWriter lw(&insn.loc);
    proc.setLocation(lw);
    const int uid = proc.fncFromOperand(opList[/* fnc */ 1]);
    const CodeStorage::Fnc *fnc = this->stor.fncs[uid];
    if (!fnc)
        // unable to resolve Fnc by UID
        TRAP;

    if (SE_MAX_CALL_DEPTH < this->bt.size()) {
        CL_ERROR_MSG(lw, "call depth exceeds SE_MAX_CALL_DEPTH"
                << " (" << SE_MAX_CALL_DEPTH << ")");
        goto fail;
    }

    if (CL_OPERAND_VOID == fnc->def.code) {
        CL_WARN_MSG(lw, "ignoring call of undefined function");
        goto fail;
    }

    // enter backtrace
    this->bt.pushCall(uid, lw);
    return fnc;

fail:
    // something wrong happened, print the backtrace
    this->bt.printBackTrace();

    const struct cl_operand dst = opList[/* dst */ 0];
    if (CL_OPERAND_VOID != dst.code) {
        // set return value to unknown
        const TValueId val = heap.valCreateUnknown(UV_UNKNOWN, dst.type);
        const TObjId obj = proc.heapObjFromOperand(dst);
        heap.objSetValue(obj, val);
    }

    // call failed, so that we have exactly one resulting heap
    results.insert(heap);
    return 0;
}

SymExecEngine* SymExec::Private::createEngine(SymCallCtx &ctx) {
    return new SymExecEngine(
            this->se,
            this->bt,
            ctx.entry(),
            ctx.rawResults());
}

struct StackItem {
    SymCallCtx      *ctx;
    SymExecEngine   *eng;
    SymHeapUnion    *dst;
};

void SymExec::Private::execLoop(const StackItem &item) {
    using CodeStorage::Fnc;

    // run-time stack
    typedef std::stack<StackItem> TStack;
    TStack rtStack;
    rtStack.push(item);

    // main loop
    while (!rtStack.empty()) {
        const StackItem &item = rtStack.top();
        SymExecEngine *engine = item.eng;

        // do as much as we can at the current call level
        if (engine->run()) {
            // call done at this level
            item.ctx->flushCallResults(*item.dst);
            this->bt.popCall();

            // remove top of the stack
            delete engine;
            rtStack.pop();

            // wake up the caller (if any)
            continue;
        }

        // function call requested
        // --> we need to nest unless the computed result is already available
        const SymHeap &entry = *engine->callEntry();
        const CodeStorage::Insn &insn = *engine->callInsn();
        const Fnc *fnc = this->resolveCallInsn(entry, insn, *item.dst);
        if (!fnc)
            // the error message should have been already emitted, but there
            // are probably some unknown values in the result; now wake up
            // the caller
            continue;

        // call cache lookup
        SymCallCtx &ctx = this->callCache.getCallCtx(entry, insn);
        if (!ctx.needExec()) {
            // call cache hit
            const LocationWriter lw(this->bt.topCallLoc());
            CL_DEBUG_MSG(lw, "(x) call of function optimized out: "
                    << nameOf(*fnc) << "()");

            // use the cached result
            ctx.flushCallResults(*engine->callResults());

            // leave backtrace
            this->bt.popCall();

            // wake up the caller
            continue;
        }

        // prepare a new run-time stack item for the call
        StackItem next;
        next.ctx = &ctx;
        next.eng = this->createEngine(ctx);

        // pass the result back to the caller as soon as we have one
        next.dst = item.eng->callResults();

        // perform the call now!
        rtStack.push(next);
    }
}

void SymExec::exec(const CodeStorage::Fnc &fnc, SymHeapUnion &results) {
    // go through all symbolic heaps of the initial state, merging the results
    // all together
    BOOST_FOREACH(const SymHeap &heap, d->stateZero) {
        if (d->bt.size())
            // *** bt offset detected ***
            TRAP;

        // initialize backtrace
        d->bt.pushCall(uidOf(fnc), &fnc.def.loc);

        // XXX: synthesize CL_INSN_CALL
        CodeStorage::Insn insn;
        insn.stor = fnc.stor;
        insn.code = CL_INSN_CALL;
        insn.loc  = fnc.def.loc;
        insn.operands.resize(2);
        insn.operands[1] = fnc.def;

        // get call context for the root function
        SymCallCtx &ctx = d->callCache.getCallCtx(heap, insn);
        if (!ctx.needExec()) {
            // not likely to happen in the way that SymExec is currently used
            CL_WARN_MSG(d->bt.topCallLoc(), "(x) root function optimized out: "
                    << nameOf(fnc) << "()");

            ctx.flushCallResults(results);
            continue;
        }

        // root stack item
        StackItem si;
        si.ctx = &ctx;
        si.eng = d->createEngine(ctx);
        si.dst = &results;

        // root call
        d->execLoop(si);
    }
}
