// this include has to be the first (according the gcc plug-in API)
#include <gcc-plugin.h>

#include "code_listener.h"

#include <coretypes.h>
#include <diagnostic.h>

// this include has to be before <function.h>; otherwise it will NOT compile
#include <tm.h>

#include <function.h>
#include <gimple.h>
#include <input.h>
#include <tree-pass.h>

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

// safe to remove (it's here for debugging purposes only)
#include <signal.h>
#define TRAP raise(SIGTRAP)

#ifndef STREQ
#   define STREQ(s1, s2) (0 == strcmp(s1, s2))
#endif

// TODO: replace with gcc native debugging infrastructure
#define SL_LOG(...) do { \
    if (SL_VERBOSE_PLUG & verbose) { \
        fprintf (stderr, "%s:%d: %s: ", __FILE__, __LINE__, plugin_name); \
        fprintf (stderr, __VA_ARGS__); \
        fprintf (stderr, "\n"); \
    } \
} while (0)

// TODO: replace with gcc native debugging infrastructure
#define SL_LOG_FNC \
    SL_LOG ("%s", __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED(what) \
    fprintf(stderr, "%s:%d: warning: '%s' not handled\n", \
            __FILE__, __LINE__, (what))

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED_GIMPLE(stmt, what) \
    fprintf(stderr, "%s:%d:%d: warning: '%s' not handled\n" \
            "%s:%d: note: raised from '%s'\n", \
            expand_location((stmt)->gsbase.location).file, \
            expand_location((stmt)->gsbase.location).line, \
            expand_location((stmt)->gsbase.location).column, \
            (what), __FILE__, __LINE__, __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED_EXPR(expr, what) \
    fprintf(stderr, "%s:%d:%d: warning: '%s' not handled\n" \
            "%s:%d: note: raised from '%s'\n", \
            expand_location(EXPR_LOCATION(expr)).file, \
            expand_location(EXPR_LOCATION(expr)).line, \
            expand_location(EXPR_LOCATION(expr)).column, \
            (what), __FILE__, __LINE__, __FUNCTION__)

// this should be using gcc's fancy_abort(), but it was actually not tested
#define SL_ASSERT(expr) \
    if (!(expr)) abort()

// required by gcc plug-in API
int plugin_is_GPL_compatible;

// name of the plug-in given by gcc during initialization
static const char *plugin_name = "[uninitialized]";

// verbose bitmask
static int verbose = 0;
#define SL_VERBOSE_PLUG             (1 << 0)
#define SL_VERBOSE_LOCATION         (1 << 1)
#define SL_VERBOSE_GIMPLE           (1 << 2)

// plug-in meta-data according to gcc plug-in API
static struct plugin_info sl_info = {
    .version = "\nslplug 0.1 [experimental]\n",
    .help =    "\nslplug 0.1 [experimental]\n"
        "\n"
        "Usage: gcc -fplugin=slplug.so [OPTIONS] ...\n"
        "\n"
        "OPTIONS:\n"
        "    -fplugin-arg-slplug-help\n"
        "    -fplugin-arg-slplug-version\n"
        "    -fplugin-arg-slplug-verbose=VERBOSE_BITMASK\n"
        "\n"
        "VERBOSE_BITMASK:\n"
        "    1    debug gcc plug-in API\n"
        "    2    print location info using \"locator\" code listener\n"
        "    4    print each gimple statement before its processing\n"
};

static struct cl_code_listener *cl = NULL;

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
    int rv = asprintf(&label, "%u", idx);
    SL_ASSERT(0 < rv);
    return label;
}

static void decl_to_cl_operand(struct cl_operand *op, tree t)
{
    // FIXME: this condition may be not sufficient in all cases
    if (DECL_NAME(t)) {
        // maybe var
        op->type            = CL_OPERAND_VAR;
        op->data.var.name   = IDENTIFIER_POINTER(DECL_NAME(t));

    } else {
        // maybe reg
        op->type            = CL_OPERAND_REG;
        op->data.reg.id     = DECL_UID(t);
    }

    op->deref = false;
    op->offset = NULL;
}

static void handle_operand_indirect_ref(struct cl_operand *op, tree t)
{
    tree op0 = TREE_OPERAND(t, 0);
    if (!op0)
        // Oops, nothing to dereference...
        TRAP;

    decl_to_cl_operand(op, op0);
    op->deref = true;
}

static void handle_operand_component_ref(struct cl_operand *op, tree t)
{
    tree op0 = TREE_OPERAND(t, 0);
    if (!op0)
        TRAP;

    // FIXME: we should distinguish between '.' and '->'
    if (INDIRECT_REF == TREE_CODE(op0)) {
        op0 = TREE_OPERAND(op0, 0);
        if (!op0)
            TRAP;
    }

    if (COMPONENT_REF == TREE_CODE(op0)) {
        // TODO: implement?
        // this should be analysed in regards to code_listener interface
        SL_WARN_UNHANDLED_EXPR(t, "access to sub-type");
        return;
    }

    // attempt to read base (usually var/reg)
    decl_to_cl_operand(op, op0);

    // attempt to read offset
    tree op1 = TREE_OPERAND(t, 1);
    if (!op1)
        // Oops, offset not available...
        TRAP;

    op->deref           = true;
    op->offset          = IDENTIFIER_POINTER(DECL_NAME(op1));
}

// TODO: simplify, check rare cases; and probably split into more functions
static void handle_operand(struct cl_operand *op, tree t)
{
    op->type = CL_OPERAND_VOID;
    op->deref = false;
    op->offset = NULL;

    if (!t)
        return;

    enum tree_code code = TREE_CODE(t);
    switch (code) {
        case VAR_DECL:
        case PARM_DECL:
        case FUNCTION_DECL:
            decl_to_cl_operand(op, t);
            break;

        case INDIRECT_REF:
            handle_operand_indirect_ref(op, t);
            break;

        case COMPONENT_REF:
            handle_operand_component_ref(op, t);
            break;

        case INTEGER_CST:
            op->type                    = CL_OPERAND_INT;
            op->data.lit_int.value      = TREE_INT_CST_LOW(t);
            break;

        case STRING_CST:
            op->type                    = CL_OPERAND_STRING;
            op->data.lit_string.value   = TREE_STRING_POINTER(t);
            break;

        case ARRAY_REF: {
                tree op0 = TREE_OPERAND(t, 0);
                if (!op0)
                    TRAP;

                switch (TREE_CODE(op0)) {
                    case VAR_DECL:
#if 0
                        SL_WARN_UNHANDLED_EXPR(t, "ARRAY_REF / VAR_DECL");
#endif
                        // go through!

                    case STRING_CST:
                        // Aiee, recursion!
                        handle_operand(op, op0);
                        break;

                    default:
                        SL_WARN_UNHANDLED_EXPR(t, "ARRAY_REF");
                }
            }
            break;

        case ADDR_EXPR: {
                tree op0 = TREE_OPERAND(t, 0);
                if (!op0)
                    TRAP;

                switch (TREE_CODE(op0)) {
                    case ARRAY_REF:
                    case STRING_CST:
                        // Aiee, recursion!
                        handle_operand(op, op0);
                        break;

                    default:
                        SL_WARN_UNHANDLED_EXPR(t, "ADDR_EXPR");
                }
            }
            break;

        case BIT_FIELD_REF:
            SL_WARN_UNHANDLED_EXPR(t, "BIT_FIELD_REF");
            break;

        case CONSTRUCTOR:
            SL_WARN_UNHANDLED_EXPR(t, "CONSTRUCTOR");
            break;

        default:
            TRAP;
            break;
    }
}

static void handle_stmt_unop(gimple stmt, enum tree_code code,
                              struct cl_operand *dst, tree src_tree)
{
    struct cl_operand src;
    handle_operand(&src, src_tree);

    struct cl_insn cli;
    cli.type                    = CL_INSN_UNOP;
    cli.data.insn_unop.dst      = dst;
    cli.data.insn_unop.src      = &src;
    read_gimple_location(&cli.loc, stmt);

    switch (code) {
        case ADDR_EXPR:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "ADDR_EXPR");
            return;

        // TODO: grok various unary operators here
        default:
            // FIXME: it silently ignores any special unary operator!!
            cli.data.insn_unop.type = CL_UNOP_ASSIGN;
            break;
    }

    cl->insn(cl, &cli);
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
    cli.type                    = CL_INSN_BINOP;
    cli.data.insn_binop.dst     = dst;
    cli.data.insn_binop.src1    = &src1;
    cli.data.insn_binop.src2    = &src2;
    read_gimple_location(&cli.loc, stmt);

    switch (code) {
        case EQ_EXPR:       cli.data.insn_binop.type = CL_BINOP_EQ;     break;
        case NE_EXPR:       cli.data.insn_binop.type = CL_BINOP_NE;     break;
        case LT_EXPR:       cli.data.insn_binop.type = CL_BINOP_LT;     break;
        case GT_EXPR:       cli.data.insn_binop.type = CL_BINOP_GT;     break;
        case LE_EXPR:       cli.data.insn_binop.type = CL_BINOP_LE;     break;
        case GE_EXPR:       cli.data.insn_binop.type = CL_BINOP_GE;     break;
        case PLUS_EXPR:     cli.data.insn_binop.type = CL_BINOP_ADD;    break;

        case BIT_AND_EXPR:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "BIT_AND_EXPR");
            return;

        case BIT_IOR_EXPR:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "BIT_IOR_EXPR");
            return;

        case MULT_EXPR:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "MULT_EXPR");
            return;

        case POINTER_PLUS_EXPR:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "POINTER_PLUS_EXPR");
            return;

        default:
            TRAP;
    }

    cl->insn(cl, &cli);
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
            break;
    };
}

static void handle_stmt_call_args(gimple stmt)
{
    const int argc = gimple_call_num_args(stmt);
    int i;
    for (i = 0; i < argc; ++i) {
        struct cl_operand src;
        handle_operand(&src, gimple_call_arg(stmt, i));
        cl->insn_call_arg(cl, i + 1, &src);
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

    // emit args
    handle_stmt_call_args(stmt);
    cl->insn_call_close(cl);

    if (ECF_NORETURN & gimple_call_flags(stmt)) {
        // this call never returns --> end of BB!!

        struct cl_insn cli;
        cli.type    = CL_INSN_ABORT;
        cli.loc     = loc;

        cl->insn(cl, &cli);
    }
}

static void handle_stmt_return(gimple stmt)
{
    struct cl_operand src;
    handle_operand(&src, gimple_return_retval(stmt));

    struct cl_insn cli;
    cli.type                    = CL_INSN_RET;
    cli.data.insn_ret.src       = &src;

    // FIXME: location info is valid only for IMPLICIT 'return' statement
    // It's really strange because it does not work properly even
    // with print_gimple_stmt()
    read_gimple_location(&cli.loc, stmt);
    cl->insn(cl, &cli);
}

static const struct cl_operand stmt_cond_fixed_reg = {
    .type           = CL_OPERAND_REG,
    .deref          = false,
    .offset         = NULL,
    .data = {
        .reg = {
            .id     = 0
        }
    }
};

static void handle_stmt_cond_br(gimple stmt, const char *then_label,
                                const char *else_label)
{
    struct cl_insn cli;
    cli.type                        = CL_INSN_COND;
    cli.data.insn_cond.src          = &stmt_cond_fixed_reg;
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
        if (e->flags & /* true */ 1024) {
            struct basic_block_def *next = e->dest;
            label_true = index_to_label(next->index);
        }
        if (e->flags & /* false */ 2048) {
            struct basic_block_def *next = e->dest;
            label_false = index_to_label(next->index);
        }
    }

    if (!label_true && !label_false)
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

        case GIMPLE_LABEL:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_LABEL");
            break;

        case GIMPLE_SWITCH:
            SL_WARN_UNHANDLED_GIMPLE(stmt, "GIMPLE_SWITCH");
            break;

        default:
            TRAP;
            break;
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
    cli.type                = CL_INSN_JMP;
    cli.data.insn_jmp.label = index_to_label(next->index);

    // no location for CL_INSN_JMP
    cl_set_location(&cli.loc, -1);

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
    if (ei_cond(ei, &e) && e->dest && (e->flags & /* fallthru */ 1)) {
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
        tree ident = DECL_NAME(args);
        cl->fnc_arg_decl(cl, ++argc, IDENTIFIER_POINTER(ident));

        args = TREE_CHAIN (args);
    }
}

// handle FUNCTION_DECL tree node given as DECL
static void handle_fnc_decl (tree decl)
{
    // obtain fnc location
    struct cl_location loc;
    read_gcc_location(&loc, DECL_SOURCE_LOCATION(decl));

    // emit fnc declaration
    tree ident = DECL_NAME (decl);
    cl->fnc_open(cl, &loc, IDENTIFIER_POINTER(ident),
            TREE_PUBLIC(decl)
                ? CL_SCOPE_GLOBAL
                : CL_SCOPE_STATIC);

    // emit arg declartaions
    tree args = DECL_ARGUMENTS (decl);
    handle_fnc_decl_arglist (args);

    // obtain CFG for current function
    struct function *fnc = DECL_STRUCT_FUNCTION (decl);
    if (NULL == fnc) {
        SL_WARN_UNHANDLED ("NULL == fnc");
        return;
    }
    struct control_flow_graph *cfg = fnc->cfg;
    if (NULL == cfg) {
        SL_WARN_UNHANDLED ("CFG not found");
        return;
    }

    // go through CFG
    handle_fnc_cfg(cfg);

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
static struct plugin_pass sl_plugin_pass = {
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

    cl->destroy(cl);
    cl_global_cleanup();
}

// callback called on start of input file processing
static void cb_start_unit (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SL_LOG ("processing input file '%s'", main_input_filename);
    cl->file_open(cl, main_input_filename);
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
                       /* callback */ NULL,
                       &sl_plugin_pass);

    register_callback (name, PLUGIN_FINISH_UNIT,
                       cb_finish_unit,
                       /* user_data */ NULL);

    register_callback (name, PLUGIN_FINISH,
                       cb_finish,
                       /* user_data */ NULL);

    register_callback (name, PLUGIN_INFO,
                       /* callback */ NULL,
                       &sl_info);

    register_callback (name, PLUGIN_START_UNIT,
                       cb_start_unit,
                       /* user_data */ NULL);
}

static int slplug_init(const struct plugin_name_args *info)
{
    // initialize global plug-in name
    plugin_name = info->full_name;

    // go through parameter list
    const int argc                      = info->argc;
    const struct plugin_argument *argv  = info->argv;

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
            puts(sl_info.version);
            return EXIT_FAILURE;

        } else if (STREQ(key, "help")) {
            // do not use info->help yet
            puts(sl_info.help);
            return EXIT_FAILURE;

        } else {
            SL_WARN_UNHANDLED(key);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

// FIXME: copy-pasted from slsparse.c
static struct cl_code_listener* create_cl_chain(void)
{
    struct cl_code_listener *cl;
    struct cl_code_listener *chain = cl_chain_create();
    if (!chain)
        // error message already emitted
        return NULL;

    if (SL_VERBOSE_LOCATION & verbose) {
        cl = cl_code_listener_create("locator", STDOUT_FILENO, false);
        if (!cl) {
            chain->destroy(chain);
            return NULL;
        }
        cl_chain_append(chain, cl);
    }

    cl = cl_code_listener_create("pp", STDOUT_FILENO, false);
    if (!cl) {
        chain->destroy(chain);
        return NULL;
    }
    cl_chain_append(chain, cl);

    return chain;
}

// plug-in initialization according to gcc plug-in API
int plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version)
{
    // global initialization
    int rv = slplug_init(plugin_info);
    if (rv)
        return rv;

    // print something like "hello world!"
    SL_LOG_FNC;
    SL_LOG ("using gcc %s %s, built at %s", version->basever,
             version->devphase, version->datestamp);

    // initialize code listener
    cl_global_init_defaults(NULL, true);
    cl = create_cl_chain();
    SL_ASSERT(cl);

    // try to register callbacks (and virtual callbacks)
    sl_regcb (plugin_info->base_name);
    SL_LOG ("'%s' successfully initialized", plugin_info->version);

    return 0;
}
