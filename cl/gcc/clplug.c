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

// see http://www.delorie.com/gnu/docs/gcc/gccint_31.html

// this include has to be the first (according the gcc plug-in API)
#include <gcc-plugin.h>
#include <plugin-version.h>

#include <cl/code_listener.h>

#include "../config_cl.h"
#include "../version_cl.h"

// avoid the following warnings with -Wundef:
//      ../include/gcc/obstack.h:299:23: warning: "__NeXT__" is not defined
//      ../include/gcc/vec.h:440:5: warning: "ENABLE_CHECKING" is not defined
//      ../include/gcc/toplev.h:54:6: warning: "ENABLE_CHECKING" is not defined
#ifndef __NeXT__
#   define __NeXT__ 0
#endif
#ifndef ENABLE_CHECKING
#   define ENABLE_CHECKING 0
#endif

#include <coretypes.h>
#include <diagnostic.h>
#include <ggc.h>
#include <hashtab.h>

// this include has to be before <gcc/function.h>; otherwise it will NOT compile
#include <tm.h>

#include <function.h>
#include <gimple.h>
#include <input.h>
#include <real.h>
#include <toplev.h>
#include <tree-pass.h>

#ifdef __cplusplus
#   define C99_FIELD(field)
#else
#   define C99_FIELD(field) .field =
#endif

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <stdarg.h>

// required by realpath(3)
#include <limits.h>
#include <stdlib.h>

#ifndef STREQ
#   define STREQ(s1, s2) (0 == strcmp(s1, s2))
#endif

// our alternative to GGC_CNEW, used prior to gcc 4.6.x
#define CL_ZNEW(type) (type *) xcalloc(1, sizeof(type))

#define CL_ZNEW_ARRAY(type, cnt) (type *) xcalloc(cnt, sizeof(type))

// our alternative to GGC_RESIZEVEC, used prior to gcc 4.6.x
#define CL_RESIZEVEC(type, p, cnt) (type *) xrealloc((p), sizeof(type) * (cnt))

// somewhere after 4.5.0, the declaration has been moved to
// gimple-pretty-print.h, which is no longer available for public
extern void print_gimple_stmt(FILE *, gimple, int, int);

// this in fact uses gcc's fancy_abort()
#define CL_ASSERT(expr) \
    if (!(expr)) abort()

// low-level strictly local error/warning emitter
#define CL_PRINT(what, ...) do {                                            \
    fprintf(stderr, "%s: %s: ", plugin_name, (what));                       \
    fprintf(stderr, __VA_ARGS__);                                           \
    fprintf(stderr, "\n");                                                  \
} while (0)

#define CL_ERROR(...)   CL_PRINT("error",   __VA_ARGS__)
#define CL_WARN(...)    CL_PRINT("warning", __VA_ARGS__)

#define CL_DEBUG(...) do {                                                  \
    if (verbose)                                                            \
        CL_PRINT("debug", __VA_ARGS__);                                     \
} while (0)

#if CLPLUG_SILENT
#   define CL_WARN_UNHANDLED(...)
#   define CL_WARN_UNHANDLED_WITH_LOC(...)
#else
#   define CL_WARN_UNHANDLED(what)                                          \
        fprintf(stderr, "%s:%d: warning: "                                  \
                "'%s' not handled in '%s' [internal location]\n",           \
                __FILE__, __LINE__, (what), __FUNCTION__)

// experimental use of warning_at()
#   define CL_WARN_UNHANDLED_WITH_LOC(loc, ...) do {                        \
        warning_at((loc), 0, __VA_ARGS__);                                  \
        fprintf(stderr,                                                     \
                "%s:%d: note: raised from '%s' [internal location]\n",      \
                __FILE__, __LINE__, __FUNCTION__);                          \
    } while (0)

#endif // CLPLUG_SILENT

#define CL_WARN_UNHANDLED_GIMPLE(stmt, what) \
    CL_WARN_UNHANDLED_WITH_LOC((stmt)->gsbase.location, "unhandled " what)

#if CL_DEBUG_GCC_TREE
#   define CL_DEBUG_TREE(expr) debug_tree(expr)
#else
#   define CL_DEBUG_TREE(expr)
#endif

#define CL_WARN_UNHANDLED_EXPR(expr, what) do { \
    CL_WARN_UNHANDLED_WITH_LOC(EXPR_LOCATION(expr), "unhandled " what); \
    CL_DEBUG_TREE(expr); \
} while (0)

// name of the plug-in given by gcc during initialization
static const char *plugin_base_name = "???";
static const char *plugin_name = "[uninitialized]";
static const char *plugin_name_alloc;

// verbose bitmask
static int verbose = 0;

// plug-in meta-data according to gcc plug-in API
static struct plugin_info cl_info = {
    C99_FIELD(version) "%s [code listener SHA1 " CL_GIT_SHA1 "]",
    C99_FIELD(help   ) "%s [code listener SHA1 " CL_GIT_SHA1 "]\n"
"\n"
"Usage: gcc -fplugin=%s [OPTIONS] ...\n"
"\n"
"OPTIONS:\n"
"    -fplugin-arg-%s-help\n"
"    -fplugin-arg-%s-version\n"
"    -fplugin-arg-%s-args=PEER_ARGS                 args given to analyzer\n"
"    -fplugin-arg-%s-dry-run                        do not run the analyzer\n"
"    -fplugin-arg-%s-dump-pp[=OUTPUT_FILE]          dump linearized code\n"
"    -fplugin-arg-%s-dump-types                     dump also type info\n"
"    -fplugin-arg-%s-gen-dot[=GLOBAL_CG_FILE]       generate CFGs\n"
"    -fplugin-arg-%s-pid-file=FILE                  write PID of self to FILE\n"
"    -fplugin-arg-%s-preserve-ec                    do not affect exit code\n"
"    -fplugin-arg-%s-type-dot=TYPE_GRAPH_FILE       generate type graphs\n"
"    -fplugin-arg-%s-verbose[=VERBOSITY_LEVEL]      turn on verbose mode\n"
};

// works on Darwin without allocating extra memory, but will not work on Windows
const char* safe_basename(const char *path)
{
    const char *slash = strrchr((char *) path, '/');
    if (slash && slash[1])
        return slash + 1;
    else
        return path;
}

static void init_plugin_name(const struct plugin_name_args *info)
{
    if (!STREQ("[uninitialized]", plugin_name)) {
        // do not use CL_ERROR here, as plugin_name may be misleading
        fprintf(stderr, "%s: error: "
                "attempt to initialize code listener multiple times!\n",
                info->full_name);
        abort();
    }

    // initialize global plug-in name
    plugin_name = info->full_name;
    plugin_name_alloc = realpath(plugin_name, NULL);
    if (plugin_name_alloc)
        plugin_name = plugin_name_alloc;

    plugin_base_name = safe_basename((char *) plugin_name);

    // read plug-in base name
    const char *name = info->base_name;
    char *msg;

    // substitute name in 'version' string
    if (-1 == asprintf(&msg, cl_info.version, plugin_base_name))
        // OOM
        abort();
    else
        cl_info.version = msg;

    // substitute name in 'help' string
    // FIXME: error-prone approach, this should be automated somehow!
    if (-1 == asprintf(&msg, cl_info.help, plugin_base_name,
                       name, name, name, name,
                       name, name, name, name,
                       name, name, name, name))
        // OOM
        abort();
    else
        cl_info.help = msg;
}

static void free_plugin_name(void)
{
    // free version/help strings
    free((char *) cl_info.version);
    free((char *) cl_info.help);
    memset(&cl_info, 0, sizeof cl_info);

    // free plug-in name
    free((char *) plugin_name_alloc);
    plugin_name_alloc = NULL;
}

// FIXME: suboptimal interface of CL messaging
static bool preserve_ec;
static int cnt_errors;
static int cnt_warnings;

static void dummy_printer(const char *msg)
{
    (void) msg;
}

static void trivial_printer(const char *msg)
{
    fprintf(stderr, "%s [-fplugin=%s]\n", msg, plugin_base_name);
}

static void cl_warn(const char *msg)
{
    trivial_printer(msg);
    ++cnt_warnings;
}

static void cl_error(const char *msg)
{
    trivial_printer(msg);
    ++cnt_errors;
}

typedef htab_t type_db_t;
typedef htab_t var_db_t;

static struct cl_code_listener *cl = NULL;
static type_db_t type_db = NULL;
static var_db_t var_db = NULL;

static hashval_t type_db_hash(const void *p)
{
    const struct cl_type *type = (const struct cl_type *) p;
    return type->uid;
}

static hashval_t var_db_hash(const void *p)
{
    const struct cl_var *var = (const struct cl_var *) p;
    return var->uid;
}

static int type_db_eq(const void *p1, const void *p2)
{
    const struct cl_type *type1 = (const struct cl_type *) p1;
    const struct cl_type *type2 = (const struct cl_type *) p2;
    return type1->uid == type2->uid;
}

static int var_db_eq(const void *p1, const void *p2)
{
    const struct cl_var *var1 = (const struct cl_var *) p1;
    const struct cl_var *var2 = (const struct cl_var *) p2;
    return var1->uid == var2->uid;
}

static void type_db_free(void *p)
{
    const struct cl_type *type = (const struct cl_type *) p;
    if (type->item_cnt)
        free(type->items);

    free(p);
}

static void free_cl_operand(const struct cl_operand *ptr);

static void free_cl_accessor(struct cl_accessor *ac)
{
    while (ac) {
        struct cl_accessor *next = ac->next;

        if (CL_ACCESSOR_DEREF_ARRAY == ac->code)
            // FIXME: unguarded recursion
            free_cl_operand(ac->data.array.index);

        free((void *) ac);
        ac = next;
    }
}

// TODO: join this with free_cl_operand_data
static void free_cl_operand(const struct cl_operand *ptr)
{
    struct cl_operand *op = (struct cl_operand *) ptr;
    free_cl_accessor(op->accessor);
    free(op);
}

static void free_initials(struct cl_initializer *initial)
{
    while (initial) {
        struct cl_insn *cli = &initial->insn;

        const enum cl_insn_e code = cli->code;
        switch (code) {
            case CL_INSN_UNOP:
                free_cl_operand(cli->data.insn_unop.dst);
                free_cl_operand(cli->data.insn_unop.src);
                break;

            case CL_INSN_BINOP:
                free_cl_operand(cli->data.insn_binop.dst);
                free_cl_operand(cli->data.insn_binop.src1);
                free_cl_operand(cli->data.insn_binop.src2);
                break;

            default:
                CL_BREAK_IF("free_initials() hit an invalid initializer");
                return;
        }

        struct cl_initializer *next = initial->next;
        free(initial);
        initial = next;
    }
}

static void var_db_free(void *p)
{
    const struct cl_var *var = (const struct cl_var *) p;
    free_initials(var->initial);
    free(p);
}

static type_db_t type_db_create(void)
{
    return htab_create_alloc(/* FIXME: hardcoded for now */ 0x100,
                             type_db_hash, type_db_eq, type_db_free,
                             xcalloc, free);
}

static var_db_t var_db_create(void)
{
    return htab_create_alloc(/* FIXME: hardcoded for now */ 0x100,
                             var_db_hash, var_db_eq, var_db_free,
                             xcalloc, free);
}

static void type_db_destroy(type_db_t db)
{
    htab_delete(db);
}

static void var_db_destroy(var_db_t db)
{
    htab_delete(db);
}

static struct cl_type* type_db_lookup(type_db_t db, int uid)
{
    struct cl_type type;
    type.uid = uid;

    return (struct cl_type *) htab_find(db, &type);
}

static struct cl_var* var_db_lookup(var_db_t db, int uid)
{
    struct cl_var var;
    var.uid = uid;

    return (struct cl_var *) htab_find(db, &var);
}

static void type_db_insert(type_db_t db, struct cl_type *type)
{
    void **slot = htab_find_slot(db, type, INSERT);
    CL_ASSERT(slot);
    *slot = type;
}

static void var_db_insert(var_db_t db, struct cl_var *var)
{
    void **slot = htab_find_slot(db, var, INSERT);
    CL_ASSERT(slot);
    *slot = var;
}

static bool error_detected(void)
{
    return global_dc && global_dc->diagnostic_count[DK_ERROR];
}

static void read_gcc_location(struct cl_loc *loc, location_t gcc_loc)
{
    expanded_location exp_loc = expand_location(gcc_loc);
    loc->file   = exp_loc.file;
    loc->line   = exp_loc.line;
    loc->column = /* FIXME: is this field always valid? */ exp_loc.column;
    loc->sysp   = /* FIXME: is this field always valid? */ exp_loc.sysp;
}

static bool read_gimple_location(struct cl_loc *loc, const_gimple g)
{
    location_t gcc_loc = g->gsbase.location;
    read_gcc_location(loc, gcc_loc);
    return !!gcc_loc;
}

static char* index_to_label(unsigned idx) {
    char *label;
    int rv = asprintf(&label, "%u:%u", DECL_UID(current_function_decl), idx);
    CL_ASSERT(0 < rv);
    return label;
}

static int get_type_sizeof(tree t)
{
    tree size = TYPE_SIZE_UNIT(t);
    if (NULL_TREE == size)
        return 0;

    if (VAR_DECL == TREE_CODE(size))
        // C99 stack-allocated arrays of variable size not known at compile-time
        return 0;

    CL_BREAK_IF(TREE_INT_CST_HIGH(size));

    return TREE_INT_CST_LOW(size);
}

static void read_base_type(struct cl_type *clt, tree type)
{
    // store sizeof
    clt->size = get_type_sizeof(type);
    clt->is_unsigned = TYPE_UNSIGNED(type);

    tree name = TYPE_NAME(type);
    if (NULL_TREE == name)
        return;

    if (TYPE_DECL == TREE_CODE(type)) {
        CL_WARN_UNHANDLED_EXPR(type, "TYPE_DECL");
        return;
    }

    // TODO: scope
    if (IDENTIFIER_NODE == TREE_CODE(name)) {
        clt->name = IDENTIFIER_POINTER(name);
        // read_gcc_location(&clt->loc, DECL_SOURCE_LOCATION(type));
    }
    else {
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
    CL_BREAK_IF(NULL_TREE == t);
    tree type = TREE_TYPE(t);

    CL_BREAK_IF(NULL_TREE == type);
    return add_bare_type_if_needed(type);
}

static int dig_field_offset(tree t)
{
    // read byte offset
    tree node = DECL_FIELD_OFFSET(t);
    CL_BREAK_IF(INTEGER_CST != TREE_CODE(node) || TREE_INT_CST_HIGH(node));
    int offset = TREE_INT_CST_LOW(node) << 3;

    // read bit offset
    node = DECL_FIELD_BIT_OFFSET(t);
    CL_BREAK_IF(INTEGER_CST != TREE_CODE(node) || TREE_INT_CST_HIGH(node));
    offset += TREE_INT_CST_LOW(node);

    // return total offset [in bits]
    return offset;
}

static void dig_record_type(struct cl_type *clt, tree t)
{
    for (t = TYPE_FIELDS(t); t; t = TREE_CHAIN(t)) {
        if (TYPE_DECL == TREE_CODE(t)) {
            CL_WARN_UNHANDLED_EXPR(t, "TYPE_DECL");
            continue;
        }

        // TODO: chunk allocation ?
        clt->items = CL_RESIZEVEC(struct cl_type_item, clt->items,
                                  clt->item_cnt + 1);

        struct cl_type_item *item = &clt->items[clt->item_cnt ++];
        item->type = /* recursion */ add_type_if_needed(t);
        item->name = /* possibly anonymous member */ NULL;

        // FIXME: bytes as units are too coarse, we should switch to bits
        item->offset = dig_field_offset(t) >> 3;

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
    clt->items = CL_ZNEW(struct cl_type_item);
    struct cl_type_item *item = clt->items;
    item->type = add_type_if_needed(t);
    item->name = NULL;

    // dig arg types
    for (t = TYPE_ARG_TYPES(t); t; t = TREE_CHAIN(t)) {
        tree type = TREE_VALUE(t);
        if ((1 < clt->item_cnt) && VOID_TYPE == TREE_CODE(type)) {
#ifndef NDEBUG
            // check there is no non-void type in the chain
            while ((t = TREE_CHAIN(t)))
                CL_BREAK_IF(VOID_TYPE != TREE_CODE(TREE_VALUE(t)));
#endif
            // for some reason, GCC gives us some trailing void types, drop them
            break;
        }

        // TODO: chunk allocation ?
        clt->items = CL_RESIZEVEC(struct cl_type_item, clt->items,
                                  clt->item_cnt + 1);

        struct cl_type_item *item = &clt->items[clt->item_cnt ++];
        item->type = /* recursion */ add_bare_type_if_needed(type);
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
            clt->items = CL_ZNEW(struct cl_type_item);
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
            clt->item_cnt = 1;
            clt->items = CL_ZNEW(struct cl_type_item);
            clt->items[0].type = /* recursion */ add_type_if_needed(type);
            clt->array_size = get_fixed_array_size(type);
            break;

        case FUNCTION_TYPE:
            clt->code = CL_TYPE_FNC;
            clt->size = 0;
            dig_fnc_type(clt, type);
            break;

        case REFERENCE_TYPE:
            // FIXME: it comes only on 32bit build of gcc (seems vararg related)
            clt->code = CL_TYPE_UNKNOWN;
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

        case REAL_TYPE:
            clt->code = CL_TYPE_REAL;
            break;

        case METHOD_TYPE:
            CL_WARN_UNHANDLED_EXPR(type, "METHOD_TYPE");
            break;

        default:
            CL_BREAK_IF("read_specific_type() got something special");
    };
}

static struct cl_type* add_bare_type_if_needed(tree type)
{
    // hashtab lookup
    const int uid = TYPE_UID(type);
    struct cl_type *clt = type_db_lookup(type_db, uid);
    if (clt)
        // type already hashed
        return clt;

    // insert new type into hashtab
    clt = CL_ZNEW(struct cl_type);
    clt->uid = uid;
    type_db_insert(type_db, clt);

    // read type (recursively if needed)
    read_base_type(clt, type);
    read_specific_type(clt, type);

    return clt;
}

static enum cl_scope_e get_decl_scope(tree t)
{
    if (VAR_DECL == TREE_CODE(t) && TREE_STATIC(t))
        // treat static variables as static, no matter where they're declared
        return CL_SCOPE_STATIC;

    tree ctx = DECL_CONTEXT(t);
    if (ctx) {
        enum tree_code code = TREE_CODE(ctx);
        switch (code) {
            case FUNCTION_DECL:
                return CL_SCOPE_FUNCTION;

            case TRANSLATION_UNIT_DECL:
                break;

            case RECORD_TYPE:
                CL_WARN_UNHANDLED_EXPR(ctx, "RECORD_TYPE)");
                return CL_SCOPE_STATIC;

            default:
                CL_BREAK_IF("unhandled declaration context");
        }
    }

    return (TREE_PUBLIC(t))
        ? CL_SCOPE_GLOBAL
        : CL_SCOPE_STATIC;
}

static const char* get_decl_name(tree t)
{
    tree name = DECL_NAME(t);
    return (name)
        ? IDENTIFIER_POINTER(name)
        : NULL;
}

static int field_lookup(tree op, tree field)
{
    tree type = TREE_TYPE(op);
    CL_BREAK_IF(NULL_TREE == type);

    tree t = TYPE_FIELDS(type);
    int i;
    for (i = 0; t; t = TREE_CHAIN(t), ++i)
        if (t == field)
            return i;

    // not found
    CL_BREAK_IF("field_lookup() has failed");
    return 0;
}

static int bitfield_lookup(tree op)
{
    // compute the offset we are looking for [in bits]
    const int offset = TREE_INT_CST_LOW(TREE_OPERAND(op, 2));

    tree type = TREE_TYPE(TREE_OPERAND(op, 0));
    CL_BREAK_IF(NULL_TREE == type);

    tree t = TYPE_FIELDS(type);
    int i;
    for (i = 0; t; t = TREE_CHAIN(t), ++i) {
        if (TYPE_DECL == TREE_CODE(t))
            continue;

        if (offset == dig_field_offset(t))
            return i;
    }

    // not found
    CL_WARN_UNHANDLED_EXPR(op, "BIT_FIELD_REF");
    return 0;
}

static void handle_operand(struct cl_operand *op, tree t);

static bool translate_unop_code(enum cl_unop_e *pDst, enum tree_code code) {
    switch (code) {
        case CONVERT_EXPR:
        case NOP_EXPR:
        case VAR_DECL:
        case FIX_TRUNC_EXPR:
            *pDst = CL_UNOP_ASSIGN;
            return true;

        case TRUTH_NOT_EXPR:
            *pDst = CL_UNOP_TRUTH_NOT;
            return true;

        case BIT_NOT_EXPR:
            *pDst = CL_UNOP_BIT_NOT;
            return true;

        case NEGATE_EXPR:
            *pDst = CL_UNOP_MINUS;
            return true;

        case ABS_EXPR:
            *pDst = CL_UNOP_ABS;
            return true;

        case FLOAT_EXPR:
            *pDst = CL_UNOP_FLOAT;
            return true;

        default:
            return false;
    }
}

static bool translate_binop_code(enum cl_binop_e *pDst, enum tree_code code) {
    switch (code) {
        case EQ_EXPR:               *pDst = CL_BINOP_EQ;               break;
        case NE_EXPR:               *pDst = CL_BINOP_NE;               break;
        case LT_EXPR:               *pDst = CL_BINOP_LT;               break;
        case GT_EXPR:               *pDst = CL_BINOP_GT;               break;
        case LE_EXPR:               *pDst = CL_BINOP_LE;               break;
        case GE_EXPR:               *pDst = CL_BINOP_GE;               break;
        case PLUS_EXPR:             *pDst = CL_BINOP_PLUS;             break;
        case MINUS_EXPR:            *pDst = CL_BINOP_MINUS;            break;
        case MULT_EXPR:             *pDst = CL_BINOP_MULT;             break;
        case EXACT_DIV_EXPR:        *pDst = CL_BINOP_EXACT_DIV;        break;
        case TRUNC_DIV_EXPR:        *pDst = CL_BINOP_TRUNC_DIV;        break;
        case TRUNC_MOD_EXPR:        *pDst = CL_BINOP_TRUNC_MOD;        break;
        case RDIV_EXPR:             *pDst = CL_BINOP_RDIV;             break;
        case MIN_EXPR:              *pDst = CL_BINOP_MIN;              break;
        case MAX_EXPR:              *pDst = CL_BINOP_MAX;              break;
        case TRUTH_AND_EXPR:        *pDst = CL_BINOP_TRUTH_AND;        break;
        case TRUTH_OR_EXPR:         *pDst = CL_BINOP_TRUTH_OR;         break;
        case TRUTH_XOR_EXPR:        *pDst = CL_BINOP_TRUTH_XOR;        break;
        case BIT_AND_EXPR:          *pDst = CL_BINOP_BIT_AND;          break;
        case BIT_IOR_EXPR:          *pDst = CL_BINOP_BIT_IOR;          break;
        case BIT_XOR_EXPR:          *pDst = CL_BINOP_BIT_XOR;          break;
        case POINTER_PLUS_EXPR:     *pDst = CL_BINOP_POINTER_PLUS;     break;
        case LSHIFT_EXPR:           *pDst = CL_BINOP_LSHIFT;           break;
        case RSHIFT_EXPR:           *pDst = CL_BINOP_RSHIFT;           break;
        case LROTATE_EXPR:          *pDst = CL_BINOP_LROTATE;          break;
        case RROTATE_EXPR:          *pDst = CL_BINOP_RROTATE;          break;

        default:
            return false;
    }

    return true;
}

struct cl_accessor* dup_ac(const struct cl_accessor *tpl)
{
    struct cl_accessor *dup = CL_ZNEW(struct cl_accessor);
    memcpy(dup, tpl, sizeof *dup);

    const enum cl_accessor_e code = tpl->code;
    if (CL_ACCESSOR_DEREF_ARRAY != code)
        return dup;

    struct cl_operand *idx = CL_ZNEW(struct cl_operand);
    memcpy(idx, tpl->data.array.index, sizeof *idx);
    dup->data.array.index = idx;
    return dup;
}

void dup_ac_chain(struct cl_accessor **ac_first, struct cl_accessor ***ac_last)
{
    *ac_last = ac_first;
    struct cl_accessor *ac = *ac_first;
    if (!ac)
        // empty chain
        return;

    struct cl_accessor *dup = dup_ac(ac);
    *ac_first = dup;
    *ac_last = &dup->next;

    for (ac = ac->next ; ac; ac = ac->next) {
        CL_BREAK_IF("not yet tested, but the count of stars promises fun ;-)");
        dup = dup_ac(ac);
        **ac_last = dup;
        *ac_last = &dup->next;
    }
}

static void read_initials(struct cl_var *var, struct cl_initializer **pinit,
                          tree ctor, struct cl_accessor *ac)
{
    var->initialized = true;

    // dig target type
    struct cl_type *clt = add_type_if_needed(ctor);

    const enum tree_code code = TREE_CODE(ctor);
    if (CONSTRUCTOR != code) {
        // allocate an initializer node
        struct cl_initializer *initial = CL_ZNEW(struct cl_initializer);
        initial->insn.loc = var->loc;
        *pinit = initial;

        struct cl_operand *dst = CL_ZNEW(struct cl_operand);
        dst->code = CL_OPERAND_VAR;
        dst->type = clt;
        dst->accessor = ac;
        dst->data.var = var;
        // TODO: scope

        enum cl_binop_e binop;
        if (translate_binop_code(&binop, code)) {
            struct cl_operand *src1 = CL_ZNEW(struct cl_operand);
            struct cl_operand *src2 = CL_ZNEW(struct cl_operand);

            tree op0 = TREE_OPERAND(ctor, 0);
            if (NOP_EXPR == TREE_CODE(op0))
                op0 = TREE_OPERAND(op0, 0);

            handle_operand(src1, op0);
            handle_operand(src2, TREE_OPERAND(ctor, 1));

            initial->insn.code                      = CL_INSN_BINOP;
            initial->insn.data.insn_binop.code      = binop;
            initial->insn.data.insn_binop.dst       = dst;
            initial->insn.data.insn_binop.src1      = src1;
            initial->insn.data.insn_binop.src2      = src2;
            return;
        }

        struct cl_operand *src = CL_ZNEW(struct cl_operand);
        enum cl_unop_e unop = CL_UNOP_ASSIGN;
        const bool is_unop = translate_unop_code(&unop, code);
        handle_operand(src, (is_unop)
                ? TREE_OPERAND(ctor, 0)
                : ctor);

        initial->insn.code                      = CL_INSN_UNOP;
        initial->insn.data.insn_unop.code       = unop;
        initial->insn.data.insn_unop.dst        = dst;
        initial->insn.data.insn_unop.src        = src;
        return;
    }

    // allocate array of nested initializers
    const bool isArray = (CL_TYPE_ARRAY == clt->code);

    unsigned idx;
    tree field, value;
    FOR_EACH_CONSTRUCTOR_ELT(CONSTRUCTOR_ELTS(ctor), idx, field, value) {
        struct cl_accessor *ac_first = ac;
        struct cl_accessor **ac_last;
        dup_ac_chain(&ac_first, &ac_last);
        *ac_last = CL_ZNEW(struct cl_accessor);
        (*ac_last)->type = (struct cl_type *) clt;

        if (isArray) {
            // initialize an item of an array
            (*ac_last)->code = CL_ACCESSOR_DEREF_ARRAY;

            struct cl_operand *op_idx = CL_ZNEW(struct cl_operand);
            op_idx->code = CL_OPERAND_CST;
            op_idx->data.cst.code = CL_TYPE_INT;
            op_idx->data.cst.data.cst_int.value = idx;
            (*ac_last)->data.array.index = op_idx;

            // FIXME: unguarded recursion
            read_initials(var, pinit, value, ac_first);
        }
        else {
            // initialize a field of a composite type
            const int nth = field_lookup(ctor, field);
            CL_BREAK_IF(clt->items[nth].type != add_type_if_needed(field));

            (*ac_last)->code = CL_ACCESSOR_ITEM;
            (*ac_last)->data.item.id = nth;

            read_initials(var, pinit, value, ac_first);
        }

        while (*pinit)
            pinit = &(*pinit)->next;
    }

    // free the cl_accessor prefix
    free_cl_accessor(ac);
}

static struct cl_var* add_var_if_needed(tree t)
{
    // hash table lookup
    const int uid = DECL_UID(t);
    struct cl_var *var = var_db_lookup(var_db, uid);
    if (var)
        // var already hashed
        return var;

    // insert a new var into hash table
    var = CL_ZNEW(struct cl_var);
    var->uid = uid;
    var_db_insert(var_db, var);

    // read meta-data
    read_gcc_location(&var->loc, DECL_SOURCE_LOCATION(t));
    var->artificial = DECL_ARTIFICIAL(t);
    var->is_extern = DECL_EXTERNAL(t);
    if (!var->is_extern) {
        const enum cl_scope_e code = get_decl_scope(t);
        switch (code) {
            case CL_SCOPE_GLOBAL:
            case CL_SCOPE_STATIC:
                // emit non-extern global variables as initialized
                var->initialized = true;

            default:
                break;
        }
    }

    // read name and initializer
    var->name = get_decl_name(t);
    if (VAR_DECL == TREE_CODE(t)) {
        tree ctor = DECL_INITIAL(t);
        if (ctor)
            read_initials(var, &var->initial, ctor, /* ac */ 0);
    }

    return var;
}

static void read_cst_fnc(struct cl_operand *op, tree t)
{
    op->code                            = CL_OPERAND_CST;
    op->scope                           = get_decl_scope(t);
    op->data.cst.code                   = CL_TYPE_FNC;
    op->data.cst.data.cst_fnc.name      = get_decl_name(t);
    op->data.cst.data.cst_fnc.is_extern = DECL_EXTERNAL(t);
    op->data.cst.data.cst_fnc.uid       = DECL_UID(t);
    read_gcc_location(&op->data.cst.data.cst_fnc.loc, DECL_SOURCE_LOCATION(t));
}

static void read_cst_int(struct cl_operand *op, tree t)
{
    // I don't understand the following code, see gcc/print-tree.c
    CL_BREAK_IF(TREE_INT_CST_HIGH(t) != 0 && (TREE_INT_CST_LOW(t) == 0
                || TREE_INT_CST_HIGH(t) != -1));

    // FIXME: should we read unsigned types separately?
    op->code                            = CL_OPERAND_CST;
    op->data.cst.code                   = CL_TYPE_INT;
    op->data.cst.data.cst_int.value     = TREE_INT_CST_LOW(t);
}

static void read_cst_real(struct cl_operand *op, tree t)
{
    // TODO: make this a compile-time assertion
    if (8 != sizeof(double))
        CL_BREAK_IF("unsupported sizeof(double) detected in read_cst_int()");

    // the float encoder in gcc writes floats to arrays of longs
    union {
        double              d;
        long                l[2];
        unsigned long       ul[2];
    } u;

    // convert gcc's internal representation of real to build-arch native format
    real_to_target_fmt(u.l, TREE_REAL_CST_PTR(t), &ieee_double_format);

    // compile-time switch
    switch (sizeof(long)) {
        case 4:
            // already encoded as build-arch native format
            break;

        case 8:
            // 4 bytes of each long are valid --> we need to pack the chunks
            u.ul[0] &= (1UL << 32) - 1UL;
            u.ul[0] |= (u.ul[1] << 32);
            break;

        default:
            // TODO: make this a compile-time assertion
            CL_BREAK_IF("build architecture not supported by read_cst_int()");
    }

    op->code                            = CL_OPERAND_CST;
    op->data.cst.code                   = CL_TYPE_REAL;
    op->data.cst.data.cst_real.value    = u.d;
}

static void read_raw_operand(struct cl_operand *op, tree t)
{
    op->code = CL_OPERAND_VOID;

    enum tree_code code = TREE_CODE(t);
    switch (code) {
        case VAR_DECL:
        case PARM_DECL:
        case RESULT_DECL:
            op->code                            = CL_OPERAND_VAR;
            op->scope                           = get_decl_scope(t);
            op->data.var                        = add_var_if_needed(t);
            break;

        case STRING_CST:
            op->code                            = CL_OPERAND_CST;
            op->data.cst.code                   = CL_TYPE_STRING;
            op->data.cst.data.cst_string.value  = TREE_STRING_POINTER(t);
            break;

        case FUNCTION_DECL:
            read_cst_fnc(op, t);
            break;

        case INTEGER_CST:
            read_cst_int(op, t);
            break;

        case REAL_CST:
            read_cst_real(op, t);
            break;

        default:
            CL_BREAK_IF("read_raw_operand() got something special");
    }
}

static void chain_accessor(struct cl_accessor **ac, enum cl_accessor_e code)
{
    // create and initialize new item
    struct cl_accessor *ac_new = CL_ZNEW(struct cl_accessor);
    CL_ASSERT(ac_new);
    ac_new->code = code;

    // append new item to chain
    ac_new->next = *ac;
    *ac = ac_new;
}

static struct cl_type* operand_type_lookup(tree t)
{
    CL_BREAK_IF(NULL_TREE == t);
    tree op0 = TREE_OPERAND(t, 0);

    CL_BREAK_IF(NULL_TREE == op0);
    return add_type_if_needed(op0);
}

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
    CL_BREAK_IF(NULL_TREE == op1);

    struct cl_operand *index = CL_ZNEW(struct cl_operand);
    CL_ASSERT(index);

    // possible recursion
    handle_operand(index, op1);
    (*ac)->data.array.index = index;
}

static void handle_accessor_indirect_ref(struct cl_accessor **ac, tree *pt)
{
    tree t = *pt;
    tree op0 = TREE_OPERAND(t, 0);

    if (ADDR_EXPR == TREE_CODE(op0)) {
        // optimize out the sequence of REF/DEREF that would confuse Predator
        *pt = op0;
        return;
    }

    chain_accessor(ac, CL_ACCESSOR_DEREF);
    (*ac)->type = add_type_if_needed(op0);
}

#ifdef MEM_REF_CHECK
static void handle_accessor_offset(struct cl_accessor **ac, tree t)
{
    const int off = TREE_INT_CST_LOW(TREE_OPERAND(t, 1));
    if (!off)
        return;

    tree op0 = TREE_OPERAND(t, 0);
    if (ADDR_EXPR == TREE_CODE(op0))
        t = op0;

    chain_accessor(ac, CL_ACCESSOR_OFFSET);
    (*ac)->type            = operand_type_lookup(t);
    (*ac)->data.offset.off = off;
}
#endif

static void handle_accessor_component_ref(struct cl_accessor **ac, tree t)
{
    tree op    = TREE_OPERAND(t, 0);
    tree field = TREE_OPERAND(t, 1);

    chain_accessor(ac, CL_ACCESSOR_ITEM);
    (*ac)->type         = operand_type_lookup(t);
    (*ac)->data.item.id = field_lookup(op, field);
}

static void handle_accessor_bitfield(struct cl_accessor **ac, tree t)
{
    chain_accessor(ac, CL_ACCESSOR_ITEM);
    (*ac)->type         = operand_type_lookup(t);
    (*ac)->data.item.id = bitfield_lookup(t);
}

static bool handle_accessor(struct cl_accessor **ac, tree *pt)
{
    tree t = *pt;
    CL_BREAK_IF(NULL_TREE == t);

    enum tree_code code = TREE_CODE(t);
    switch (code) {
        case ADDR_EXPR:
            handle_accessor_addr_expr(ac, t);
            break;

        case ARRAY_REF:
            handle_accessor_array_ref(ac, t);
            break;

        // MEM_REF appeared after 4.5.0
#ifdef MEM_REF_CHECK
        case MEM_REF:
            handle_accessor_offset(ac, t);
            // fall through!
#endif
        case INDIRECT_REF:
            handle_accessor_indirect_ref(ac, &t);
            break;

        case COMPONENT_REF:
            handle_accessor_component_ref(ac, t);
            break;

        case BIT_FIELD_REF:
            handle_accessor_bitfield(ac, t);
            break;

        default:
            return false;
    }

    *pt = TREE_OPERAND(t, 0);
    return true;
}

static void free_cl_operand_data(struct cl_operand *op)
{
    // free the chain of accessors
    const struct cl_accessor *ac = op->accessor;
    while (ac) {
        const struct cl_accessor *next = ac->next;

        if (CL_ACCESSOR_DEREF_ARRAY == ac->code)
            // FIXME: unguarded recursion
            free_cl_operand_data(ac->data.array.index);

        free((void *) ac);
        ac = next;
    }
}

static void handle_operand(struct cl_operand *op, tree t)
{
    memset(op, 0, sizeof *op);

    if (!t)
        return;

    // read type
    op->type = add_type_if_needed(t);

    // read accessor
    while (handle_accessor(&op->accessor, &t))
        ;

    CL_BREAK_IF(NULL_TREE == t);
    read_raw_operand(op, t);
}

struct gimple_walk_data {
    bool abort_sent;
};

static void handle_stmt_unop(gimple stmt, enum tree_code code,
                             struct cl_operand *dst, tree src_tree)
{
    if (CONSTRUCTOR == TREE_CODE(src_tree)) {
        CL_BREAK_IF(dst->code != CL_OPERAND_VAR);
        struct cl_var *var = dst->data.var;
        if (!var->initial)
            read_initials(var, &var->initial, src_tree, /* ac */ 0);

        return;
    }

    struct cl_operand src;
    handle_operand(&src, src_tree);

    struct cl_insn cli;
    cli.code                    = CL_INSN_UNOP;
    cli.data.insn_unop.code     = CL_UNOP_ASSIGN;
    cli.data.insn_unop.dst      = dst;
    cli.data.insn_unop.src      = &src;
    read_gimple_location(&cli.loc, stmt);

    // TODO: check validity/usefulness of the following condition
    if (code != TREE_CODE(src_tree)
            && !translate_unop_code(&cli.data.insn_unop.code, code))
    {
        CL_ERROR("unhandled unary operator");
        CL_BREAK_IF("please implement");
    }

    if (CL_INSN_NOP != cli.code)
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

    if (!translate_binop_code(&cli.data.insn_binop.code, code)) {
        CL_ERROR("unhandled binary operator");
        CL_BREAK_IF("please implement");
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
            CL_BREAK_IF("unhandled operator");
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

static void handle_stmt_call(gimple stmt, struct gimple_walk_data *data)
{
    tree op0 = gimple_call_fn(stmt);

    if (ADDR_EXPR == TREE_CODE(op0))
        // automagic dereference
        op0 = TREE_OPERAND(op0, 0);

    CL_BREAK_IF(NULL_TREE == op0);

    // lhs
    struct cl_operand dst;
    handle_operand(&dst, gimple_call_lhs(stmt));

    // fnc is also operand (call through pointer, struct member, etc.)
    struct cl_operand fnc;
    handle_operand(&fnc, op0);

    // emit CALL insn
    struct cl_loc loc;
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
        data->abort_sent = true;
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
    C99_FIELD(uid        ) /* FIXME */ -1,
    C99_FIELD(code       ) CL_TYPE_BOOL,
    C99_FIELD(loc        ) /* cl_loc_unknown */ { NULL, 0, 0, false },
    C99_FIELD(scope      ) CL_SCOPE_GLOBAL,
    C99_FIELD(name       ) "<builtin_bool>",
    C99_FIELD(size       ) /* FIXME */ sizeof(bool),
    C99_FIELD(item_cnt   ) 0,
    C99_FIELD(items      ) NULL,
    C99_FIELD(array_size ) 0,
    C99_FIELD(is_unsigned) false
};

static void handle_stmt_cond_br(gimple stmt, const char *then_label,
                                const char *else_label)
{
    static int last_reg_uid = /* XXX */ 0x100000;

    struct cl_var *var = CL_ZNEW(struct cl_var);
    var->uid = ++last_reg_uid;
    var_db_insert(var_db, var);

    struct cl_operand dst;
    memset(&dst, 0, sizeof dst);

    dst.code            = CL_OPERAND_VAR;
    dst.scope           = CL_SCOPE_FUNCTION;
    dst.type            = &builtin_bool_type;
    dst.data.var        = var;

    handle_stmt_binop(stmt,
            gimple_cond_code(stmt),
            &dst,
            gimple_cond_lhs(stmt),
            gimple_cond_rhs(stmt));

    struct cl_insn cli;
    cli.code                        = CL_INSN_COND;
    cli.data.insn_cond.src          = &dst;
    cli.data.insn_cond.then_label   = then_label;
    cli.data.insn_cond.else_label   = else_label;

    read_gimple_location(&cli.loc, stmt);
    cl->insn(cl, &cli);
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

    CL_BREAK_IF(!label_true || !label_false);

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
        gimple_seq gs = bb_seq(e->dest);
        const gimple_stmt_iterator stmt_it = gsi_start(gs);

        // check whether first statement in BB is GIMPLE_LABEL
        const gimple bb_stmt = gsi_stmt(stmt_it);
        if (!bb_stmt || GIMPLE_LABEL != bb_stmt->gsbase.code)
            continue;

        // get label declaration
        tree label = gimple_label_label(bb_stmt);
        CL_BREAK_IF(!label);

        if (label_decl_uid == LABEL_DECL_UID(label))
            // match
            return bb->index;
    }

    CL_BREAK_IF("find_case_label_target() has failed");
    // no matching GIMPLE_LABEL was found in BB successors
    // this should never happen
    return (unsigned) -1;
}

static void handle_stmt_switch(gimple stmt)
{
    struct cl_operand src;
    handle_operand(&src, gimple_switch_index(stmt));

    // emit insn_switch_open
    struct cl_loc loc;
    read_gimple_location(&loc, stmt);
    cl->insn_switch_open(cl, &loc, &src);
    free_cl_operand_data(&src);

    unsigned i;
    for (i = 0; i < gimple_switch_num_labels(stmt); ++i) {
        tree case_decl = gimple_switch_label(stmt, i);
        CL_BREAK_IF(!case_decl);

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
        CL_BREAK_IF(!case_label || LABEL_DECL != TREE_CODE(case_label));

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

static void handle_stmt_label(gimple stmt)
{
    tree label = gimple_label_label(stmt);
    struct cl_insn cli;
    cli.code = CL_INSN_LABEL;
    cli.data.insn_label.name = get_decl_name(label);
    read_gcc_location(&cli.loc, DECL_SOURCE_LOCATION(label));
    cl->insn(cl, &cli);
}

static void handle_stmt_resx(gimple stmt, struct gimple_walk_data *data)
{
    CL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_RESX");
    if (data->abort_sent)
        return;

    static struct cl_insn cli;
    cli.code    = CL_INSN_ABORT;
    cl->insn(cl, &cli);
    data->abort_sent = true;
}

// callback of walk_gimple_seq declared in <gimple.h>
static tree cb_walk_gimple_stmt(gimple_stmt_iterator *iter,
                                bool *subtree_done,
                                struct walk_stmt_info *info)
{
    gimple stmt = gsi_stmt(*iter);
    (void) subtree_done;

    struct gimple_walk_data *data = (struct gimple_walk_data *) info->info;

#if CL_DEBUG_GCC_GIMPLE
    printf("\n\t\t");
    print_gimple_stmt(stdout, stmt, /* indentation */ 0, TDF_LINENO);
#endif

    enum gimple_code code = stmt->gsbase.code;
    switch (code) {
        case GIMPLE_COND:
            handle_stmt_cond(stmt);
            break;

        case GIMPLE_ASSIGN:
            handle_stmt_assign(stmt);
            break;

        case GIMPLE_CALL:
            handle_stmt_call(stmt, data);
            break;

        case GIMPLE_RETURN:
            handle_stmt_return(stmt);
            break;

        case GIMPLE_SWITCH:
            handle_stmt_switch(stmt);
            break;

        case GIMPLE_LABEL:
            handle_stmt_label(stmt);
            break;

        case GIMPLE_RESX:
            handle_stmt_resx(stmt, data);
            break;

        case GIMPLE_ASM:
            CL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_ASM");
            break;

        case GIMPLE_NOP:
            CL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_NOP");
            break;

        case GIMPLE_PREDICT:
            // Although I am too lazy to find out what these nodes are useful
            // for, I have been observing the following warnings for years
            // without noticing any negative impact of them on the analysis.
#if 0
            CL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_PREDICT");
#endif
            break;

        default:
            CL_BREAK_IF("cb_walk_gimple_stmt() got something special");
    }

#if CL_DEBUG_GCC_GIMPLE
    printf("\n");
#endif

    return NULL;
}

// walk through gimple BODY using <gimple.h> API
static void handle_bb_gimple(struct basic_block_def *bb, void *data)
{
    const gimple_seq seq = bb_seq(bb);

    struct walk_stmt_info info;
    memset(&info, 0, sizeof(info));
    info.info = data;

    walk_gimple_seq(seq, cb_walk_gimple_stmt, NULL, &info);
}

static bool dig_edge_location(struct cl_loc *loc, const edge e)
{
    const gimple_seq src_seq = bb_seq(e->src);
    if (src_seq) {
        // use the last statement of the source bb
        const gimple last_stmt = gimple_seq_last_stmt(src_seq);
        if (last_stmt && read_gimple_location(loc, last_stmt))
            return true;
    }

    gimple_seq dst_seq = bb_seq(e->dest);
    if (dst_seq) {
        // use the 1st statement of the destination bb with valid location info
        gimple_stmt_iterator iter = gsi_start(dst_seq);
        for (; !gsi_end_p(iter); gsi_next(&iter))
            if (read_gimple_location(loc, gsi_stmt(iter)))
                return true;
    }

    if (e->flags & EDGE_FALLTHRU)
        // fallthru edges have no location info if the successor is empty
        return false;

    CL_BREAK_IF("dig_edge_location() failed to read any location");
    return false;
}

static void handle_jmp_edge(edge e)
{
    char *label = index_to_label(e->dest->index);

    struct cl_insn cli;
    cli.code                = CL_INSN_JMP;
    cli.data.insn_jmp.label = label;
    cli.loc.file            = NULL;
    cli.loc.line            = -1;

    dig_edge_location(&cli.loc, e);

    cl->insn(cl, &cli);
    free(label);
}

static void handle_fnc_bb(struct basic_block_def *bb)
{
    // declare bb
    char *label = index_to_label(bb->index);
    cl->bb_open(cl, label);
    free(label);

    // go through the bb's content
    struct gimple_walk_data data = { false };
    handle_bb_gimple(bb, &data);

    // check for a fallthru successor
    edge e;
    edge_iterator ei = ei_start(bb->succs);
    if (ei_cond(ei, &e)) {
        if (e->dest && (e->flags & EDGE_FALLTHRU))
            handle_jmp_edge(e);

        if (e->flags & EDGE_EH) {
            CL_WARN_UNHANDLED("EDGE_EH (exception edge)");
            if (data.abort_sent)
                return;

            static struct cl_insn cli;
            cli.code    = CL_INSN_ABORT;
            cl->insn(cl, &cli);
        }
    }
}

static void handle_fnc_cfg(struct control_flow_graph *cfg)
{
    struct basic_block_def *bb = cfg->x_entry_block_ptr;
    edge e;
    edge_iterator ei = ei_start(bb->succs);
    if (ei_cond(ei, &e) && e->dest) {
        // ENTRY block
        handle_jmp_edge(e);
        bb = bb->next_bb;
    }

    CL_BREAK_IF(!bb);

    while (/* skip EXIT block */ bb->next_bb) {
        handle_fnc_bb(bb);
        bb = bb->next_bb;
    }
}

// go through argument list ARGS of fnc declaration
static void handle_fnc_decl_arglist(tree args)
{
    int argc = 0;

    while (args) {
        struct cl_operand arg_src;
        handle_operand(&arg_src, args);
        arg_src.scope = CL_SCOPE_FUNCTION;

        cl->fnc_arg_decl(cl, ++argc, &arg_src);

        args = TREE_CHAIN(args);
    }
}

// handle FUNCTION_DECL tree node given as DECL
static void handle_fnc_decl(tree decl)
{
    CL_BREAK_IF(NULL_TREE == DECL_NAME(decl));

    // emit fnc declaration
    struct cl_operand fnc;
    handle_operand(&fnc, decl);
    cl->fnc_open(cl, &fnc);

    // emit arg declarations
    tree args = DECL_ARGUMENTS(decl);
    handle_fnc_decl_arglist(args);

    // obtain CFG for current function
    struct function *def = DECL_STRUCT_FUNCTION(decl);
    if (!def || !def->cfg) {
        CL_WARN_UNHANDLED("CFG not found");
        return;
    }

    // go through CFG
    handle_fnc_cfg(def->cfg);

    // fnc traverse complete
    cl->fnc_close(cl);
}

// callback of tree pass declared in <tree-pass.h>
static unsigned int cl_pass_execute(void)
{
    if (error_detected())
        // we're already on the error path
        return 0;

    if (!current_function_decl) {
        CL_WARN_UNHANDLED("NULL == current_function_decl");
        return 0;
    }

    if (FUNCTION_DECL != TREE_CODE(current_function_decl)) {
        CL_WARN_UNHANDLED("TREE_CODE(current_function_decl)");
        return 0;
    }

    handle_fnc_decl(current_function_decl);
    return 0;
}

// will be initialized in cl_regcb()
static struct opt_pass cl_pass;

// definition of a new pass provided by the plug-in
static struct register_pass_info cl_plugin_pass = {
    C99_FIELD(pass                    ) &cl_pass,
    C99_FIELD(reference_pass_name     ) "cfg",
    C99_FIELD(ref_pass_instance_number) 0,
    C99_FIELD(pos_op                  ) PASS_POS_INSERT_AFTER
};

// callback called as last (if the plug-in does not crash before)
static void cb_finish(void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    if (error_detected())
        CL_WARN("some errors already detected, "
                "additional passes will be skipped");
    else
        // this should trigger the code listener analyzer (if any)
        cl->acknowledge(cl);

    // FIXME: suboptimal interface of CL messaging
    if (!preserve_ec) {
        if (cnt_errors) {
            // this causes non-zero exit code of gcc
            error_at(input_location,
                     "%s has detected some errors", plugin_name);
        }
        else if (cnt_warnings) {
            // this causes non-zero exit code of gcc in case of -Werror
            warning_at(input_location, 0,
                       "%s has reported some warnings", plugin_name);
        }
    }

    // final cleanup
    cl->destroy(cl);
    cl_global_cleanup();
    var_db_destroy(var_db);
    type_db_destroy(type_db);
    free_plugin_name();
}

// callback called on start of input file processing
static void cb_start_unit(void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    cl->file_open(cl, input_filename);
}

static void cb_finish_unit(void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    cl->file_close(cl);
}

// register callbacks for plug-in NAME
static void cl_regcb(const char *name) {
    // the structure changes between versions of GCCs, so we do not use initials
    cl_pass.type = GIMPLE_PASS;
    cl_pass.name = "clplug";
    cl_pass.execute = cl_pass_execute;
    cl_pass.properties_required = PROP_cfg | PROP_gimple_any;

    // passing NULL as CALLBACK to register_callback stands for virtual callback

    // register new pass provided by the plug-in
    register_callback(name, PLUGIN_PASS_MANAGER_SETUP,
                      /* callback */   NULL,
                      &cl_plugin_pass);

    register_callback(name, PLUGIN_FINISH_UNIT,
                      cb_finish_unit,
                      /* user_data */  NULL);

    register_callback(name, PLUGIN_FINISH,
                      cb_finish,
                      /* user_data */  NULL);

    register_callback(name, PLUGIN_INFO,
                      /* callback */   NULL,
                      &cl_info);

    register_callback(name, PLUGIN_START_UNIT,
                      cb_start_unit,
                      /* user_data */  NULL);
}

struct cl_plug_options {
    bool                    dump_types;
    bool                    use_dotgen;
    bool                    use_pp;
    bool                    use_analyzer;
    bool                    use_typedot;
    const char              *gl_dot_file;
    const char              *pp_out_file;
    const char              *analyzer_args;
    const char              *type_dot_file;
    const char              *pid_file;
};

static int clplug_init(const struct plugin_name_args *info,
                       struct cl_plug_options *opt)
{
    // initialize opt data
    memset(opt, 0, sizeof(*opt));
    opt->use_analyzer       = true;
    opt->analyzer_args      = "";

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
                : 1;
        }
        else if (STREQ(key, "version")) {
            // do not use info->version yet
            printf("\n%s\n", cl_info.version);
            return EXIT_FAILURE;
        }
        else if (STREQ(key, "help")) {
            // do not use info->help yet
            printf("\n%s\n", cl_info.help);
            return EXIT_FAILURE;
        }
        else if (STREQ(key, "args")) {
            opt->analyzer_args = (value)
                ? value
                : "";
        }
        else if (STREQ(key, "dry-run")) {
            opt->use_analyzer   = false;
            // TODO: warn about ignoring extra value?
        }
        else if (STREQ(key, "dump-pp")) {
            opt->use_pp         = true;
            opt->pp_out_file    = value;
        }
        else if (STREQ(key, "dump-types")) {
            opt->dump_types     = true;
            // TODO: warn about ignoring extra value?
        }
        else if (STREQ(key, "gen-dot")) {
            opt->use_dotgen     = true;
            opt->gl_dot_file    = value;
        }
        else if (STREQ(key, "preserve-ec")) {
            // FIXME: do not use gl variable, use the pointer user_data instead
            preserve_ec = true;
            // TODO: warn about ignoring extra value?
        }
        else if (STREQ(key, "pid-file")) {
            if (value)
                opt->pid_file = value;
            else {
                CL_ERROR("mandatory value omitted for pid-file");
                return EXIT_FAILURE;
            }

        }
        else if (STREQ(key, "type-dot")) {
            if (value) {
                opt->use_typedot    = true;
                opt->type_dot_file  = value;
            }
            else {
                CL_ERROR("mandatory value omitted for type-dot");
                return EXIT_FAILURE;
            }
        }
        else {
            CL_ERROR("unhandled plug-in argument: %s", key);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static bool cl_append_listener(struct cl_code_listener *chain,
                               const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *config_string;
    int rv = vasprintf(&config_string, fmt, ap);
    CL_ASSERT(0 < rv);
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

static bool cl_append_def_listener(struct cl_code_listener *chain,
                                   const char *listener, const char *args,
                                   const struct cl_plug_options *opt)
{
    const char *clf = (opt->use_analyzer)
        ? "unfold_switch,unify_labels_gl"
        : "unify_labels_fnc";

    return cl_append_listener(chain,
            "listener=\"%s\" listener_args=\"%s\" clf=\"%s\"",
            listener, args, clf);
}

static struct cl_code_listener*
create_cl_chain(const struct cl_plug_options *opt)
{
    struct cl_code_listener *chain = cl_chain_create();
    if (!chain)
        // error message already emitted
        return NULL;

#if CL_DEBUG_LOCATION
    if (!cl_append_listener(chain, "listener=\"locator\""))
        return NULL;
#endif

    if (opt->use_pp) {
        const char *use_listener = (opt->dump_types)
            ? "pp_with_types"
            : "pp";

        const char *out = (opt->pp_out_file)
            ? opt->pp_out_file
            : "";

        if (!cl_append_def_listener(chain, use_listener, out, opt))
            return NULL;
    }

    if (opt->use_dotgen) {
        const char *gl_dot = (opt->gl_dot_file)
            ? opt->gl_dot_file
            : "";
        if (!cl_append_def_listener(chain, "dotgen", gl_dot, opt))
            return NULL;
    }

    if (opt->use_typedot && !cl_append_def_listener(chain, "typedot",
                opt->type_dot_file, opt))
        return NULL;

    if (opt->use_analyzer
            && !cl_append_def_listener(chain, "easy", opt->analyzer_args, opt))
        return NULL;

    return chain;
}

static bool write_pid_file(const char *pid_file)
{
    if (!pid_file)
        // no PID file --> OK
        return true;

    FILE *fpid = fopen(pid_file, "w");
    if (!fpid) {
        fprintf(stderr, "failed to open '%s' for writing\n", pid_file);
        return false;
    }

    // write the PID
    fprintf(fpid, "%d\n", getpid());

    if (fclose(fpid)) {
        fprintf(stderr, "failed to write to '%s'\n", pid_file);
        return false;
    }

    // all OK
    return true;
}

// plug-in initialization according to gcc plug-in API
__attribute__ ((__visibility__ ("default")))
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    struct cl_plug_options opt;

    // global initialization
    init_plugin_name(plugin_info);
    int rv = clplug_init(plugin_info, &opt);
    if (rv)
        return rv;

    if (!write_pid_file(opt.pid_file))
        // error already printed out
        return 1;

    // print something like "hello world!"
    CL_DEBUG("initializing code listener [SHA1 %s]", CL_GIT_SHA1);
    CL_DEBUG("plug-in is compiled against gcc %s%s%s, built at %s, conf: %s",
           gcc_version.basever, gcc_version.devphase, gcc_version.revision,
           gcc_version.datestamp, gcc_version.configuration_arguments);
    CL_DEBUG("now going to be loaded into gcc %s%s%s, built at %s, conf: %s",
           version->basever, version->devphase, version->revision,
           version->datestamp, version->configuration_arguments);

    // check for compatibility with host gcc's version
    if (!plugin_default_version_check(version, &gcc_version)) {
        CL_ERROR("host gcc's version/build mismatch"
                 ", call-backs not registered!");

        return 1;
    }

    // initialize code listener
    static struct cl_init_data init = {
        C99_FIELD(debug      ) dummy_printer,
        C99_FIELD(warn       ) cl_warn,
        C99_FIELD(error      ) cl_error,
        C99_FIELD(note       ) trivial_printer,
        C99_FIELD(die        ) trivial_printer,
        C99_FIELD(debug_level) 0
    };

    if ((init.debug_level = verbose))
        init.debug = trivial_printer;

    cl_global_init(&init);
    cl = create_cl_chain(&opt);
    CL_ASSERT(cl);

    // initialize type database and var database
    type_db = type_db_create();
    var_db = var_db_create();
    CL_ASSERT(type_db && var_db);

    // register callbacks (and virtual callbacks)
    cl_regcb(plugin_info->base_name);
    CL_DEBUG("plug-in successfully initialized");

    return 0;
}
