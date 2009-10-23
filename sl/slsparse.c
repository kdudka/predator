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

#include "code_listener.h"

#define _GNU_SOURCE

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// sparse headers
#include <sparse/expression.h>
#include <sparse/flow.h>
#include <sparse/linearize.h>
#include <sparse/parse.h>
#include <sparse/scope.h>
#include <sparse/storage.h>
#include <sparse/symbol.h>
#include <sparse/token.h>

// safe to remove (it's here for debugging purposes only)
#include <signal.h>
#define TRAP raise(SIGTRAP)

#define DO_EXPAND_SYMBOL            1
#define DO_PER_EP_UNSAA             1
#define DO_PER_EP_SET_UP_STORAGE    1
#define SHOW_PSEUDO_INSNS           0

#define WARN_UNHANDLED(pos, what) do { \
    sl_warn(pos, "warning: '%s' not handled", what); \
    fprintf(stderr, \
            "%s:%d: note: raised from function '%s' [internal location]\n", \
            __FILE__, __LINE__, __FUNCTION__); \
} while (0)

#define WARN_UNHANDLED_SYM(sym) \
    WARN_UNHANDLED((sym)->pos, show_ident((sym)->ident))

#define WARN_VA(pos, fmt, ...) do {\
    sl_warn(pos, "warning: " fmt, __VA_ARGS__); \
    fprintf(stderr, \
            "%s:%d: note: raised from function '%s' [internal location]\n", \
            __FILE__, __LINE__, __FUNCTION__); \
} while (0)

#define WARN_CASE_UNHANDLED(pos, what) \
    case what: WARN_UNHANDLED(pos, #what); break;

static void sl_warn(struct position pos, const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "%s:%d: ", stream_name(pos.stream), pos.line);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");
}

static void read_sparse_location(struct cl_location *loc, struct position pos)
{
    loc->file   = stream_name(pos.stream);
    loc->line   = pos.line;
    loc->column = pos.pos;
    loc->sysp   = /* not used by SPARSE */ false;
}

static bool is_pseudo(pseudo_t pseudo)
{
    return pseudo
        && pseudo != VOID;
}

static void free_cl_operand_data(struct cl_operand *op)
{
    // TODO
#if 0
    free((char *) op->offset);
    switch (op->type) {
        case CL_OPERAND_VAR:
            free((char *) op->data.var.name);
            break;

        case CL_OPERAND_STRING:
            free((char *) op->data.lit_string.value);
            break;

        default:
            break;
    }
#endif
}

static const char* strdup_sparse_string(const struct string *str)
{
    return (str->length)
        ? strndup(str->data, str->length)
        : NULL;
}

static /* const */ struct cl_type builtin_fnc_type = {
    .uid            = CL_UID_INVALID,
    .code           = CL_TYPE_FNC,
    .loc = {
        .file       = NULL,
        .line       = -1
    },
    .scope          = CL_SCOPE_GLOBAL,
    .name           = "<builtin_fnc_type>",
    .size           = /* FIXME */ sizeof(cl_get_type_fnc_t)
};

static /* const */ struct cl_type builtin_string_type = {
    .uid            = CL_UID_INVALID,
    .code           = CL_TYPE_STRING,
    .loc = {
        .file       = NULL,
        .line       = -1
    },
    .scope          = CL_SCOPE_GLOBAL,
    .name           = "<builtin_string_type>",
    .size           = /* FIXME */ sizeof(cl_get_type_fnc_t)
};

static /* const */ struct cl_type builtin_int_type = {
    .uid            = CL_UID_INVALID,
    .code           = CL_TYPE_INT,
    .loc = {
        .file       = NULL,
        .line       = -1
    },
    .scope          = CL_SCOPE_GLOBAL,
    .name           = "<builtin_int>",
    .size           = /* FIXME */ sizeof(int)
};

static void pseudo_to_cl_operand(struct instruction *insn, pseudo_t pseudo,
                                 struct cl_operand *op, bool deref)
{
    // TODO: op->deref       = deref;
    // TODO: op->offset      = NULL;
    op->code        = CL_OPERAND_VOID;
    op->scope       = /* TODO */ CL_SCOPE_GLOBAL;
    op->loc.file    = NULL;
    op->loc.line    = -1;
    op->accessor    = NULL;

    if (!is_pseudo(pseudo))
        return;

    switch(pseudo->type) {
        case PSEUDO_SYM: {
            struct symbol *sym = pseudo->sym;
            struct expression *expr;

            // read symbol location
            read_sparse_location(&op->loc, sym->pos);

            if (sym->bb_target) {
                WARN_UNHANDLED(insn->pos, "sym->bb_target");
                op->type = CL_OPERAND_VOID;
                return;
            }
            if (sym->ident) {
                struct symbol *base = sym->ctype.base_type;
                if (base && base->type == SYM_FN) {
                    op->code                    = CL_OPERAND_CST;
                    op->type                    = &builtin_fnc_type;
                    op->data.cst_fnc.name       = strdup(show_ident(sym->ident));
                    op->data.cst_fnc.is_extern  = MOD_EXTERN & sym->ctype.modifiers;
                } else {
                    op->code                    = CL_OPERAND_VAR;
                    op->type                    = /* TODO */ &builtin_fnc_type;
                    op->data.var.name           = strdup(show_ident(sym->ident));
                }
                break;
            }
            expr = sym->initializer;
            if (expr) {
                switch (expr->type) {
#if 0
                    case EXPR_VALUE:
                        printf("<symbol value: %lld>", expr->value);
                        break;
#endif
                    case EXPR_STRING:
                        op->code                    = CL_OPERAND_CST;
                        op->type                    = &builtin_string_type;
                        op->data.cst_string.value   =
                            strdup_sparse_string(expr->string);
                        return;

                    default:
                        TRAP;
                }
            }
            break;
        }

        case PSEUDO_REG:
            op->code                = CL_OPERAND_REG;
            op->type                = /* TODO */ &builtin_fnc_type;
            // not used: op->name = strdup(show_ident(pseudo->ident));
            op->data.reg.id         = pseudo->nr;
            break;

        case PSEUDO_VAL: {
            long long value = pseudo->value;

            op->code                = CL_OPERAND_CST;
            op->type                = /* TODO: CL_OPERAND_INT */ &builtin_int_type;
            op->data.cst_int.value  = value;
            return;
        }

        case PSEUDO_ARG:
            op->code                = CL_OPERAND_ARG;
            op->type                = /* TODO */ &builtin_fnc_type;
            op->data.arg.id         = pseudo->nr;
            break;

        case PSEUDO_PHI:
            WARN_UNHANDLED(insn->pos, "PSEUDO_PHI");
            break;

        default:
            TRAP;
    }

    if (deref && insn->type) {
        const struct ident *id = insn->type->ident;
        const char *id_string = show_ident(id);
        if (id
                /* FIXME: deref? */
                && 0 != strcmp("__ptr", id_string))
        {
            // TODO: op->offset = strdup(id_string);
        }
    }
}

static bool handle_insn_call(struct instruction *insn,
                             struct cl_code_listener *cl)
{
    struct cl_operand dst, fnc;
    struct pseudo *arg;
    int cnt = 0;

    struct cl_location loc;
    read_sparse_location(&loc, insn->pos);

    // open call
    pseudo_to_cl_operand(insn, insn->target , &dst  , false);
    pseudo_to_cl_operand(insn, insn->func   , &fnc  , false);
    cl->insn_call_open(cl, &loc, &dst, &fnc);
    free_cl_operand_data(&dst);
    free_cl_operand_data(&fnc);

    // go through arguments
    FOR_EACH_PTR(insn->arguments, arg) {
        struct cl_operand src;
        pseudo_to_cl_operand(insn, arg, &src, false);

        cl->insn_call_arg(cl, ++cnt, &src);
        free_cl_operand_data(&src);
    } END_FOR_EACH_PTR(arg);

    // close call
    cl->insn_call_close(cl);
    if (insn->func->sym->ctype.modifiers & MOD_NORETURN) {
        // this call never returns --> end of BB!!

        struct cl_insn cli;
        cli.code    = CL_INSN_ABORT;
        cli.loc     = loc;

        cl->insn(cl, &cli);
        return false;
    }

    return true;
}

static void handle_insn_br(struct instruction *insn,
                           struct cl_code_listener *cl)
{
    char *bb_name_true = NULL;
    char *bb_name_false = NULL;
    struct cl_operand op;

    if (asprintf(&bb_name_true, "%p", insn->bb_true) < 0)
        die("asprintf failed");

    if (!is_pseudo(insn->cond)) {
        struct cl_insn cli;
        cli.code                    = CL_INSN_JMP;
        cli.data.insn_jmp.label     = bb_name_true;
        read_sparse_location(&cli.loc, insn->pos);
        cl->insn(cl, &cli);
        free(bb_name_true);
        return;
    }

    if (asprintf(&bb_name_false, "%p", insn->bb_false) < 0)
        die("asprintf failed");

    pseudo_to_cl_operand(insn, insn->cond, &op, false);

    // TODO: move to function?
    {
        struct cl_insn cli;
        cli.code                        = CL_INSN_COND;
        cli.data.insn_cond.src          = &op;
        cli.data.insn_cond.then_label   = bb_name_true;
        cli.data.insn_cond.else_label   = bb_name_false;
        read_sparse_location(&cli.loc, insn->pos);
        cl->insn(cl, &cli);
    }

    free_cl_operand_data(&op);
    free(bb_name_true);
    free(bb_name_false);
}

static void handle_insn_switch(struct instruction *insn,
                               struct cl_code_listener *cl)
{
    struct cl_operand op;
    struct cl_location loc;
    struct multijmp *jmp;

    // emit insn_switch_open
    pseudo_to_cl_operand(insn, insn->target, &op, false);
    read_sparse_location(&loc, insn->pos);
    cl->insn_switch_open(cl, &loc, &op);
    free_cl_operand_data(&op);

    // go through cases
    FOR_EACH_PTR(insn->multijmp_list, jmp) {
        struct cl_operand val_lo = { CL_OPERAND_VOID };
        struct cl_operand val_hi = { CL_OPERAND_VOID };
        char *label = NULL;

        // if true, it's case; default otherwise
        if (jmp->begin <= jmp->end) {
            val_lo.code = CL_OPERAND_CST;
            val_hi.code = CL_OPERAND_CST;

            // TODO: read types
            val_lo.type = &builtin_int_type;
            val_hi.type = &builtin_int_type;

            val_lo.data.cst_int.value = jmp->begin;
            val_hi.data.cst_int.value = jmp->end;
        }

        if (asprintf(&label, "%p", jmp->target) < 0)
            die("asprintf failed");

        // emit insn_switch_case
        // FIXME: not enough accurate location info from SPARSE for switch/case
        cl->insn_switch_case(cl, &loc, &val_lo, &val_hi, label);

        free_cl_operand_data(&val_lo);
        free_cl_operand_data(&val_hi);
        free(label);

    } END_FOR_EACH_PTR(jmp);

    // emit insn_switch_close
    cl->insn_switch_close(cl);
}

static void handle_insn_ret(struct instruction *insn,
                            struct cl_code_listener *cl)
{
    struct cl_operand op;
    struct cl_insn cli;

    pseudo_to_cl_operand(insn, insn->src, &op, false);
    cli.code                = CL_INSN_RET;
    cli.data.insn_ret.src   = &op;
    read_sparse_location(&cli.loc, insn->pos);
    cl->insn(cl, &cli);
    free_cl_operand_data(&op);
}

static void insn_assignment_base(struct instruction                 *insn,
                                 struct cl_code_listener            *cl,
                                 pseudo_t     lhs,        pseudo_t  rhs,
                                 bool         lhs_deref,  bool      rhs_deref)
{
    struct cl_operand op_lhs;
    struct cl_operand op_rhs;

    pseudo_to_cl_operand(insn, lhs, &op_lhs, lhs_deref);
    pseudo_to_cl_operand(insn, rhs, &op_rhs, rhs_deref);

#if 0
    if (op_lhs.deref && op_lhs.name && op_lhs.offset
            && 0 == strcmp(op_lhs.name, op_lhs.offset))
        TRAP;

    if (op_rhs.deref && op_rhs.name && op_rhs.offset
            && 0 == strcmp(op_rhs.name, op_rhs.offset))
        TRAP;
#endif

    // TODO: move to function?
    {
        struct cl_insn cli;
        cli.code                    = CL_INSN_UNOP;
        cli.data.insn_unop.code     = CL_UNOP_ASSIGN;
        cli.data.insn_unop.dst      = &op_lhs;
        cli.data.insn_unop.src      = &op_rhs;
        read_sparse_location(&cli.loc, insn->pos);
        cl->insn(cl, &cli);
    }

    free_cl_operand_data(&op_lhs);
    free_cl_operand_data(&op_rhs);
}

static void handle_insn_store(struct instruction *insn,
                              struct cl_code_listener *cl)

{
    insn_assignment_base(insn, cl,
            insn->symbol, insn->target,
            true        , false);
}
static void handle_insn_load(struct instruction *insn,
                             struct cl_code_listener *cl)
{
    insn_assignment_base(insn, cl,
            insn->target, insn->symbol,
            false       , true);
}
static void handle_insn_copy(struct instruction *insn,
                             struct cl_code_listener *cl)
{
    insn_assignment_base(insn, cl,
            insn->target, insn->src,
            false       , false);
}

static void handle_insn_binop(struct instruction *insn, enum cl_operand_e type,
                            struct cl_code_listener *cl)
{
    struct cl_operand dst, src1, src2;

    pseudo_to_cl_operand(insn, insn->target , &dst  , false);
    pseudo_to_cl_operand(insn, insn->src1   , &src1 , false);
    pseudo_to_cl_operand(insn, insn->src2   , &src2 , false);

    // TODO: move to function?
    {
        struct cl_insn cli;
        cli.code = CL_INSN_BINOP;
        cli.data.insn_binop.code    = type;
        cli.data.insn_binop.dst     = &dst;
        cli.data.insn_binop.src1    = &src1;
        cli.data.insn_binop.src2    = &src2;
        read_sparse_location(&cli.loc, insn->pos);
        cl->insn(cl, &cli);
    }

    free_cl_operand_data(&dst);
    free_cl_operand_data(&src1);
    free_cl_operand_data(&src2);
}

static bool handle_insn(struct instruction *insn, struct cl_code_listener *cl)
{
    switch (insn->opcode) {
        WARN_CASE_UNHANDLED(insn->pos, OP_BADOP)

        /* Entry */
        case OP_ENTRY:
            // ignore for now
            break;

        /* Terminator */
        case OP_RET /*= OP_TERMINATOR*/:
            handle_insn_ret(insn, cl);
            break;

        case OP_BR:
            handle_insn_br(insn, cl);
            break;

        case OP_SWITCH:
            handle_insn_switch(insn, cl);
            break;

        WARN_CASE_UNHANDLED(insn->pos, OP_INVOKE)
        WARN_CASE_UNHANDLED(insn->pos, OP_COMPUTEDGOTO)
        WARN_CASE_UNHANDLED(insn->pos, OP_TERMINATOR_END /*= OP_UNWIND*/)

        /* Binary */
        case OP_ADD /*= OP_BINARY*/:
            handle_insn_binop(insn, CL_BINOP_PLUS, cl);
            break;

        WARN_CASE_UNHANDLED(insn->pos, OP_SUB)
        WARN_CASE_UNHANDLED(insn->pos, OP_MULU)
        WARN_CASE_UNHANDLED(insn->pos, OP_MULS)
        WARN_CASE_UNHANDLED(insn->pos, OP_DIVU)
        WARN_CASE_UNHANDLED(insn->pos, OP_DIVS)
        WARN_CASE_UNHANDLED(insn->pos, OP_MODU)
        WARN_CASE_UNHANDLED(insn->pos, OP_MODS)
        WARN_CASE_UNHANDLED(insn->pos, OP_SHL)
        WARN_CASE_UNHANDLED(insn->pos, OP_LSR)
        WARN_CASE_UNHANDLED(insn->pos, OP_ASR)

        /* Logical */
        WARN_CASE_UNHANDLED(insn->pos, OP_AND)
        WARN_CASE_UNHANDLED(insn->pos, OP_OR)
        WARN_CASE_UNHANDLED(insn->pos, OP_XOR)
        WARN_CASE_UNHANDLED(insn->pos, OP_AND_BOOL)
        WARN_CASE_UNHANDLED(insn->pos, OP_BINARY_END /*= OP_OR_BOOL*/)

        /* Binary comparison */
        case OP_SET_EQ /*= OP_BINCMP*/:
            handle_insn_binop(insn, CL_BINOP_EQ, cl);
            break;

        case OP_SET_NE:
            handle_insn_binop(insn, CL_BINOP_NE, cl);
            break;

        case OP_SET_LE:
            handle_insn_binop(insn, CL_BINOP_LE, cl);
            break;

        case OP_SET_GE:
            handle_insn_binop(insn, CL_BINOP_GE, cl);
            break;

        case OP_SET_LT:
            handle_insn_binop(insn, CL_BINOP_LT, cl);
            break;

        case OP_SET_GT:
            handle_insn_binop(insn, CL_BINOP_GT, cl);
            break;

        WARN_CASE_UNHANDLED(insn->pos, OP_SET_B)
        WARN_CASE_UNHANDLED(insn->pos, OP_SET_A)
        WARN_CASE_UNHANDLED(insn->pos, OP_SET_BE)
        WARN_CASE_UNHANDLED(insn->pos, OP_BINCMP_END /*= OP_SET_AE*/)

        /* Uni */
        WARN_CASE_UNHANDLED(insn->pos, OP_NOT)
        WARN_CASE_UNHANDLED(insn->pos, OP_NEG)

        /* Select - three input values */
        WARN_CASE_UNHANDLED(insn->pos, OP_SEL)

        /* Memory */
        WARN_CASE_UNHANDLED(insn->pos, OP_MALLOC)
        WARN_CASE_UNHANDLED(insn->pos, OP_FREE)
        WARN_CASE_UNHANDLED(insn->pos, OP_ALLOCA)
        case OP_LOAD:
            handle_insn_load(insn, cl);
            break;

        case OP_STORE:
            handle_insn_store(insn, cl);
            break;

        WARN_CASE_UNHANDLED(insn->pos, OP_SETVAL)
        WARN_CASE_UNHANDLED(insn->pos, OP_SYMADDR)
        WARN_CASE_UNHANDLED(insn->pos, OP_GET_ELEMENT_PTR)

        /* Other */
        case OP_PHI:
        case OP_PHISOURCE:
            // FIXME: this might be a SPARSE bug if DO_PER_EP_UNSAA is set
            WARN_UNHANDLED(insn->pos, show_instruction(insn));
            break;

        case OP_CAST:
        case OP_SCAST:
        case OP_FPCAST:
        case OP_PTRCAST:
            // TODO: separate handler?
            handle_insn_copy(insn, cl);
            break;

        WARN_CASE_UNHANDLED(insn->pos, OP_INLINED_CALL)
        case OP_CALL:
            return handle_insn_call(insn, cl);

        WARN_CASE_UNHANDLED(insn->pos, OP_VANEXT)
        WARN_CASE_UNHANDLED(insn->pos, OP_VAARG)
        WARN_CASE_UNHANDLED(insn->pos, OP_SLICE)
        case OP_SNOP:
            //handle_insn_store(insn);
            WARN_UNHANDLED(insn->pos, show_instruction(insn));
            break;

        case OP_LNOP:
            //handle_insn_load(insn);
            WARN_UNHANDLED(insn->pos, show_instruction(insn));
            break;

        WARN_CASE_UNHANDLED(insn->pos, OP_NOP)
        WARN_CASE_UNHANDLED(insn->pos, OP_DEATHNOTE)
        WARN_CASE_UNHANDLED(insn->pos, OP_ASM)

        /* Sparse tagging (line numbers, context, whatever) */
        WARN_CASE_UNHANDLED(insn->pos, OP_CONTEXT)
        WARN_CASE_UNHANDLED(insn->pos, OP_RANGE)

        /* Needed to translate SSA back to normal form */
        case OP_COPY:
            handle_insn_copy(insn, cl);
            break;
    }
    return true;
}

static bool is_insn_interesting(struct instruction *insn)
{
#if 0
    unsigned size = insn->size;
    if (size && KNOWN_PTR_SIZE != size) {
        WARN_VA(insn->pos, "ignored instruction with operand of size %d",
                insn->size);
        return false;
    }
#endif

    switch (insn->opcode) {
        case OP_ENTRY:
            return false;

        default:
            return true;
    }
}

static bool handle_bb_insn(struct instruction *insn,
                           struct cl_code_listener *cl)
{
    if (!insn)
        return true;

    if (!insn->bb) {
#if SHOW_PSEUDO_INSNS
        WARN_VA(insn->pos, "ignoring pseudo: %s", show_instruction(insn));
#endif
        return true;
    }

    if (!is_insn_interesting(insn))
        return true;

    return handle_insn(insn, cl);
}

static void handle_bb(struct basic_block *bb, struct cl_code_listener *cl)
{
    struct instruction *insn;
    char *bb_name;

    if (!bb)
        return;

    if (asprintf(&bb_name, "%p", bb) < 0)
        die("asprintf failed");

    cl->bb_open(cl, bb_name);
    free(bb_name);

    FOR_EACH_PTR(bb->insns, insn) {
        if (!handle_bb_insn(insn, cl))
            // subtle: 'break' stmt here does not work as one would expected to
            goto done;
    } END_FOR_EACH_PTR(insn);
done:
    return;
}

static void handle_fnc_ep(struct entrypoint *ep, struct cl_code_listener *cl)
{
    struct instruction *entry = ep->entry;
    struct basic_block *bb;
    char *entry_name;

    if (!bb)
        return;

    // jump to entry basic block
    if (asprintf(&entry_name, "%p", entry->bb) < 0)
        die("asprintf failed");

    // TODO: move to function?
    {
        struct cl_insn cli;
        cli.code                    = CL_INSN_JMP;
        cli.data.insn_jmp.label     = entry_name;
        read_sparse_location(&cli.loc, entry->pos);
        cl->insn(cl, &cli);
    }
    free(entry_name);

    // go through basic blocks
    FOR_EACH_PTR(ep->bbs, bb) {
        if (!bb)
            continue;

        if (bb->parents || bb->children || bb->insns
                || /* FIXME: is the following actually useful? */ 2 < verbose)
        {
            handle_bb(bb, cl);
        }
    } END_FOR_EACH_PTR(bb);
}

static void handle_fnc_body(struct symbol *sym, struct cl_code_listener *cl)
{
    struct entrypoint *ep = linearize_symbol(sym);
    if (!ep)
        TRAP;

#if DO_PER_EP_UNSAA
    unssa(ep);
#endif

#if DO_PER_EP_SET_UP_STORAGE
    set_up_storage(ep);
#endif

    handle_fnc_ep(ep, cl);

#if DO_PER_EP_SET_UP_STORAGE
    free_storage();
#endif
}

static void handle_fnc_def(struct symbol *sym, struct cl_code_listener *cl)
{
    struct symbol *base_type = sym->ctype.base_type;
    struct symbol *arg;
    struct cl_location loc;
    int argc = 0;

    read_sparse_location(&loc, sym->pos);
    cl->fnc_open(cl, &loc, show_ident(sym->ident),
            (sym->scope==file_scope)
            ? CL_SCOPE_STATIC
            : CL_SCOPE_GLOBAL);

    // dump argument list
    FOR_EACH_PTR(base_type->arguments, arg) {
        // TODO: cl->fnc_arg_decl(cl, ++argc, show_ident(arg->ident));
        struct cl_operand op;
        op.loc.file                 = NULL;
        op.loc.line                 = -1;
        op.code                     = CL_OPERAND_VAR;
        op.scope                    = /* TODO */ CL_SCOPE_GLOBAL;
        op.type                     = /* TODO */ &builtin_fnc_type;
        op.accessor                 = NULL;
        op.data.var.name            = strdup(show_ident(arg->ident));
        cl->fnc_arg_decl(cl, ++argc, &op);
    } END_FOR_EACH_PTR(arg);

    // handle fnc body
    handle_fnc_body(sym, cl);
    cl->fnc_close(cl);
}

static void handle_sym_fn(struct symbol *sym, struct cl_code_listener *cl)
{
    struct symbol *base_type = sym->ctype.base_type;
    struct statement *stmt = base_type->stmt;

    if (stmt) {
        // function definition
        handle_fnc_def(sym, cl);
        return;
    }

    WARN_UNHANDLED_SYM(sym);
}

static void handle_top_level_sym(struct symbol *sym,
                                 struct cl_code_listener *cl)
{
    struct symbol *base_type;

    if (!sym)
        return;

    base_type = sym->ctype.base_type;
    if (!base_type)
        return;

    switch (base_type->type) {
        WARN_CASE_UNHANDLED(sym->pos, SYM_UNINITIALIZED)
        WARN_CASE_UNHANDLED(sym->pos, SYM_PREPROCESSOR)
        WARN_CASE_UNHANDLED(sym->pos, SYM_BASETYPE)
        WARN_CASE_UNHANDLED(sym->pos, SYM_NODE)
        WARN_CASE_UNHANDLED(sym->pos, SYM_PTR)
        WARN_CASE_UNHANDLED(sym->pos, SYM_ARRAY)
        WARN_CASE_UNHANDLED(sym->pos, SYM_STRUCT)
        WARN_CASE_UNHANDLED(sym->pos, SYM_UNION)
        WARN_CASE_UNHANDLED(sym->pos, SYM_ENUM)
        WARN_CASE_UNHANDLED(sym->pos, SYM_TYPEDEF)
        WARN_CASE_UNHANDLED(sym->pos, SYM_TYPEOF)
        WARN_CASE_UNHANDLED(sym->pos, SYM_MEMBER)
        WARN_CASE_UNHANDLED(sym->pos, SYM_BITFIELD)
        WARN_CASE_UNHANDLED(sym->pos, SYM_LABEL)
        WARN_CASE_UNHANDLED(sym->pos, SYM_RESTRICT)
        WARN_CASE_UNHANDLED(sym->pos, SYM_FOULED)
        WARN_CASE_UNHANDLED(sym->pos, SYM_KEYWORD)
        WARN_CASE_UNHANDLED(sym->pos, SYM_BAD)

        case SYM_FN:
            handle_sym_fn(sym, cl);
            break;
    }

    if (sym->initializer)
        WARN_UNHANDLED(sym->pos, "sym->initializer");
}

static void clean_up_symbols(struct symbol_list *list,
                             struct cl_code_listener *cl)
{
    struct symbol *sym;

    FOR_EACH_PTR(list, sym) {
#if DO_EXPAND_SYMBOL
        expand_symbol(sym);
#endif
        handle_top_level_sym(sym, cl);
    } END_FOR_EACH_PTR(sym);
}

static struct cl_code_listener* create_cl_chain()
{
    struct cl_code_listener *cl;
    struct cl_code_listener *chain = cl_chain_create();
    if (!chain)
        // error message already emitted
        return NULL;

    if (1 < verbose) {
        cl = cl_code_listener_create("listener=\"locator\"");
        if (!cl) {
            chain->destroy(chain);
            return NULL;
        }
        cl_chain_append(chain, cl);
    }

    cl = cl_code_listener_create("listener=\"pp\" "
            "cld=\"arg_subst,unify_labels_fnc,unify_regs,unify_vars\"");
    if (!cl) {
        chain->destroy(chain);
        return NULL;
    }
    cl_chain_append(chain, cl);

    cl = cl_code_listener_create("listener=\"dotgen\" "
            "cld=\"arg_subst,unify_labels_fnc,unify_regs,unify_vars\"");
    if (!cl) {
        chain->destroy(chain);
        return NULL;
    }
    cl_chain_append(chain, cl);

    return chain;
}

int main(int argc, char **argv)
{
    char *file;
    struct string_list *filelist = NULL;
    struct cl_code_listener *cl;
    struct symbol_list *symlist;

#if 1
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    symlist = sparse_initialize(argc, argv, &filelist);

    cl_global_init_defaults(NULL, verbose);
    cl = create_cl_chain();
    if (!cl)
        // error message already emitted
        return EXIT_FAILURE;

    cl->file_open(cl, "sparse-internal-symbols");
    clean_up_symbols(symlist, cl);
    cl->file_close(cl);

    FOR_EACH_PTR_NOTAG(filelist, file) {
        if (0 < verbose)
            fprintf(stderr, "%s: about to process '%s'...\n", argv[0], file);

        cl->file_open(cl, file);
        clean_up_symbols(sparse(file), cl);
        cl->file_close(cl);
    } END_FOR_EACH_PTR_NOTAG(file);

    cl->destroy(cl);
    cl_global_cleanup();

    return 0;
}
