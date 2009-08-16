// this include has to be the first (according the gcc plug-in API)
#include <gcc-plugin.h>

#include <coretypes.h>
#include <diagnostic.h>

// this include has to be before <function.h>; otherwise it will NOT compile
#include <tm.h>

#include <function.h>
#include <gimple.h>
#include <input.h>
#include <tree-pass.h>

// safe to remove (it's here for debugging purposes only)
#include <signal.h>

// TODO: replace with gcc native debugging infrastructure
#define SEP_LOG(...) do { \
    fprintf (stderr, "--- %s: ", plugin_name); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

// TODO: replace with gcc native debugging infrastructure
#define SEP_LOG_FNC \
    SEP_LOG ("%s", __FUNCTION__)

// TODO: replace with gcc native debugging infrastructure
#define SEP_WARN_UNHANDLED(what) \
    SEP_LOG ("%s () at %s:%d: warning: '%s' not handled", \
             __FUNCTION__, __FILE__, __LINE__, (what))

// required by gcc plug-in API
int plugin_is_GPL_compatible;

// name of the plug-in given by gcc during initialization
static const char *plugin_name = "[uninitialized]";

// plug-in meta-data according to gcc plug-in API
static struct plugin_info sep_info = {
    .version = "gcc plug-in separate 0.1 experimental",
    .help = "print output program directly to stdout; verbose output to stderr",
};

// print name of function argument ARG to stdout
static void handle_fnc_decl_arg (tree arg)
{
    tree ident = DECL_NAME (arg);
    printf ("%s", IDENTIFIER_POINTER (ident));
}

// go through argument list ARGS of fnc declaration
static void handle_fnc_decl_arglist (tree args)
{
    while (args) {
        handle_fnc_decl_arg (args);
        args = TREE_CHAIN (args);
        if (args)
            printf (", ");
    }
}

// callback of walk_gimple_seq declared in <gimple.h>
static tree cb_walk_gimple_stmt (gimple_stmt_iterator *iter,
                                 bool *subtree_done,
                                 struct walk_stmt_info *info)
{
    gimple stmt = gsi_stmt (*iter);

    (void) subtree_done;
    (void) info;

#if 0
    raise (SIGTRAP);
#endif

    if (GIMPLE_LABEL != gimple_code(stmt))
        // only basic indentation for now
        printf("    ");

    print_gimple_stmt(stdout, stmt,
                      /* indentation */ 0,
                      /* flags, e.g. TDF_LINENO */ 0);

    return NULL;
}

// walk through gimple BODY using <gimple.h> API
static void handle_fnc_gimple (gimple_seq body)
{
    struct walk_stmt_info info;
    memset (&info, 0, sizeof(info));
    walk_gimple_seq (body, cb_walk_gimple_stmt, NULL, &info);
}

// handle FUNCTION_DECL tree node given as DECL
static void handle_fnc_decl (tree decl)
{
    // print function name
    tree ident = DECL_NAME (decl);
    printf ("%s (", IDENTIFIER_POINTER (ident));

    // print argument list
    tree args = DECL_ARGUMENTS (decl);
    handle_fnc_decl_arglist (args);
    printf (")\n");

    // obtain fnc structure
    struct function *fnc = DECL_STRUCT_FUNCTION (decl);
    if (NULL == fnc) {
        SEP_WARN_UNHANDLED ("NULL == fnc");
        return;
    }

    // obtain gimple for fnc
    gimple_seq body = fnc->gimple_body;
    if (NULL == body) {
        SEP_WARN_UNHANDLED ("gimple not found");
        return;
    }

    // dump gimple body
    printf ("{\n");
    handle_fnc_gimple (body);
    printf ("}\n\n");
}

// callback of tree pass declared in <tree-pass.h>
static unsigned int sep_pass_execute (void)
{
    if (!current_function_decl) {
        SEP_WARN_UNHANDLED ("NULL == current_function_decl");
        return 0;
    }

    if (FUNCTION_DECL != TREE_CODE (current_function_decl)) {
        SEP_WARN_UNHANDLED ("TREE_CODE (current_function_decl)");
        return 0;
    }

    handle_fnc_decl (current_function_decl);
    return 0;
}

// pass description according to <tree-pass.h> API
static struct opt_pass sep_pass = {
    .type = GIMPLE_PASS,
    .name = "separate",
    .gate = NULL,
    .execute = sep_pass_execute,
    .properties_required = PROP_gimple_any,
    // ...
};

// definition of a new pass provided by the plug-in
static struct plugin_pass sep_plugin_pass = {
    .pass                     = &sep_pass,

    // cfg ... control_flow_graph
    .reference_pass_name      = "cfg",

    .ref_pass_instance_number = 0,
    .pos_op                   = PASS_POS_INSERT_BEFORE,
};

// callback called as last (if the plug-in does not crash before)
static void cb_finish (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SEP_LOG_FNC;
}

// callback called on start of input file processing
static void cb_start_unit (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SEP_LOG ("processing input file '%s'", main_input_filename);
}

// register callbacks for plug-in NAME
static void sep_regcb (const char *name) {
    // passing NULL as CALLBACK to register_callback stands for virtual callback

    // register new pass provided by the plug-in
    register_callback (name, PLUGIN_PASS_MANAGER_SETUP,
                       /* callback */ NULL,
                       &sep_plugin_pass);

    register_callback (name, PLUGIN_FINISH,
                       cb_finish,
                       /* user_data */ NULL);

    register_callback (name, PLUGIN_INFO,
                       /* callback */ NULL,
                       &sep_info);

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
    SEP_LOG_FNC;
    SEP_LOG ("using gcc %s %s, built at %s", version->basever,
             version->devphase, version->datestamp);

    // try to register callbacks (and virtual callbacks)
    sep_regcb (plugin_info->base_name);
    SEP_LOG ("'%s' successfully initialized", plugin_info->version);
    return 0;
}
