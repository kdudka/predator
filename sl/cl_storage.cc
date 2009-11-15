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
                opStack.push(idxDst, idxSrc);
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
        opStack.push(&dst, src);
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

    void releaseFnc(Fnc &fnc) {
        releaseOperand(fnc.def);
        BOOST_FOREACH(const Block *bb, fnc.cfg) {
            destroyBlock(const_cast<Block *>(bb));
        }
        // TODO
    }

    void releaseFncMap(FncMap &fncMap) {
        BOOST_FOREACH(const Fnc &fnc, fncMap) {
            releaseFnc(const_cast<Fnc &>(fnc));
        }
    }

    void releaseStorage(Storage &stor) {
        BOOST_FOREACH(const File &file, stor.files) {
            releaseFncMap(const_cast<File &>(file).fncs);
            // TODO
        }
        releaseFncMap(stor.orphans);
    }
}

struct ClStorageBuilder::Private {
    Storage     stor;
    File        *file;
    Fnc         *fnc;
    Block       *bb;

    Private():
        file(0),
        fnc(0),
        bb(0)
    {
    }
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

void ClStorageBuilder::file_open(const char *fileName) {
    FileMap &fmap = d->stor.files;
    d->file = &fmap[fileName];
}

void ClStorageBuilder::file_close() {
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
    FileMap &fmap = d->stor.files;
    File &file = fmap[op->loc.file];
    d->file = &file;

    // store fnc definition
    Fnc &fnc = file.fncs[cst.data.cst_fnc.uid];
    storeOperand(fnc.def, op);
    d->fnc = &fnc;
    d->bb = 0;
}

void ClStorageBuilder::fnc_arg_decl(int, const struct cl_operand *op) {
    if (CL_OPERAND_VAR != op->code)
        TRAP;

    const int uid = op->data.var.id;
    d->fnc->vars[uid] = Var(VAR_FNC_ARG, op);
}

void ClStorageBuilder::fnc_close() {
    d->fnc = 0;
}

void ClStorageBuilder::bb_open(const char *bb_name) {
    ControlFlow &cfg = d->fnc->cfg;
    d->bb = cfg[bb_name];
}

void ClStorageBuilder::insn(const struct cl_insn *cli) {
    if (!d->bb)
        // FIXME: this simply ignores jump to entry instruction
        return;

    // TODO: store var and type
    Insn *insn = createInsn(cli, d->fnc->cfg);
    d->bb->append(insn);
}

void ClStorageBuilder::insn_call_open(
    const struct cl_location*loc,
    const struct cl_operand *dst,
    const struct cl_operand *fnc)
{
    // TODO
}

void ClStorageBuilder::insn_call_arg(int arg_id,
                                     const struct cl_operand *arg_src)
{
    // TODO
}

void ClStorageBuilder::insn_call_close() {
    // TODO
}

void ClStorageBuilder::insn_switch_open(
    const struct cl_location*loc,
    const struct cl_operand *src)
{
    // TODO
}

void ClStorageBuilder::insn_switch_case(
    const struct cl_location*loc,
    const struct cl_operand *val_lo,
    const struct cl_operand *val_hi,
    const char              *label)
{
    // TODO
}

void ClStorageBuilder::insn_switch_close() {
    // TODO
}
