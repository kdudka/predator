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

#include "config_cl.h"
#include "cl_storage.hh"

#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "builtins.hh"
#include "util.hh"

#include <cstring>
#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

namespace CodeStorage {
    /**
     * @param str dst/src to call strdup(3) for
     */
    void dupString(const char *&str) {
        str = (str)
            ? strdup(str)
            : 0;
    }

    /**
     * @param str Legacy string to be freed, usually formerly given by dupString
     */
    void freeString(const char *str) {
        free(const_cast<char *>(str));
    }

    /**
     * @param fnc An arbitrary function we should call on any (valid) string
     * inside struct cl_cst object.
     * @param cst An instance of struct cl_cst being processed.
     */
    template <typename TFnc>
    void handleCstStrings(TFnc fnc, struct cl_cst &cst) {
        enum cl_type_e code = cst.code;
        switch (code) {
            case CL_TYPE_FNC:
                fnc(cst.data.cst_fnc.name);
                break;

            case CL_TYPE_STRING:
                fnc(cst.data.cst_string.value);
                break;

            default:
                break;
        }
    }

    /**
     * @param fnc An arbitrary function we should call on any (valid) string
     * inside struct cl_operand object.
     * @param op An instance of struct cl_operand being processed.
     */
    template <typename TFnc>
    void handleOperandStrings(TFnc fnc, struct cl_operand *op) {
        enum cl_operand_e code = op->code;
        switch (code) {
            // TODO
#if 0
            case CL_OPERAND_VAR:
                fnc(op->data.var.name);
                break;
#endif

            case CL_OPERAND_CST:
                handleCstStrings(fnc, op->data.cst);
                break;

            default:
                break;
        }
    }

    /**
     * clone a chain of cl_accessor objects and eventually push all array
     * indexes to given stack
     * @param dst Where to store just cloned cl_accessor chain.
     * @param src The chain of cl_accessor objects being cloned.
     * @param opStack Stack to push all array indexes to.
     */
    template <class TStack>
    void cloneAccessor(struct cl_accessor **dst, const struct cl_accessor *src,
                       TStack &opStack)
    {
        while (src) {
            // clone current cl_accessor object
            *dst = new struct cl_accessor(*src);

            if (CL_ACCESSOR_DEREF_ARRAY == src->code) {
                // clone array index
                struct cl_operand const *idxSrc = src->data.array.index;
                struct cl_operand *idxDst = new struct cl_operand(*idxSrc);
                (*dst)->data.array.index = idxDst;

                // schedule array index as an operand for the next wheel
                push(opStack, idxDst, idxSrc);
            }

            // move to next cl_accessor object (if any)
            src = src->next;
            dst = &(*dst)->next;
        }
    }

    /**
     * deep copy of a cl_operand object
     * @note FIXME: I guess this will need a debugger first :-)
     */
    void storeOperand(struct cl_operand &dst, const struct cl_operand *src) {
        // shallow copy
        dst = *src;

        // clone objects recursively using std::stack
        typedef std::pair<struct cl_operand *, const struct cl_operand *> TPair;
        typedef std::stack<TPair> TStack;
        TStack opStack;
        push(opStack, &dst, src);
        while (!opStack.empty()) {
            struct cl_operand *cDst;
            struct cl_operand const *cSrc;
            boost::tie(cDst, cSrc) = opStack.top();
            opStack.pop();

            if (CL_OPERAND_VOID == cSrc->code)
                // no operand here
                continue;

            // clone list of cl_accessor objects
            // and schedule all array indexes for the next wheel eventually
            cloneAccessor(&cDst->accessor, cSrc->accessor, opStack);

            // duplicate all strings
            handleOperandStrings(dupString, cDst);
        }
    }

    /**
     * free all data allocated previously by storeOperand()
     */
    void releaseOperand(struct cl_operand &ref) {
        if (CL_OPERAND_VOID == ref.code)
            // nothing to free
            return;

        // use std::stack to avoid recursion
        std::stack<struct cl_operand *> opStack;
        opStack.push(&ref);
        while (!opStack.empty()) {
            struct cl_operand *op = opStack.top();
            opStack.pop();

            // remove all duplicated strings
            handleOperandStrings(freeString, op);

            // destroy cl_accessor objects recursively
            struct cl_accessor *ac = op->accessor;
            while (ac) {
                struct cl_accessor *next = ac->next;
                if (CL_ACCESSOR_DEREF_ARRAY == ac->code)
                    opStack.push(ac->data.array.index);

                delete ac;
                ac = next;
            }

            if (op != &ref)
                // free cloned array index
                delete op;
        }

        // all accessors freed
        ref.accessor = 0;
    }

    void storeLabel(struct cl_operand &op, const struct cl_insn *cli) {
        const char *name = cli->data.insn_label.name;
        struct cl_operand tpl;
        tpl.code = CL_OPERAND_VOID;
        tpl.scope                               = CL_SCOPE_FUNCTION;
        tpl.type                                = /* FIXME */ 0;
        tpl.accessor                            = 0;

        if (name) {
            tpl.code                            = CL_OPERAND_CST;
            tpl.data.cst.code                   = CL_TYPE_STRING;
            tpl.data.cst.data.cst_string.value  = name;
        }

        storeOperand(op, &tpl);
    }

    Insn* createInsn(const struct cl_insn *cli, ControlFlow *cfg) {
        enum cl_insn_e code = cli->code;

        Insn *insn = new Insn;
        insn->code = cli->code;
        insn->loc = cli->loc;

        TOperandList &operands = insn->operands;
        TTargetList &targets = insn->targets;

        switch (code) {
            case CL_INSN_NOP:
                CL_BREAK_IF("createInsn() got CL_INSN_NOP, why?");
                break;

            case CL_INSN_JMP:
                targets.push_back(cfg->operator[](cli->data.insn_jmp.label));
                break;

            case CL_INSN_COND:
                operands.resize(1);
                storeOperand(operands[0], cli->data.insn_cond.src);

                targets.resize(2);
                targets[0] = cfg->operator[](cli->data.insn_cond.then_label);
                targets[1] = cfg->operator[](cli->data.insn_cond.else_label);
                break;

            case CL_INSN_RET:
                operands.resize(1);
                storeOperand(operands[0], cli->data.insn_ret.src);
                // fall through!

            case CL_INSN_ABORT:
                break;

            case CL_INSN_UNOP:
                insn->subCode = static_cast<int> (cli->data.insn_unop.code);
                operands.resize(2);
                storeOperand(operands[0], cli->data.insn_unop.dst);
                storeOperand(operands[1], cli->data.insn_unop.src);
                break;

            case CL_INSN_BINOP:
                insn->subCode = static_cast<int> (cli->data.insn_binop.code);
                operands.resize(3);
                storeOperand(operands[0], cli->data.insn_binop.dst);
                storeOperand(operands[1], cli->data.insn_binop.src1);
                storeOperand(operands[2], cli->data.insn_binop.src2);
                break;

            case CL_INSN_CALL:
                CL_BREAK_IF("createInsn() got CL_INSN_CALL, why?");
                break;

            case CL_INSN_SWITCH:
                CL_BREAK_IF("createInsn() got CL_INSN_SWITCH, why?");
                break;

            case CL_INSN_LABEL:
                operands.resize(1);
                storeLabel(operands[0], cli);
                break;
        }

        return insn;
    }

    void destroyInsn(Insn *insn) {
        BOOST_FOREACH(struct cl_operand &op, insn->operands) {
            releaseOperand(op);
        }
        delete insn;
    }

    void destroyBlock(Block *bb) {
        BOOST_FOREACH(const Insn *insn, *bb) {
            destroyInsn(const_cast<Insn *>(insn));
        }
        delete bb;
    }

    void destroyFnc(Fnc *fnc) {
        releaseOperand(fnc->def);
        BOOST_FOREACH(const Block *bb, fnc->cfg) {
            destroyBlock(const_cast<Block *>(bb));
        }
        delete fnc->cgNode;
        delete fnc;
    }

    void releaseStorage(Storage &stor) {
        BOOST_FOREACH(const Fnc *fnc, stor.fncs) {
            destroyFnc(const_cast<Fnc *>(fnc));
        }
    }
}

using namespace CodeStorage;

struct ClStorageBuilder::Private {
    Storage     stor;
    const char  *file;
    Fnc         *fnc;
    Block       *bb;
    Insn        *insn;
    bool        preventRefOps;

    Private():
        file(0),
        fnc(0),
        bb(0),
        insn(0),
        preventRefOps(0)
    {
    }

    typedef struct cl_operand TOp;

    void digInitials(const TOp *);
    bool digOperandVar(const TOp *, bool isArgDecl);
    void digOperandCst(const struct cl_operand *);
    void digOperand(const TOp *);
    void openInsn(Insn *);
    void closeInsn();
};

ClStorageBuilder::ClStorageBuilder():
    d(new Private)
{
}

ClStorageBuilder::~ClStorageBuilder()
{
    releaseStorage(d->stor);
    delete d;
}

void ClStorageBuilder::acknowledge()
{
    this->run(d->stor);
}

void ClStorageBuilder::Private::digInitials(const TOp *op)
{
    const int id = varIdFromOperand(op);
    const struct cl_var *clv = op->data.var;

    const struct cl_initializer *initial;
    for (initial = clv->initial; initial; initial = initial->next) {
        Insn *insn = createInsn(&initial->insn, /* cfg */ 0);
        insn->stor = &stor;

        // initializer instructions are not associated with any basic block
        insn->bb = 0;

        // NOTE: keeping a reference for this may cause a SIGSEGV or lockup
        stor.vars[id].initials.push_back(insn);

        BOOST_FOREACH(const struct cl_operand &op, insn->operands)
            this->digOperand(&op);
    }
}

EVar varCodeByScope(const enum cl_scope_e scope, const bool isArgDecl)
{
    switch (scope) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
            return VAR_GL;

        case CL_SCOPE_FUNCTION:
            return (isArgDecl)
                ? VAR_FNC_ARG
                : VAR_LC;

        default:
            CL_BREAK_IF("varCodeByScope() got an invalid scope code");
            return VAR_VOID;
    }
}

bool ClStorageBuilder::Private::digOperandVar(const TOp *op, bool isArgDecl)
{
    const int id = varIdFromOperand(op);

    // mark as used in the current function
    this->fnc->vars.insert(id);

    // lookup by var ID
    Var &var = stor.vars[id];
    if (VAR_VOID != var.code)
        // already processed
        return false;

    const enum cl_scope_e scope = op->scope;
    const EVar code = varCodeByScope(scope, isArgDecl);
    var = Var(code, op);

    const std::string &varName = var.name;
    const char *fileName = var.loc.file;

    switch (scope) {
        case CL_SCOPE_GLOBAL:
            if (!varName.empty())
                stor.varNames.glNames[varName] = id;
            break;

        case CL_SCOPE_STATIC:
            if (fileName && !varName.empty())
                stor.varNames.lcNames[fileName][varName] = id;
            break;

        default:
            break;
    }

    return true;
}

void ClStorageBuilder::Private::digOperandCst(const struct cl_operand *op)
{
    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        // we are interested only in fncs for now
        return;

    // check scope
    enum cl_scope_e scope = op->scope;
    switch (scope) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
            break;
        default:
            // unexpected scope for fnc
            CL_TRAP;
    }

    // create a place-holder if needed
    const int uid = cst.data.cst_fnc.uid;
    Fnc *fnc = stor.fncs[uid];
    fnc->stor = &stor;

    // store fnc declaration if not already
    struct cl_operand &def = fnc->def;
    if (CL_OPERAND_VOID == def.code)
        storeOperand(def, op);

    // select the appropriate name mapping by scope
    NameDb::TNameMap &nameMap = (CL_SCOPE_GLOBAL == scope)
        ? stor.fncNames.glNames
        : stor.fncNames.lcNames[this->file];

    // store name-to-uid mapping if needed
    const char *name = cst.data.cst_fnc.name;
    nameMap[name] = uid;
}

void ClStorageBuilder::Private::digOperand(const TOp *op)
{
    if (!op || CL_OPERAND_VOID == op->code)
        return;

    // read base type
    TypeDb &typeDb = stor.types;
    readTypeTree(typeDb, op->type);

    // read type of each array index in the chain
    const struct cl_accessor *ac = op->accessor;
    for (; ac; ac = ac->next) {
        readTypeTree(typeDb, ac->type);

        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            readTypeTree(typeDb, ac->data.array.index->type);
    }

    enum cl_operand_e code = op->code;
    switch (code) {
        case CL_OPERAND_VAR:
            // store variable's metadata
            break;

        case CL_OPERAND_CST:
            this->digOperandCst(op);
            return;

        case CL_OPERAND_VOID:
        default:
            CL_BREAK_IF("invalid call of digOperand()");
            return;
    }

    const bool skipVarInit = !this->digOperandVar(op, /* isArgDecl */ false);

    ac = op->accessor;
    if (!preventRefOps && ac && ac->code != CL_ACCESSOR_DEREF &&
            seekRefAccessor(ac))
    {
        // we are taking a reference to the variable by this operand!
        const int id = varIdFromOperand(op);
        stor.vars[id].mayBePointed = true;
    }

    if (skipVarInit)
        return;

    this->digInitials(op);
}

void ClStorageBuilder::Private::openInsn(Insn *newInsn)
{
    // set pointer to the owning instance of Storage
    newInsn->stor = &this->stor;

    // check there is no insn already opened
    CL_BREAK_IF(insn);
    CL_BREAK_IF(!bb);

    bb->append(newInsn);
    insn = newInsn;
}

void ClStorageBuilder::Private::closeInsn()
{
    TOperandList &operands = insn->operands;
    BOOST_FOREACH(const struct cl_operand &op, operands) {
        this->digOperand(&op);
    }

    TTargetList &tlist = insn->targets;
    BOOST_FOREACH(const Block *target, tlist) {
        const_cast<Block *>(target)->appendPredecessor(this->bb);
    }

    insn->killPerTarget.resize(insn->targets.size());

    // let it honestly crash if callback sequence is incorrect since this should
    // have already been caught by ClfCbSeqChk cl filter
    insn = 0;
}

void ClStorageBuilder::file_open(const char *fileName)
{
    if (!fileName)
        CL_TRAP;

    d->file = fileName;
}

void ClStorageBuilder::file_close()
{
    // let it honestly crash if callback sequence is incorrect since this should
    // have already been caught by ClfCbSeqChk cl filter
    d->file = 0;
}

void ClStorageBuilder::fnc_open(const struct cl_operand *op)
{
    if (CL_OPERAND_CST != op->code)
        CL_TRAP;

    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        CL_TRAP;

    // store file where the fnc originates from
    const char *file = op->data.cst.data.cst_fnc.loc.file;
    CL_BREAK_IF(!file);
    d->file = file;

    // set current fnc
    int uid = cst.data.cst_fnc.uid;
    Fnc *fnc = d->stor.fncs[uid];
    fnc->stor = &d->stor;
    d->fnc = fnc;

    // store fnc definition
    struct cl_operand &def = fnc->def;
    storeOperand(def, op);
    d->digOperand(&def);

    // let it honestly crash if callback sequence is incorrect since this should
    // have already been caught by ClfCbSeqChk cl filter
    d->bb = 0;
}

void ClStorageBuilder::fnc_arg_decl(int pos, const struct cl_operand *op)
{
    if (CL_OPERAND_VAR != op->code)
        CL_TRAP;

    d->digOperandVar(op, /* isArgDecl */ true);

    TArgByPos &args = d->fnc->args;
    const int uid = varIdFromOperand(op);
    args.push_back(uid);
    CL_BREAK_IF(static_cast<int>(args.size()) != pos);
    (void) pos;
}

void ClStorageBuilder::fnc_close()
{
    // let it honestly crash if callback sequence is incorrect since this should
    // have already been caught by ClfCbSeqChk cl filter
    d->fnc = 0;
}

void ClStorageBuilder::bb_open(const char *bb_name)
{
    ControlFlow &cfg = d->fnc->cfg;
    d->bb = cfg[bb_name];
}

void ClStorageBuilder::insn(const struct cl_insn *cli)
{
    if (!d->bb)
        // FIXME: this simply ignores 'jump to entry' insn
        return;

    // serialize given insn
    Insn *insn = createInsn(cli, &d->fnc->cfg);
    d->openInsn(insn);

    // current insn is actually already complete
    // in case of one-shot insn
    d->closeInsn();
}

void ClStorageBuilder::insn_call_open(
    const struct cl_loc     *loc,
    const struct cl_operand *dst,
    const struct cl_operand *fnc)
{
    Insn *insn = new Insn;
    insn->code = CL_INSN_CALL;
    insn->loc = *loc;

    TOperandList &operands = insn->operands;
    operands.resize(2);
    storeOperand(operands[0], dst);
    storeOperand(operands[1], fnc);

    // prevent existing reference marks '&' on operands to be taken into account
    // for operands of some internal handlers like VK_ASSERT() or PT_ASSERT().
    d->preventRefOps = isBuiltInFnc(operands[/* fnc */ 1]);

    d->openInsn(insn);
}

void ClStorageBuilder::insn_call_arg(int, const struct cl_operand *arg_src)
{
    TOperandList &operands = d->insn->operands;
    unsigned idx = operands.size();
    operands.resize(idx + 1);
    storeOperand(operands[idx], arg_src);
}

void ClStorageBuilder::insn_call_close()
{
    d->closeInsn();

    // switch back preventing for next instructions
    d->preventRefOps = false;
}

void ClStorageBuilder::insn_switch_open(
    const struct cl_loc     *loc,
    const struct cl_operand *src)
{
    Insn *insn = new Insn;
    insn->code = CL_INSN_SWITCH;
    insn->loc = *loc;

    // store src operand
    TOperandList &operands = insn->operands;
    operands.resize(1);
    storeOperand(operands[0], src);

    // reserve for default
    insn->targets.push_back(static_cast<Block *>(0));

    d->openInsn(insn);
}

void ClStorageBuilder::insn_switch_case(
    const struct cl_loc     *,
    const struct cl_operand *val_lo,
    const struct cl_operand *val_hi,
    const char              *label)
{
    ControlFlow &cfg = d->fnc->cfg;
    Insn &insn = *d->insn;
    TTargetList &targets = insn.targets;

    if (CL_OPERAND_VOID == val_lo->code && CL_OPERAND_VOID == val_hi->code) {
        const Block* &defTarget = targets[0];
        if (defTarget)
            // attempt to redefine default label
            CL_TRAP;

        // store target for default
        targets[0] = cfg[label];
        return;
    }

    if (CL_OPERAND_CST != val_lo->code || CL_OPERAND_CST != val_hi->code)
        CL_TRAP;

    const struct cl_cst &cst_lo = val_lo->data.cst;
    const struct cl_cst &cst_hi = val_hi->data.cst;
    if (CL_TYPE_INT != cst_lo.code || CL_TYPE_INT != cst_hi.code)
        CL_TRAP;

    const int lo = cst_lo.data.cst_int.value;
    const int hi = cst_hi.data.cst_int.value;
    CL_BREAK_IF(hi < lo);

    TOperandList &operands = d->insn->operands;
    struct cl_operand val = *val_lo;

    // FIXME: case ranges has not been tested yet
    for (int i = lo; i <= hi; ++i) {
        val.data.cst.data.cst_int.value = i;

        const unsigned idx = targets.size();
        if (operands.size() != idx)
            // something went wrong, offset detected
            CL_TRAP;

        // store case value
        operands.resize(idx + 1);
        storeOperand(operands[idx], &val);

        // store case target
        targets.resize(idx + 1);
        targets[idx] = cfg[label];
    }
}

void ClStorageBuilder::insn_switch_close()
{
    d->closeInsn();
}
