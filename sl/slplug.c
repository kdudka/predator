// TODO: replace (almost) all occurrence of gcc_assert with SL_DIE
// to die correctly in production version

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

static char* index_to_label (unsigned idx) {
    char *label;
    int rv = asprintf(&label, "%u", idx);
    gcc_assert(0 < rv);
    return label;
}

static void handle_stmt_cond(gimple stmt)
{
    char *label_true = NULL;
    char *label_false = NULL;
    struct basic_block_def *bb = stmt->gsbase.bb;

    edge e;
    edge_iterator ei;
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

    struct cl_operand op;
    /* TODO */ op.type = CL_OPERAND_VOID;
    cl->insn_cond(cl, /* TODO */ 0, &op, label_true, label_false);
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

    if (stmt->gsbase.code == GIMPLE_COND) {
        GIMPLE_CHECK(stmt, GIMPLE_COND);
        handle_stmt_cond(stmt);
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

static void handle_fnc_bb (struct basic_block_def *bb)
{
    if (bb == cfun->cfg->x_entry_block_ptr) {
        edge e;
        edge_iterator ei = ei_start(bb->succs);
        if (ei_cond(ei, &e) && e->dest) {
            struct basic_block_def *next = e->dest;
            char *label = index_to_label(next->index);
            cl->insn_jmp(cl, /* TODO */ 0, label);
            free(label);
            return;
        }
        TRAP;
    }

    char *label = index_to_label(bb->index);
    cl->bb_open(cl, label);
    free(label);

    struct gimple_bb_info *gimple = bb->il.gimple;
    if (NULL == gimple) {
        SL_WARN_UNHANDLED ("gimple not found");
        TRAP;
        return;
    }
    handle_bb_gimple(gimple->seq);

    edge e;
    edge_iterator ei = ei_start(bb->succs);
    if (ei_cond(ei, &e) && e->dest && (e->flags & /* fallthru */ 1)) {
        struct basic_block_def *next = e->dest;
        char *label = index_to_label(next->index);
        cl->insn_jmp(cl, /* TODO */ 0, label);
        free(label);
        return;
    }
}

static void handle_fnc_cfg (struct control_flow_graph *cfg)
{
    struct basic_block_def *bb = cfg->x_entry_block_ptr;

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
    cl->fnc_open(cl,
            DECL_SOURCE_LINE(decl),
            IDENTIFIER_POINTER(ident),
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
    cl = cl_code_listener_create("pp",
                                 /* TODO: create a file */ STDOUT_FILENO,
                                 false);
    gcc_assert(cl);

    // try to register callbacks (and virtual callbacks)
    sl_regcb (plugin_info->base_name);
    SL_LOG ("'%s' successfully initialized", plugin_info->version);

    return 0;
}
