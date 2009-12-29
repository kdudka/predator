/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#include "cl_storage.hh"

#include "storage.hh"

#include <cstring>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace CodeStorage;

namespace {
    /**
     * @param str dst/src to call strdup(3) for
     */
    void dupString(const char *&str) {
        str = strdup(str);
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
            case CL_OPERAND_VAR:
                fnc(op->data.var.name);
                break;

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

    Insn* createInsn(const struct cl_insn *cli, ControlFlow &cfg) {
        enum cl_insn_e code = cli->code;

        Insn *insn = new Insn;
        insn->code = cli->code;
        insn->loc = cli->loc;

        TOperandList &operands = insn->operands;
        TTargetList &targets = insn->targets;

        switch (code) {
            case CL_INSN_NOP:
                TRAP;
                break;

            case CL_INSN_JMP:
                targets.push_back(cfg[cli->data.insn_jmp.label]);
                break;

            case CL_INSN_COND:
                operands.resize(1);
                storeOperand(operands[0], cli->data.insn_cond.src);

                targets.resize(2);
                targets[0] = cfg[cli->data.insn_cond.then_label];
                targets[1] = cfg[cli->data.insn_cond.else_label];
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
            case CL_INSN_SWITCH:
                // wrong constructor used
                TRAP;
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
        delete fnc;
    }

    void releaseFncDb(FncDb &fncMap) {
        BOOST_FOREACH(const Fnc *fnc, fncMap) {
            destroyFnc(const_cast<Fnc *>(fnc));
        }
    }

    void releaseStorage(Storage &stor) {
        BOOST_FOREACH(const File *file, stor.files) {
            releaseFncDb(const_cast<File *>(file)->fncs);
            delete file;
        }
        releaseFncDb(stor.orphans);
    }
}

struct ClStorageBuilder::Private {
    Storage     stor;
    File        *file;
    Fnc         *fnc;
    Block       *bb;
    Insn        *insn;

    Private():
        file(0),
        fnc(0),
        bb(0),
        insn(0)
    {
    }

    void digOperandVar(const struct cl_operand *);
    void digOperandCst(const struct cl_operand *);
    void digOperand(const struct cl_operand *);
    void openInsn(Insn *);
    void closeInsn();
};

ClStorageBuilder::ClStorageBuilder():
    d(new Private)
{
}

ClStorageBuilder::~ClStorageBuilder() {
    releaseStorage(d->stor);
    delete d;
}

void ClStorageBuilder::finalize() {
    this->run(d->stor);
}

void ClStorageBuilder::Private::digOperandVar(const struct cl_operand *op) {
    int id = (CL_OPERAND_REG == op->code)
        ? op->data.reg.id
        : op->data.var.id;
    enum cl_scope_e scope = op->scope;
    switch (scope) {
        case CL_SCOPE_GLOBAL:
            stor.glVars[id] = Var(VAR_GL, op);
            break;

        case CL_SCOPE_STATIC:
            file->vars[id] = Var(VAR_GL, op);
            break;

        case CL_SCOPE_FUNCTION: {
            Var &var = fnc->vars[id];
            EVar code = var.code;
            switch (code) {
                case VAR_VOID:
                    var = Var((CL_OPERAND_REG == op->code)
                                  ? VAR_REG
                                  : VAR_LC,
                              op);
                case VAR_FNC_ARG:
                    break;
                case VAR_LC:
                case VAR_REG:
                    if (id == var.uid)
                        break;
                case VAR_GL:
                    TRAP;
            }
            break;
        }

        case CL_SCOPE_BB:
            TRAP;
    }
}

void ClStorageBuilder::Private::digOperandCst(const struct cl_operand *op) {
    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        // we are interested only in fncs for now
        return;

    // look for File where the fnc is defined/declared
    const char *fileName = op->loc.file;
    File *file = stor.files[fileName];

    // check scope
    enum cl_scope_e scope = op->scope;
    switch (scope) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
            break;
        default:
            // unexpected scope for fnc
            TRAP;
    }

    // select appropriate index by scope
    TFncNames &fncMap = (CL_SCOPE_GLOBAL == scope)
        ? stor.glFncByName
        : file->fncByName;

    // look for precedent fnc name
    const char *fncName = cst.data.cst_fnc.name;
    Fnc* &ref = fncMap[fncName];

    // look for fnc by UID
    const int uid = cst.data.cst_fnc.uid;
    Fnc *fnc = file->fncs[uid];
    if (ref && ref != fnc)
        // fnc redefinition
        TRAP;

    // index fnc name and uid
    ref = fnc;
    stor.anyFncById[uid] = fnc;
}

void ClStorageBuilder::Private::digOperand(const struct cl_operand *op) {
    if (!op || CL_OPERAND_VOID == op->code)
        return;

    // read base type
    TypeDb &typeDb = stor.types;
    readTypeTree(typeDb, op->type);

    // read type of each array index in the chain
    const struct cl_accessor *ac = op->accessor;
    for (; ac; ac = ac->next)
        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            readTypeTree(typeDb, ac->data.array.index->type);

    enum cl_operand_e code = op->code;
    switch (code) {
        case CL_OPERAND_REG:
        case CL_OPERAND_VAR:
            // store variable's metadata
            this->digOperandVar(op);
            break;

        case CL_OPERAND_CST:
            this->digOperandCst(op);
            break;

        case CL_OPERAND_VOID:
        case CL_OPERAND_ARG:
            TRAP;
    }
}

void ClStorageBuilder::Private::openInsn(Insn *newInsn) {
    if (insn)
        // Aiee, insn already opened
        TRAP;

    if (!bb)
        // we have actually no basic block to append the insn to
        TRAP;

    bb->append(newInsn);
    insn = newInsn;
}

void ClStorageBuilder::Private::closeInsn() {
    TOperandList &operands = insn->operands;
    BOOST_FOREACH(const struct cl_operand &op, operands) {
        this->digOperand(&op);
    }

    // let it honestly crash if callback sequence is incorrect since this should
    // be already caught by CldCbSeqChk cl decorator
    insn = 0;
}

void ClStorageBuilder::file_open(const char *fileName) {
    FileDb &fmap = d->stor.files;
    d->file = fmap[fileName];
}

void ClStorageBuilder::file_close() {
    // let it honestly crash if callback sequence is incorrect since this should
    // be already caught by CldCbSeqChk cl decorator
    d->file = 0;
}

void ClStorageBuilder::fnc_open(const struct cl_operand *op) {
    if (CL_OPERAND_CST != op->code)
        TRAP;

    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        TRAP;

    if (!op->loc.file)
        // orphans not implemented yet
        TRAP;

    // store file for fnc
    FileDb &fmap = d->stor.files;
    d->file = fmap[op->loc.file];

    // set current fnc
    int uid = cst.data.cst_fnc.uid;
    Fnc *fnc = d->file->fncs[uid];
    d->fnc = fnc;

    // store fnc definition
    fnc->file = d->file;
    struct cl_operand &def = fnc->def;
    storeOperand(def, op);
    d->digOperand(&def);

    // let it honestly crash if callback sequence is incorrect since this should
    // be already caught by CldCbSeqChk cl decorator
    d->bb = 0;
}

void ClStorageBuilder::fnc_arg_decl(int pos, const struct cl_operand *op) {
    if (CL_OPERAND_VAR != op->code)
        TRAP;

    const int uid = op->data.var.id;
    Fnc &fnc = *(d->fnc);
    Var &var = fnc.vars[uid];
    var = Var(VAR_FNC_ARG, op);

    const int argCnt = fnc.args.size();
    if (argCnt + /* FIXME: start with zero instead? */ 1 != pos)
        // argument list not sorted
        TRAP;

    else
        fnc.args.push_back(uid);
}

void ClStorageBuilder::fnc_close() {
    // let it honestly crash if callback sequence is incorrect since this should
    // be already caught by CldCbSeqChk cl decorator
    d->fnc = 0;
}

void ClStorageBuilder::bb_open(const char *bb_name) {
    ControlFlow &cfg = d->fnc->cfg;
    d->bb = cfg[bb_name];
}

void ClStorageBuilder::insn(const struct cl_insn *cli) {
    if (!d->bb)
        // FIXME: this simply ignores 'jump to entry' insn
        return;

    // serialize given insn
    Insn *insn = createInsn(cli, d->fnc->cfg);
    d->openInsn(insn);

    // current insn is actually already complete
    // in case of one-shot insn
    d->closeInsn();
}

void ClStorageBuilder::insn_call_open(
    const struct cl_location*loc,
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

    d->openInsn(insn);
}

void ClStorageBuilder::insn_call_arg(int, const struct cl_operand *arg_src) {
    TOperandList &operands = d->insn->operands;
    unsigned idx = operands.size();
    operands.resize(idx + 1);
    storeOperand(operands[idx], arg_src);
}

void ClStorageBuilder::insn_call_close() {
    d->closeInsn();
}

void ClStorageBuilder::insn_switch_open(
    const struct cl_location*loc,
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
    const struct cl_location*,
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
            TRAP;

        // store target for default
        targets[0] = cfg[label];
        return;
    }

    if (CL_OPERAND_CST != val_lo->code || CL_OPERAND_CST != val_hi->code)
        TRAP;

    const struct cl_cst &cst_lo = val_lo->data.cst;
    const struct cl_cst &cst_hi = val_hi->data.cst;
    if (CL_TYPE_INT != cst_lo.code || CL_TYPE_INT != cst_hi.code)
        TRAP;

    const int lo = cst_lo.data.cst_int.value;
    const int hi = cst_hi.data.cst_int.value;
    if (lo != hi)
        // case range not supported for now
        TRAP;

    unsigned idx = targets.size();
    TOperandList &operands = d->insn->operands;
    if (operands.size() != idx)
        // something went wrong, offset detected
        TRAP;

    // store case value
    operands.resize(idx + 1);
    storeOperand(operands[idx], val_lo);

    // store case target
    targets.resize(idx + 1);
    targets[idx] = cfg[label];
}

void ClStorageBuilder::insn_switch_close() {
    d->closeInsn();
}
