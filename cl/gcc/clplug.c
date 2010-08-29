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

// this include has to be the first (according the gcc plug-in API)
#include <gcc/gcc-plugin.h>

#include <cl/code_listener.h>

#include "../config.h"
#include "../version.h"

#include <gcc/coretypes.h>
#include <gcc/diagnostic.h>
#include <gcc/ggc.h>
#include <gcc/hashtab.h>

// this include has to be before <gcc/function.h>; otherwise it will NOT compile
#include <gcc/tm.h>

#include <gcc/function.h>
#include <gcc/gimple.h>
#include <gcc/input.h>
#include <gcc/tree-pass.h>

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <stdarg.h>

#ifndef DEBUG_SLPLUG
#   define DEBUG_SLPLUG 0
#endif

#ifndef STREQ
#   define STREQ(s1, s2) (0 == strcmp(s1, s2))
#endif

// somewhere after 4.5.0, the GGC_CNEW somehow vanished
#ifndef GGC_CNEW
#   define GGC_CNEW(type) ggc_internal_cleared_alloc_stat(sizeof (type))
#endif

// somewhere after 4.5.0, the declaration has been moved to
// gimple-pretty-print.h, which is no longer available for public
extern void print_gimple_stmt (FILE *, gimple, int, int);

// this should be using gcc's fancy_abort(), but it was actually not tested
#define SL_ASSERT(expr) \
    if (!(expr)) abort()

#if DEBUG_SLPLUG
#   define SL_LOG(...) do { \
        if (SL_VERBOSE_PLUG & verbose) { \
            fprintf (stderr, "%s:%d: %s: ", __FILE__, __LINE__, plugin_name); \
            fprintf (stderr, __VA_ARGS__); \
            fprintf (stderr, " [internal location]\n"); \
        } \
    } while (0)
#else
#   define SL_LOG(...) (void) 0
#endif

// TODO: replace with gcc native debugging infrastructure
#define SL_LOG_FNC \
    SL_LOG ("%s", __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED(what) \
    fprintf(stderr, \
            "%s:%d: warning: '%s' not handled in '%s' [internal location]\n", \
            __FILE__, __LINE__, (what), __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED_WITH_LOC(loc, what) \
    fprintf(stderr, "%s:%d:%d: warning: '%s' not handled\n" \
            "%s:%d: note: raised from '%s' [internal location]\n", \
            expand_location(loc).file, \
            expand_location(loc).line, \
            expand_location(loc).column, \
            (what), \
            __FILE__, __LINE__, __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED_GIMPLE(stmt, what) \
    SL_WARN_UNHANDLED_WITH_LOC((stmt)->gsbase.location, what)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED_EXPR(expr, what) do { \
    SL_WARN_UNHANDLED_WITH_LOC(EXPR_LOCATION(expr), what); \
    if (SL_VERBOSE_UNHANDLED_EXPR & verbose) \
        debug_tree(expr); \
} while (0)

// name of the plug-in given by gcc during initialization
static const char *plugin_name = "[uninitialized]";

// verbose bitmask
static int verbose = 0;
#define SL_VERBOSE_PLUG             (1 << 0)
#define SL_VERBOSE_LOCATION         (1 << 1)
#define SL_VERBOSE_GIMPLE           (1 << 2)
#define SL_VERBOSE_UNHANDLED_EXPR   (1 << 3)

// plug-in meta-data according to gcc plug-in API
// TODO: split also version of code_listener and version of peer
static struct plugin_info sl_info = {
    .version = "XXX [SHA1 " CL_GIT_SHA1 "]",
    .help =    "XXX [SHA1 " CL_GIT_SHA1 "]\n"
"\n"
"Usage: gcc -fplugin=XXX.so [OPTIONS] ...\n"
"\n"
"// TODO: replace \"symexec\" by something more generic??\n"
"OPTIONS:\n"
"    -fplugin-arg-XXX-help\n"
"    -fplugin-arg-XXX-version\n"
"    -fplugin-arg-XXX-bypass-symexec                 do not run symexec\n"
"    -fplugin-arg-XXX-dump-pp[=OUTPUT_FILE]          dump linearized code\n"
"    -fplugin-arg-XXX-dump-types                     dump also type info\n"
"    -fplugin-arg-XXX-gen-dot[=GLOBAL_CG_FILE]       generate CFGs\n"
"    -fplugin-arg-XXX-symexec-args=SYMEXEC_ARGS      args given to symexec\n"
"    -fplugin-arg-XXX-type-dot=TYPE_GRAPH_FILE       generate type graphs\n"
"    -fplugin-arg-XXX-verbose[=VERBOSE_BITMASK]      turn on verbose mode\n"
"\n"
"//TODO: replace XXX by name of the shared library in run-time\n"
"\n"
"VERBOSE_BITMASK:\n"
"    1          debug gcc plug-in API and code_listener\n"
"    2          print location info using \"locator\" code listener\n"
"    4          print each gimple statement before its processing\n"
"    8          dump gcc tree of unhandled expressions\n"
"\n"
"SYMEXEC_ARGS:\n"
"    fast       disable OOM simulation within symbolic execution of malloc()\n"
};

typedef htab_t type_db_t;

static struct cl_code_listener *cl = NULL;
static type_db_t type_db = NULL;

static hashval_t type_db_hash (const void *p)
{
    const struct cl_type *type = (const struct cl_type *) p;
    return type->uid;
}

static int type_db_eq (const void *p1, const void *p2)
{
    const struct cl_type *type1 = (const struct cl_type *) p1;
    const struct cl_type *type2 = (const struct cl_type *) p2;
    return type1->uid == type2->uid;
}

static void type_db_free (void *p)
{
    // TODO
    (void) p;
}

static type_db_t type_db_create(void)
{
    type_db_t db = htab_create_alloc(/* FIXME: hardcoded for now */ 0x100,
                                     type_db_hash, type_db_eq, type_db_free,
                                     xcalloc, free);
    SL_ASSERT(db);
    return db;
}

static void type_db_destroy(type_db_t db)
{
    htab_delete(db);
}

static struct cl_type* type_db_lookup(type_db_t db, cl_type_uid_t uid)
{
    struct cl_type type;
    type.uid = uid;

    return htab_find(db, &type);
}

static void type_db_insert(type_db_t db, struct cl_type *type)
{
    void **slot = htab_find_slot(db, type, INSERT);
    SL_ASSERT(slot);
    *slot = type;
}

static void read_gcc_location(struct cl_location *loc, location_t gcc_loc)
{
    expanded_location exp_loc = expand_location(gcc_loc);
    loc->file   = exp_loc.file;
    loc->line   = exp_loc.line;
    loc->column = /* FIXME: is this field always valid? */ exp_loc.column;
    loc->sysp   = /* FIXME: is this field always valid? */ exp_loc.sysp;
}

static void read_gimple_location(struct cl_location *loc, const_gimple g)
{
    read_gcc_location(loc, g->gsbase.location);
}

static char* index_to_label (unsigned idx) {
    char *label;
    int rv = asprintf(&label, "%u:%u", DECL_UID(current_function_decl), idx);
    SL_ASSERT(0 < rv);
    return label;
}

static int get_type_sizeof(tree t)
{
    tree size = TYPE_SIZE_UNIT(t);
    if (NULL_TREE == size)
        return 0;

    if (TREE_INT_CST_HIGH(size))
        TRAP;

    return TREE_INT_CST_LOW(size);
}

static void read_base_type(struct cl_type *clt, tree type)
{
    // store sizeof
    clt->size = get_type_sizeof(type);

    tree name = TYPE_NAME(type);
    if (NULL_TREE == name)
        return;

    // TODO: scope
    if (IDENTIFIER_NODE == TREE_CODE(name)) {
        clt->name = IDENTIFIER_POINTER(name);
        // read_gcc_location(&clt->loc, DECL_SOURCE_LOCATION(type));
    } else {
        read_gcc_location(&clt->loc, DECL_SOURCE_LOCATION(name));
        name = DECL_NAME(name);
        if (name)
            clt->name = IDENTIFIER_POINTER(name);
    }
}

static int get_fixed_array_size(tree t)
{
    if (NULL_TREE == t)
        return 0;

    tree type = TREE_TYPE(t);
    if (NULL_TREE == type)
        return 0;

    int item_size = get_type_sizeof(type);
    if (!item_size)
        // avoid division by zero
        return 0;

    // FIXME: this simply ignores any alignment
    return get_type_sizeof(t) / item_size;
}

static struct cl_type* add_bare_type_if_needed(tree t);
static struct cl_type* add_type_if_needed(tree t)
{
    if (NULL_TREE == t)
        TRAP;

    tree type = TREE_TYPE(t);
    if (NULL_TREE == type)
        TRAP;

    return add_bare_type_if_needed(type);
}

static int dig_field_offset(tree t)
{
    // read bit offset
    tree node = DECL_FIELD_BIT_OFFSET(t);
    if (!node || INTEGER_CST != TREE_CODE(node) || TREE_INT_CST_HIGH(node))
        TRAP;
    int offset = TREE_INT_CST_LOW(node) >> 3;

    // read byte offset
    node = DECL_FIELD_OFFSET(t);
    if (!node || INTEGER_CST != TREE_CODE(node) || TREE_INT_CST_HIGH(node))
        TRAP;
    offset += TREE_INT_CST_LOW(node);

    // return total offset [in bytes]
    return offset;
}

static void dig_record_type(struct cl_type *clt, tree t)
{
    for (t = TYPE_FIELDS(t); t; t = TREE_CHAIN(t)) {
        // TODO: chunk allocation ?
        clt->items = GGC_RESIZEVEC(struct cl_type_item, clt->items,
                                   clt->item_cnt + 1);

        struct cl_type_item *item = &clt->items[clt->item_cnt ++];
        item->type = /* recursion */ add_type_if_needed(t);
        item->name = /* possibly anonymous member */ NULL;
        item->offset = dig_field_offset(t);

        // read item's name (if any)
        tree name = DECL_NAME(t);
        if (name)
            item->name = IDENTIFIER_POINTER(name);
    }
}

static void dig_fnc_type(struct cl_type *clt, tree t)
{
    // dig return type
    clt->item_cnt = 1;
    clt->items = GGC_RESIZEVEC(struct cl_type_item, 0, 1);
    struct cl_type_item *item = clt->items;
    item->type = add_type_if_needed(t);
    item->name = NULL;

    // dig arg types
    for (t = TYPE_ARG_TYPES(t); t; t = TREE_CHAIN(t)) {
        tree val = TREE_VALUE(t);
        if (!val)
            TRAP;

        // TODO: chunk allocation ?
        clt->items = GGC_RESIZEVEC(struct cl_type_item, clt->items,
                                   clt->item_cnt + 1);

        struct cl_type_item *item = &clt->items[clt->item_cnt ++];
        item->type = /* recursion */ add_bare_type_if_needed(val);
        item->name = NULL;
    }
}

static void read_specific_type(struct cl_type *clt, tree type)
{
    enum tree_code code = TREE_CODE(type);
    switch (code) {
        case VOID_TYPE:
            clt->code = CL_TYPE_VOID;
            break;

        case POINTER_TYPE:
            clt->code = CL_TYPE_PTR;
            clt->item_cnt = 1;
            clt->items = GGC_CNEW(struct cl_type_item);
            clt->items[0].type = /* recursion */ add_type_if_needed(type);
            break;

        case RECORD_TYPE:
            clt->code = CL_TYPE_STRUCT;
            dig_record_type(clt, type);
            break;

        case UNION_TYPE:
            clt->code = CL_TYPE_UNION;
            dig_record_type(clt, type);
            break;

        case ARRAY_TYPE:
            clt->code = CL_TYPE_ARRAY;
            clt->item_cnt = get_fixed_array_size(type);
            clt->items = GGC_CNEW(struct cl_type_item);
            clt->items[0].type = /* recursion */ add_type_if_needed(type);
            break;

        case FUNCTION_TYPE:
            clt->code = CL_TYPE_FNC;
            clt->size = 0;
            dig_fnc_type(clt, type);
            break;

        case INTEGER_TYPE:
            clt->code = CL_TYPE_INT;
            break;

        case BOOLEAN_TYPE:
            clt->code = CL_TYPE_BOOL;
            break;

        case ENUMERAL_TYPE:
            clt->code = CL_TYPE_ENUM;
            break;

        default:
            TRAP;
    };
}

static struct cl_type* add_bare_type_if_needed(tree type)
{
    // hashtab lookup
    cl_type_uid_t uid = TYPE_UID(type);
    struct cl_type *clt = type_db_lookup(type_db, uid);
    if (clt)
        // type already hashed
        return clt;

    // insert new type into hashtab
    clt = GGC_CNEW(struct cl_type);
    clt->uid = uid;
    type_db_insert(type_db, clt);

    // read type (recursively if needed)
    read_base_type(clt, type);
    read_specific_type(clt, type);

    // extra hooks follow
    if (clt->code == CL_TYPE_INT && clt->name && STREQ("char", clt->name))
        clt->code = CL_TYPE_CHAR;

    return clt;
}

static enum cl_scope_e get_decl_scope(tree t)
{
    tree ctx = DECL_CONTEXT(t);
    if (ctx) {
        enum tree_code code = TREE_CODE(ctx);
        switch (code) {
            case FUNCTION_DECL:
                return CL_SCOPE_FUNCTION;

            case TRANSLATION_UNIT_DECL:
            default:
                TRAP;
        }
    }

    return (TREE_PUBLIC(t))
        ? CL_SCOPE_GLOBAL
        : CL_SCOPE_STATIC;
}

static void read_operand_decl(struct cl_operand *op, tree t)
{
    enum tree_code code = TREE_CODE(t);
    read_gcc_location(&op->loc, DECL_SOURCE_LOCATION(t));

    if (DECL_ARTIFICIAL(t)) {
        // emit as a register
        op->code            = CL_OPERAND_REG;
        op->data.reg.id     = DECL_UID(t);
        op->scope           = /* make it possible to use unify_regs decorator */
                              CL_SCOPE_FUNCTION;
        return;
    }

    if (!DECL_NAME(t))
        // anonymous operand ... something went wrong
        TRAP;

    // read operand's name and scope
    const char *name = IDENTIFIER_POINTER(DECL_NAME(t));
    op->scope = get_decl_scope(t);

    switch (code) {
        case FUNCTION_DECL:
            op->code                            = CL_OPERAND_CST;
            op->data.cst.code                   = CL_TYPE_FNC;
            op->data.cst.data.cst_fnc.name      = name;
            op->data.cst.data.cst_fnc.is_extern = DECL_EXTERNAL(t);
            op->data.cst.data.cst_fnc.uid       = DECL_UID(t);
            break;

        case PARM_DECL:
        case VAR_DECL:
            op->code                            = CL_OPERAND_VAR;
            op->data.var.id                     = DECL_UID(t);
            op->data.var.name                   = name;
            break;

        default:
            TRAP;
    }
}

static void read_raw_operand(struct cl_operand *op, tree t)
{
    enum tree_code code = TREE_CODE(t);
    switch (code) {
        case VAR_DECL:
        case PARM_DECL:
        case FUNCTION_DECL:
        case RESULT_DECL:
            read_operand_decl(op, t);
            break;

        case INTEGER_CST:
            op->code                            = CL_OPERAND_CST;
            op->data.cst.code                   = CL_TYPE_INT;

            // I don't understand the following code, see gcc/print-tree.c
            if (TREE_INT_CST_HIGH (t) == 0 || (TREE_INT_CST_LOW (t) != 0
                        && TREE_INT_CST_HIGH (t) == -1))
                op->data.cst.data.cst_int.value = TREE_INT_CST_LOW (t);
            else
                // FIXME: this would probably overflow...
                TRAP;

            break;

        case STRING_CST:
            op->code                            = CL_OPERAND_CST;
            op->data.cst.code                   = CL_TYPE_STRING;
            op->data.cst.data.cst_string.value  = TREE_STRING_POINTER(t);
            break;

        case REAL_CST:
            SL_WARN_UNHANDLED_EXPR(t, "REAL_CST");
            break;

        case CONSTRUCTOR:
            SL_WARN_UNHANDLED_EXPR(t, "CONSTRUCTOR");
            break;

        default:
            TRAP;
    }
}

// FIXME: check direction
static void chain_accessor(struct cl_accessor **ac, enum cl_type_e code)
{
    // create and initialize new item
    struct cl_accessor *ac_new = GGC_CNEW(struct cl_accessor);
    SL_ASSERT(ac_new);
    ac_new->code = code;

    // append new item to chain
    ac_new->next = *ac;
    *ac = ac_new;
}

static int accessor_item_lookup(const struct cl_type *type, tree t)
{
    tree name = DECL_NAME(t);
    if (NULL_TREE == name)
        // decl omitted?
        TRAP;

    const char *ident = IDENTIFIER_POINTER(name);
    const struct cl_type_item *items = type->items;

    int i;
    for (i = 0; i < type->item_cnt; ++i)
        if (STREQ(ident, items[i].name))
            return i;

    // not found
    TRAP;
    return -1;
}

static struct cl_type* operand_type_lookup(tree t)
{
    if (NULL_TREE == t)
        TRAP;

    tree op0 = TREE_OPERAND(t, 0);
    if (NULL_TREE == op0)
        TRAP;

    return add_type_if_needed(op0);
}

static void handle_operand(struct cl_operand *op, tree t);

static void handle_accessor_addr_expr(struct cl_accessor **ac, tree t)
{
    if (STRING_CST == TREE_CODE(TREE_OPERAND(t, 0)))
        return;

    chain_accessor(ac, CL_ACCESSOR_REF);
    (*ac)->type = operand_type_lookup(t);
}

static void handle_accessor_array_ref(struct cl_accessor **ac, tree t)
{
    if (STRING_CST == TREE_CODE(TREE_OPERAND(t, 0)))
        return;

    chain_accessor(ac, CL_ACCESSOR_DEREF_ARRAY);
    (*ac)->type = operand_type_lookup(t);

    tree op1 = TREE_OPERAND(t, 1);
    if (!op1)
        TRAP;

    struct cl_operand *index = GGC_CNEW(struct cl_operand);
    SL_ASSERT(index);

    // FIXME: unguarded recursion
    handle_operand(index, op1);
    (*ac)->data.array.index = index;
}

static void handle_accessor_indirect_ref(struct cl_accessor **ac, tree t)
{
    chain_accessor(ac, CL_ACCESSOR_DEREF);
    (*ac)->type = operand_type_lookup(t);
}

static void handle_accessor_component_ref(struct cl_accessor **ac, tree t)
{
    chain_accessor(ac, CL_ACCESSOR_ITEM);

    struct cl_type *type = operand_type_lookup(t);
    (*ac)->type         = type;
    (*ac)->data.item.id = accessor_item_lookup(type, TREE_OPERAND(t, 1));
}

static bool handle_accessor(struct cl_accessor **ac, tree *pt)
{
    tree t = *pt;
    if (NULL_TREE == t)
        TRAP;

    enum tree_code code = TREE_CODE(t);
    switch (code) {
        case ADDR_EXPR:
            handle_accessor_addr_expr(ac, t);
            break;

        case ARRAY_REF:
            handle_accessor_array_ref(ac, t);
            break;

#ifdef MEM_REF
        case MEM_REF:
            // MEM_REF appeared after 4.5.0 (should be equal to INDIRECT_REF)
#endif
        case INDIRECT_REF:
            handle_accessor_indirect_ref(ac, t);
            break;

        case COMPONENT_REF:
            handle_accessor_component_ref(ac, t);
            break;

        case BIT_FIELD_REF:
            SL_WARN_UNHANDLED_EXPR(t, "BIT_FIELD_REF");
            break;

        default:
            return false;
    }

    *pt = TREE_OPERAND(t, 0);
    return true;
}

static void free_cl_operand_data(struct cl_operand *op)
{
    // TODO
    (void) op;
}

static void handle_operand(struct cl_operand *op, tree t)
{
    op->code            = CL_OPERAND_VOID;
    op->loc.file        = NULL;
    op->loc.line        = -1;
    op->scope           = CL_SCOPE_GLOBAL;
    op->type            = NULL;
    op->accessor        = NULL;

    if (!t)
        return;

    // read type
    op->type = add_type_if_needed(t);

    // read accessor
    op->accessor = NULL;
    while (handle_accessor(&op->accessor, &t));

    if (NULL_TREE == t)
        TRAP;
    read_raw_operand(op, t);
}

static void handle_stmt_unop(gimple stmt, enum tree_code code,
                              struct cl_operand *dst, tree src_tree)
{
    struct cl_operand src;
    handle_operand(&src, src_tree);

    struct cl_insn cli;
    cli.code                    = CL_INSN_UNOP;
    cli.data.insn_unop.code     = CL_UNOP_ASSIGN;
    cli.data.insn_unop.dst      = dst;
    cli.data.insn_unop.src      = &src;
    read_gimple_location(&cli.loc, stmt);

    enum cl_unop_e *ptype = &cli.data.insn_unop.code;

    if (code != TREE_CODE(src_tree)) {
        switch (code) {
            case CONVERT_EXPR:
            case NOP_EXPR:
            case VAR_DECL:
                break;

            case TRUTH_NOT_EXPR:        *ptype = CL_UNOP_TRUTH_NOT;     break;
            case BIT_NOT_EXPR:          *ptype = CL_UNOP_BIT_NOT;       break;
            case NEGATE_EXPR:           *ptype = CL_UNOP_MINUS;         break;

            default:
                TRAP;
        }
    }

    cl->insn(cl, &cli);
    free_cl_operand_data(&src);
}

static void handle_stmt_binop(gimple stmt, enum tree_code code,
                              const struct cl_operand *dst,
                              tree src1_tree, tree src2_tree)
{
    struct cl_operand src1;
    struct cl_operand src2;

    handle_operand(&src1, src1_tree);
    handle_operand(&src2, src2_tree);

    struct cl_insn cli;
    cli.code                    = CL_INSN_BINOP;
    cli.data.insn_binop.dst     = dst;
    cli.data.insn_binop.src1    = &src1;
    cli.data.insn_binop.src2    = &src2;
    read_gimple_location(&cli.loc, stmt);

    enum cl_binop_e *ptype = &cli.data.insn_binop.code;

    switch (code) {
        case EQ_EXPR:               *ptype = CL_BINOP_EQ;               break;
        case NE_EXPR:               *ptype = CL_BINOP_NE;               break;
        case LT_EXPR:               *ptype = CL_BINOP_LT;               break;
        case GT_EXPR:               *ptype = CL_BINOP_GT;               break;
        case LE_EXPR:               *ptype = CL_BINOP_LE;               break;
        case GE_EXPR:               *ptype = CL_BINOP_GE;               break;
        case PLUS_EXPR:             *ptype = CL_BINOP_PLUS;             break;
        case MINUS_EXPR:            *ptype = CL_BINOP_MINUS;            break;
        case MULT_EXPR:             *ptype = CL_BINOP_MULT;             break;
        case TRUNC_DIV_EXPR:        *ptype = CL_BINOP_TRUNC_DIV;        break;
        case TRUNC_MOD_EXPR:        *ptype = CL_BINOP_TRUNC_MOD;        break;
        case RDIV_EXPR:             *ptype = CL_BINOP_RDIV;             break;
        case MIN_EXPR:              *ptype = CL_BINOP_MIN;              break;
        case MAX_EXPR:              *ptype = CL_BINOP_MAX;              break;
        case TRUTH_AND_EXPR:        *ptype = CL_BINOP_TRUTH_AND;        break;
        case TRUTH_OR_EXPR:         *ptype = CL_BINOP_TRUTH_OR;         break;
        case TRUTH_XOR_EXPR:        *ptype = CL_BINOP_TRUTH_XOR;        break;
        case BIT_AND_EXPR:          *ptype = CL_BINOP_BIT_AND;          break;
        case BIT_IOR_EXPR:          *ptype = CL_BINOP_BIT_IOR;          break;
        case BIT_XOR_EXPR:          *ptype = CL_BINOP_BIT_XOR;          break;
        case POINTER_PLUS_EXPR:     *ptype = CL_BINOP_POINTER_PLUS;     break;

#define SL_BINOP_UNHANDLED(what) \
    case what: SL_WARN_UNHANDLED_GIMPLE(stmt, #what); \
               cli.code = CL_INSN_NOP; \
               break;

        SL_BINOP_UNHANDLED(EXACT_DIV_EXPR)
        SL_BINOP_UNHANDLED(LSHIFT_EXPR)
        SL_BINOP_UNHANDLED(RSHIFT_EXPR)
        SL_BINOP_UNHANDLED(LROTATE_EXPR)
        SL_BINOP_UNHANDLED(RROTATE_EXPR)

        default:
            TRAP;
    }

    if (CL_INSN_NOP != cli.code)
        cl->insn(cl, &cli);
    free_cl_operand_data(&src1);
    free_cl_operand_data(&src2);
}

static void handle_stmt_assign(gimple stmt)
{
    struct cl_operand dst;
    handle_operand(&dst, gimple_assign_lhs(stmt));

    switch (gimple_num_ops(stmt)) {
        case 2:
            // unary operator (lhs + 1)
            handle_stmt_unop(stmt, gimple_assign_rhs_code(stmt),
                    &dst,
                    gimple_assign_rhs1(stmt));
            break;

        case 3:
            // binary operator (lhs + 2)
            handle_stmt_binop(stmt, gimple_assign_rhs_code(stmt),
                    &dst,
                    gimple_assign_rhs1(stmt),
                    gimple_assign_rhs2(stmt));
            break;

        default:
            TRAP;
    };
    free_cl_operand_data(&dst);
}

static void handle_stmt_call_args(gimple stmt)
{
    const int argc = gimple_call_num_args(stmt);
    int i;
    for (i = 0; i < argc; ++i) {
        struct cl_operand src;
        handle_operand(&src, gimple_call_arg(stmt, i));
        cl->insn_call_arg(cl, i + 1, &src);
        free_cl_operand_data(&src);
    }
}

static void handle_stmt_call(gimple stmt)
{
    tree op0 = gimple_call_fn(stmt);

    if (ADDR_EXPR == TREE_CODE(op0))
        // automagic dereference
        op0 = TREE_OPERAND(op0, 0);

    if (!op0)
        TRAP;

    // lhs
    struct cl_operand dst;
    handle_operand(&dst, gimple_call_lhs(stmt));

    // fnc is also operand (call through pointer, struct member, etc.)
    struct cl_operand fnc;
    handle_operand(&fnc, op0);

    // emit CALL insn
    struct cl_location loc;
    read_gimple_location(&loc, stmt);
    cl->insn_call_open(cl, &loc, &dst, &fnc);
    free_cl_operand_data(&dst);
    free_cl_operand_data(&fnc);

    // emit args
    handle_stmt_call_args(stmt);
    cl->insn_call_close(cl);

    if (ECF_NORETURN & gimple_call_flags(stmt)) {
        // this call never returns --> end of BB!!

        struct cl_insn cli;
        cli.code    = CL_INSN_ABORT;
        cli.loc     = loc;

        cl->insn(cl, &cli);
    }
}

static void handle_stmt_return(gimple stmt)
{
    struct cl_operand src;
    handle_operand(&src, gimple_return_retval(stmt));

    struct cl_insn cli;
    cli.code                    = CL_INSN_RET;
    cli.data.insn_ret.src       = &src;

    // FIXME: location info seems to be valid only for IMPLICIT 'return'
    // statement. It's really strange because it does not work properly
    // even with print_gimple_stmt()
    read_gimple_location(&cli.loc, stmt);
    cl->insn(cl, &cli);
    free_cl_operand_data(&src);
}

static /* const */ struct cl_type builtin_bool_type = {
    .uid            = /* FIXME */ -1,
    .code           = CL_TYPE_BOOL,
    .loc = {
        .file       = NULL,
        .line       = -1
    },
    .scope          = CL_SCOPE_GLOBAL,
    .name           = "<builtin_bool>",
    .size           = /* FIXME */ sizeof(bool)
};

static /* XXX const */ struct cl_operand stmt_cond_fixed_reg = {
    .code           = CL_OPERAND_REG,
    .loc = {
        .file       = NULL,
        .line       = -1
    },
    .scope          = CL_SCOPE_FUNCTION,
    .type           = &builtin_bool_type,
    .data = {
        .reg = {
            .id     = /* XXX */ 0x100000
        }
    }
};

static void handle_stmt_cond_br(gimple stmt, const char *then_label,
                                const char *else_label)
{
    struct cl_insn cli;
    cli.code                        = CL_INSN_COND;
    cli.data.insn_cond.src          = &stmt_cond_fixed_reg;
    cli.data.insn_cond.then_label   = then_label;
    cli.data.insn_cond.else_label   = else_label;

    read_gimple_location(&cli.loc, stmt);
    cl->insn(cl, &cli);

    // XXX
    stmt_cond_fixed_reg.data.reg.id ++;
}

static void handle_stmt_cond(gimple stmt)
{
    GIMPLE_CHECK(stmt, GIMPLE_COND);

    char *label_true = NULL;
    char *label_false = NULL;

    edge e;
    edge_iterator ei;
    struct basic_block_def *bb = stmt->gsbase.bb;
    FOR_EACH_EDGE(e, ei, bb->succs) {
        if (e->flags & EDGE_TRUE_VALUE) {
            struct basic_block_def *next = e->dest;
            label_true = index_to_label(next->index);
        }
        if (e->flags & EDGE_FALSE_VALUE) {
            struct basic_block_def *next = e->dest;
            label_false = index_to_label(next->index);
        }
    }

    if (!label_true || !label_false)
        TRAP;

    handle_stmt_binop(stmt,
            gimple_cond_code(stmt),
            &stmt_cond_fixed_reg,
            gimple_cond_lhs(stmt),
            gimple_cond_rhs(stmt));

    handle_stmt_cond_br(stmt, label_true, label_false);
    free(label_true);
    free(label_false);
}

static unsigned find_case_label_target(gimple stmt, int label_decl_uid)
{
    GIMPLE_CHECK(stmt, GIMPLE_SWITCH);

    edge e;
    edge_iterator ei;
    struct basic_block_def *switch_bb = stmt->gsbase.bb;
    FOR_EACH_EDGE(e, ei, switch_bb->succs) {
        // FIXME: treat e->flags somehow?

        struct basic_block_def *bb = e->dest;
        if (!bb)
            // edge with no target
            TRAP;

        // obtain gimple
        struct gimple_bb_info *bb_info = bb->il.gimple;
        if (!bb_info || ! bb_info->seq || !bb_info->seq->first)
            TRAP;

        // check whether first statement in BB is GIMPLE_LABEL
        gimple bb_stmt = bb_info->seq->first->stmt;
        if (!bb_stmt || GIMPLE_LABEL != bb_stmt->gsbase.code)
            continue;

        // get label declaration
        tree label = gimple_label_label(bb_stmt);
        if (!label)
            TRAP;

        if (label_decl_uid == LABEL_DECL_UID(label))
            // match
            return bb->index;
    }

    TRAP;
    // no matching GIMPLE_LABEL was found in BB successors
    // this should never happen
    return (unsigned) -1;
}

static void handle_stmt_switch(gimple stmt)
{
    struct cl_operand src;
    handle_operand(&src, gimple_switch_index(stmt));

    // emit insn_switch_open
    struct cl_location loc;
    read_gimple_location(&loc, stmt);
    cl->insn_switch_open(cl, &loc, &src);
    free_cl_operand_data(&src);

    unsigned i;
    for (i = 0; i < gimple_switch_num_labels(stmt); ++i) {
        tree case_decl = gimple_switch_label(stmt, i);
        if (!case_decl)
            TRAP;

        // lowest case value with same label
        struct cl_operand val_lo;
        tree case_low = CASE_LOW(case_decl);
        handle_operand(&val_lo, case_low);

        // highest case value with same lable
        struct cl_operand val_hi;
        tree case_high = CASE_HIGH(case_decl);
        if (!case_high)
            // there is no range, only one value
            case_high = case_low;
        handle_operand(&val_hi, case_high);

        // figure out where to jump in that case
        tree case_label = CASE_LABEL(case_decl);
        if (!case_label || LABEL_DECL != TREE_CODE(case_label))
            TRAP;

        // look for corresponding GIMPLE_LABEL in successor BBs
        int case_label_uid = LABEL_DECL_UID(case_label);
        unsigned label_uid = find_case_label_target(stmt, case_label_uid);
        const char *label = index_to_label(label_uid);

        // emit insn_switch_case
        read_gcc_location(&loc, EXPR_LOCATION(case_decl));
        cl->insn_switch_case(cl, &loc, &val_lo, &val_hi, label);

        free_cl_operand_data(&val_lo);
        free_cl_operand_data(&val_hi);
        free((char *) label);
    }

    // emit insn_switch_close
    cl->insn_switch_close(cl);
}

// callback of walk_gimple_seq declared in <gimple.h>
static tree cb_walk_gimple_stmt (gimple_stmt_iterator *iter,
                                 bool *subtree_done,
                                 struct walk_stmt_info *info)
{
    gimple stmt = gsi_stmt (*iter);
    bool show_gimple = SL_VERBOSE_GIMPLE & verbose;

    (void) subtree_done;
    (void) info;

    if (show_gimple) {
        printf("\n\t\t");
        print_gimple_stmt(stdout, stmt,
                          /* indentation */ 0,
                          TDF_LINENO);
    }

    enum gimple_code code = stmt->gsbase.code;
    switch (code) {
        case GIMPLE_COND:
            handle_stmt_cond(stmt);
            break;

        case GIMPLE_ASSIGN:
            handle_stmt_assign(stmt);
            break;

        case GIMPLE_CALL:
            handle_stmt_call(stmt);
            break;

        case GIMPLE_RETURN:
            handle_stmt_return(stmt);
            break;

        case GIMPLE_SWITCH:
            handle_stmt_switch(stmt);
            break;

        case GIMPLE_LABEL:
            // should be already handled by handle_stmt_switch
            break;

        case GIMPLE_ASM:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_ASM");
            break;

        case GIMPLE_PREDICT:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_PREDICT");
            break;

        default:
            TRAP;
    }

    if (show_gimple)
        printf("\n");

    return NULL;
}

// walk through gimple BODY using <gimple.h> API
static void handle_bb_gimple (gimple_seq body)
{
    struct walk_stmt_info info;
    memset (&info, 0, sizeof(info));
    walk_gimple_seq (body, cb_walk_gimple_stmt, NULL, &info);
}

static void handle_jmp_edge (edge e)
{
    struct basic_block_def *next = e->dest;

    struct cl_insn cli;
    cli.code                = CL_INSN_JMP;
    cli.data.insn_jmp.label = index_to_label(next->index);

    // no location for CL_INSN_JMP for now
    cli.loc.file = NULL;
    cli.loc.line = -1;

    cl->insn(cl, &cli);
    free((char *) cli.data.insn_jmp.label);
}

static void handle_fnc_bb (struct basic_block_def *bb)
{
    // declare bb
    char *label = index_to_label(bb->index);
    cl->bb_open(cl, label);
    free(label);

    // go through the bb's content
    struct gimple_bb_info *gimple = bb->il.gimple;
    if (NULL == gimple) {
        SL_WARN_UNHANDLED ("gimple not found");
        TRAP;
        return;
    }
    handle_bb_gimple(gimple->seq);

    // check for a fallthru successor
    edge e;
    edge_iterator ei = ei_start(bb->succs);
    if (ei_cond(ei, &e) && e->dest && (e->flags & EDGE_FALLTHRU)) {
        handle_jmp_edge(e);
        return;
    }
}

static void handle_fnc_cfg (struct control_flow_graph *cfg)
{
    struct basic_block_def *bb = cfg->x_entry_block_ptr;
    edge e;
    edge_iterator ei = ei_start(bb->succs);
    if (ei_cond(ei, &e) && e->dest) {
        // ENTRY block
        handle_jmp_edge(e);
        bb = bb->next_bb;
    }

    if (!bb)
        TRAP;

    while (/* skip EXIT block */ bb->next_bb) {
        handle_fnc_bb(bb);
        bb = bb->next_bb;
    }
}

// go through argument list ARGS of fnc declaration
static void handle_fnc_decl_arglist (tree args)
{
    int argc = 0;

    while (args) {
        struct cl_operand arg_src;
        handle_operand(&arg_src, args);
        arg_src.scope = CL_SCOPE_FUNCTION;

        cl->fnc_arg_decl(cl, ++argc, &arg_src);

        args = TREE_CHAIN (args);
    }
}

// handle FUNCTION_DECL tree node given as DECL
static void handle_fnc_decl (tree decl)
{
    tree ident = DECL_NAME (decl);
    if (!ident)
        TRAP;

    // emit fnc declaration
    struct cl_operand fnc;
    handle_operand(&fnc, decl);
    cl->fnc_open(cl, &fnc);

    // emit arg declarations
    tree args = DECL_ARGUMENTS (decl);
    handle_fnc_decl_arglist (args);

    // obtain CFG for current function
    struct function *def = DECL_STRUCT_FUNCTION (decl);
    if (!def || !def->cfg) {
        SL_WARN_UNHANDLED ("CFG not found");
        return;
    }

    // go through CFG
    handle_fnc_cfg(def->cfg);

    // fnc traverse complete
    cl->fnc_close(cl);
}

// callback of tree pass declared in <tree-pass.h>
static unsigned int sl_pass_execute (void)
{
    if (!current_function_decl) {
        SL_WARN_UNHANDLED ("NULL == current_function_decl");
        return 0;
    }

    if (FUNCTION_DECL != TREE_CODE (current_function_decl)) {
        SL_WARN_UNHANDLED ("TREE_CODE (current_function_decl)");
        return 0;
    }

    handle_fnc_decl (current_function_decl);
    return 0;
}

// pass description according to <tree-pass.h> API
static struct opt_pass sl_pass = {
    .type                       = GIMPLE_PASS,
    .name                       = "slplug",
    .gate                       = NULL,
    .execute                    = sl_pass_execute,
    .properties_required        = PROP_cfg | PROP_gimple_any,
    // ...
};

// definition of a new pass provided by the plug-in
static struct register_pass_info sl_plugin_pass = {
    .pass                       = &sl_pass,

    // cfg ... control_flow_graph
    .reference_pass_name        = "cfg",

    .ref_pass_instance_number   = 0,
    .pos_op                     = PASS_POS_INSERT_AFTER,
};

// callback called as last (if the plug-in does not crash before)
static void cb_finish (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SL_LOG_FNC;

    type_db_destroy(type_db);
    cl->destroy(cl);
    cl_global_cleanup();
}

// callback called on start of input file processing
static void cb_start_unit (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SL_LOG ("processing input file '%s'", main_input_filename);
    cl->file_open(cl, input_filename);
}

static void cb_finish_unit (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SL_LOG_FNC;
    cl->file_close(cl);
}

// register callbacks for plug-in NAME
static void sl_regcb (const char *name) {
    // passing NULL as CALLBACK to register_callback stands for virtual callback

    // register new pass provided by the plug-in
    register_callback (name, PLUGIN_PASS_MANAGER_SETUP,
                       /* callback */   NULL,
                       &sl_plugin_pass);

    register_callback (name, PLUGIN_FINISH_UNIT,
                       cb_finish_unit,
                       /* user_data */  NULL);

    register_callback (name, PLUGIN_FINISH,
                       cb_finish,
                       /* user_data */  NULL);

    register_callback (name, PLUGIN_INFO,
                       /* callback */   NULL,
                       &sl_info);

    register_callback (name, PLUGIN_START_UNIT,
                       cb_start_unit,
                       /* user_data */  NULL);
}

struct sl_plug_options {
    bool                    dump_types;
    bool                    use_dotgen;
    bool                    use_pp;
    bool                    use_symexec;
    bool                    use_typedot;
    const char              *gl_dot_file;
    const char              *pp_out_file;
    const char              *symexec_args;
    const char              *type_dot_file;
};

static int slplug_init(const struct plugin_name_args *info,
                       struct sl_plug_options *opt)
{
    // initialize opt data
    memset (opt, 0, sizeof(*opt));
    opt->use_symexec         = true;
    opt->symexec_args        = "";

    // initialize global plug-in name
    plugin_name = info->full_name;

    // obtain arg list
    const int argc                      = info->argc;
    const struct plugin_argument *argv  = info->argv;

    // handle plug-in args
    int i;
    for (i = 0; i < argc; ++i) {
        const struct plugin_argument *arg = argv + i;
        const char *key     = arg->key;
        const char *value   = arg->value;

        if (STREQ(key, "verbose")) {
            verbose = (value)
                ? atoi(value)
                : ~0;

        } else if (STREQ(key, "version")) {
            // do not use info->version yet
            printf("\n%s\n", sl_info.version);
            return EXIT_FAILURE;

        } else if (STREQ(key, "help")) {
            // do not use info->help yet
            printf ("\n%s\n", sl_info.help);
            return EXIT_FAILURE;

        } else if (STREQ(key, "bypass-symexec")) {
            opt->use_symexec    = false;
            // TODO: warn about ignoring extra value?

        } else if (STREQ(key, "dump-pp")) {
            opt->use_pp         = true;
            opt->pp_out_file    = value;

        } else if (STREQ(key, "dump-types")) {
            opt->dump_types     = true;
            // TODO: warn about ignoring extra value?

        } else if (STREQ(key, "gen-dot")) {
            opt->use_dotgen     = true;
            opt->gl_dot_file    = value;

        } else if (STREQ(key, "symexec-args")) {
            opt->symexec_args = (value)
                ? value
                : "";

        } else if (STREQ(key, "type-dot")) {
            if (value) {
                opt->use_typedot    = true;
                opt->type_dot_file  = value;
            } else {
                fprintf(stderr, "%s: error: "
                        "mandatory value omitted for type-dot\n",
                        plugin_name);
                return EXIT_FAILURE;
            }

        } else {
            SL_WARN_UNHANDLED(key);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static bool sl_append_listener(struct cl_code_listener *chain,
                               const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *config_string;
    int rv = vasprintf(&config_string, fmt, ap);
    SL_ASSERT(0 < rv);
    va_end(ap);

    struct cl_code_listener *cl = cl_code_listener_create(config_string);
    free(config_string);

    if (!cl) {
        // FIXME: deserves a big comment (subtle)
        chain->destroy(chain);
        return false;
    }

    cl_chain_append(chain, cl);
    return true;
}

static bool sl_append_def_listener(struct cl_code_listener *chain,
                                   const char *listener, const char *args,
                                   const struct sl_plug_options *opt)
{
    const char *cld = (opt->use_symexec)
        ? "unfold_switch,unify_labels_gl"
        : "unify_labels_fnc,unify_regs,unify_vars";

    return sl_append_listener(chain,
            "listener=\"%s\" listener_args=\"%s\" cld=\"%s\"",
            listener, args, cld);
}

static struct cl_code_listener*
create_cl_chain(const struct sl_plug_options *opt)
{
    struct cl_code_listener *chain = cl_chain_create();
    if (!chain)
        // error message already emitted
        return NULL;

    if (SL_VERBOSE_LOCATION & verbose) {
        if (!sl_append_listener(chain, "listener=\"locator\""))
            return NULL;
    }

    if (opt->use_pp) {
        const char *use_listener = (opt->dump_types)
            ? "pp_with_types"
            : "pp";

        const char *out = (opt->pp_out_file)
            ? opt->pp_out_file
            : "";

        if (!sl_append_def_listener(chain, use_listener, out, opt))
            return NULL;
    }

    if (opt->use_dotgen) {
        const char *gl_dot = (opt->gl_dot_file)
            ? opt->gl_dot_file
            : "";
        if (!sl_append_def_listener(chain, "dotgen", gl_dot, opt))
            return NULL;
    }

    if (opt->use_typedot
            && !sl_append_def_listener(chain, "typedot", opt->type_dot_file, opt))
        return NULL;

    if (opt->use_symexec
            && !sl_append_def_listener(chain, "easy", opt->symexec_args, opt))
        return NULL;

    return chain;
}

// plug-in initialization according to gcc plug-in API
int plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version)
{
    struct sl_plug_options opt;

    // suppress compile-time warning when (DEBUG_SLPLUG == 0)
    (void) version;

    // global initialization
    int rv = slplug_init(plugin_info, &opt);
    if (rv)
        return rv;

    // print something like "hello world!"
    SL_LOG_FNC;
    SL_LOG ("using gcc %s %s, built at %s", version->basever,
             version->devphase, version->datestamp);

    // TODO: check for compatibility with particular gcc version here

    // initialize code listener
    cl_global_init_defaults(NULL, verbose & SL_VERBOSE_PLUG);
    cl = create_cl_chain(&opt);
    SL_ASSERT(cl);

    // initialize type database
    type_db = type_db_create();
    SL_ASSERT(type_db);

    // try to register callbacks (and virtual callbacks)
    sl_regcb (plugin_info->base_name);
    SL_LOG ("'%s' successfully initialized", plugin_info->version);

    return 0;
}
