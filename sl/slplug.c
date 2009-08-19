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

// TODO: replace with gcc native debugging infrastructure
#define SL_LOG(...) do { \
    fprintf (stderr, "%s:%d: %s: ", __FILE__, __LINE__, plugin_name); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

// TODO: replace with gcc native debugging infrastructure
#define SL_LOG_FNC \
    SL_LOG ("%s", __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SL_WARN_UNHANDLED(what) \
    fprintf(stderr, "%s:%d: warning: '%s' not handled\n", \
            __FILE__, __LINE__, (what))

// this should be using gcc's fancy_abort(), but it was actually not tested
#define SL_ASSERT(expr) \
    if (!(expr)) abort()

// required by gcc plug-in API
int plugin_is_GPL_compatible;

// name of the plug-in given by gcc during initialization
static const char *plugin_name = "[uninitialized]";

// plug-in meta-data according to gcc plug-in API
static struct plugin_info sl_info = {
    .version = "slplug 0.1 [experimental]",
    .help = "[not implemented]",
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

static void decl_to_cl_operand(struct cl_operand *op, tree t)
{
    if (DECL_NAME(t)) {
        op->type            = CL_OPERAND_VAR;
        op->data.var.name   = IDENTIFIER_POINTER(DECL_NAME(t));
    } else {
        op->type            = CL_OPERAND_REG;
        op->data.reg.id     = DECL_UID(t);
    }

    op->deref = false;
    op->offset = NULL;
}

static void handle_operand(struct cl_operand *op, tree t)
{
    enum tree_code code = TREE_CODE(t);

    op->type = CL_OPERAND_VOID;
    op->deref = false;
    op->offset = NULL;

    switch (code) {
        case VAR_DECL:
        case PARM_DECL:
            decl_to_cl_operand(op, t);
            break;

        case INDIRECT_REF: {
                tree op0 = TREE_OPERAND(t, 0);
                if (!op0)
                    TRAP;

                decl_to_cl_operand(op, op0);
                op->deref = true;
            }
            break;

        case COMPONENT_REF: {
                tree op0 = TREE_OPERAND(t, 0);
                if (!op0)
                    TRAP;

                if (INDIRECT_REF == TREE_CODE(op0)) {
                    op0 = TREE_OPERAND(op0, 0);
                    if (!op0)
                        TRAP;
                }

                tree op1 = TREE_OPERAND(t, 1);
                if (!op1)
                    TRAP;

                if (COMPONENT_REF == TREE_CODE(op0)) {
                    SL_WARN_UNHANDLED("access to sub-type");
                    return;
                }

                decl_to_cl_operand(op, op0);
                op->deref           = true;
                op->offset          = IDENTIFIER_POINTER(DECL_NAME(op1));
            }
            break;

        case INTEGER_CST:
            op->type                    = CL_OPERAND_INT;
            op->data.lit_int.value      = TREE_INT_CST_LOW(t);
            break;

        case ARRAY_REF:
            SL_WARN_UNHANDLED("ARRAY_REF");
            return;

        case ADDR_EXPR:
            SL_WARN_UNHANDLED("ADDR_EXPR");
            return;

        case BIT_FIELD_REF:
            SL_WARN_UNHANDLED("BIT_FIELD_REF");
            return;

        case CONSTRUCTOR:
            SL_WARN_UNHANDLED("CONSTRUCTOR");
            return;

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
            SL_WARN_UNHANDLED("ADDR_EXPR");
            return;

        // TODO: grok various unary operators here
        default:
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
        case EQ_EXPR:
            cli.data.insn_binop.type = CL_BINOP_EQ;
            break;

        case NE_EXPR:
            cli.data.insn_binop.type = CL_BINOP_NE;
            break;

        case LT_EXPR:
            cli.data.insn_binop.type = CL_BINOP_LT;
            break;

        case GT_EXPR:
            cli.data.insn_binop.type = CL_BINOP_GT;
            break;

        case LE_EXPR:
            cli.data.insn_binop.type = CL_BINOP_LE;
            break;

        case GE_EXPR:
            cli.data.insn_binop.type = CL_BINOP_GE;
            break;

        case PLUS_EXPR:
            cli.data.insn_binop.type = CL_BINOP_ADD;
            break;

        case BIT_AND_EXPR:
            SL_WARN_UNHANDLED("BIT_AND_EXPR");
            return;

        case MULT_EXPR:
            SL_WARN_UNHANDLED("MULT_EXPR");
            return;

        case POINTER_PLUS_EXPR:
            SL_WARN_UNHANDLED("POINTER_PLUS_EXPR");
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
            // unary operator
            handle_stmt_unop(stmt, gimple_assign_rhs_code(stmt),
                    &dst,
                    gimple_assign_rhs1(stmt));
            break;

        case 3:
            // binary operator
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

static char* index_to_label (unsigned idx) {
    char *label;
    int rv = asprintf(&label, "%u", idx);
    SL_ASSERT(0 < rv);
    return label;
}

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

    (void) subtree_done;
    (void) info;

    printf("\t\t");
    print_gimple_stmt(stdout, stmt,
                      /* indentation */ 0,
                      TDF_LINENO);

    switch (stmt->gsbase.code) {
        case GIMPLE_COND:
            handle_stmt_cond(stmt);
            break;

        case GIMPLE_ASSIGN:
            handle_stmt_assign(stmt);
            break;

        default:
            //TRAP;
            break;
    }

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

    while (/* FIXME: off by one error */ bb->next_bb) {
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
    tree ident = DECL_NAME (decl);

    struct cl_location loc;
    read_gcc_location(&loc, DECL_SOURCE_LOCATION(decl));

    cl->fnc_open(cl, &loc, IDENTIFIER_POINTER(ident),
            TREE_PUBLIC(decl)
                ? CL_SCOPE_GLOBAL
                : CL_SCOPE_STATIC);

    // print argument list
    tree args = DECL_ARGUMENTS (decl);
    handle_fnc_decl_arglist (args);

    // obtain fnc structure
    struct function *fnc = DECL_STRUCT_FUNCTION (decl);
    if (NULL == fnc) {
        SL_WARN_UNHANDLED ("NULL == fnc");
        return;
    }

    // obtain CFG
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

// FIXME: copy-pasted from slsparse.c
static struct cl_code_listener* create_cl_chain(int verbose)
{
    struct cl_code_listener *cl;
    struct cl_code_listener *chain = cl_chain_create();
    if (!chain)
        // error message already emitted
        return NULL;

    if (1 < verbose) {
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
    // store plug-in name to global variable
    plugin_name = plugin_info->full_name;

#if 1
    // for debugging purposes only
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    // print something like "hello world!"
    SL_LOG_FNC;
    SL_LOG ("using gcc %s %s, built at %s", version->basever,
             version->devphase, version->datestamp);

    // initialize code listener
    cl_global_init_defaults(NULL, true);
    cl = create_cl_chain(/* SPARSE verbose level */ 2);
    SL_ASSERT(cl);

    // try to register callbacks (and virtual callbacks)
    sl_regcb (plugin_info->base_name);
    SL_LOG ("'%s' successfully initialized", plugin_info->version);

    return 0;
}
