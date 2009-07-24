#include <gcc-plugin.h>

// auxiliary gcc headers
#define IN_GCC
#include <coretypes.h> // needed by tree-pass.h, gcc bug?
#include <tm.h>
#include <function.h>
#include <input.h>
#include <langhooks.h>
#include <tree-pass.h>

#include <signal.h>

#define SEP_LOG(...) do { \
    fprintf (stderr, "--- %s: ", plugin_name); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

#define SEP_LOG_FNC \
    SEP_LOG ("%s", __FUNCTION__)

#define SEP_WARN_UNHANDLED(what) \
    SEP_LOG ("%s () at %s:%d: warning: '%s' not handled", \
             __FUNCTION__, __FILE__, __LINE__, (what))

int plugin_is_GPL_compatible;

static const char *plugin_name = "[uninitialized]";

static struct plugin_info info = {
    .version = "gcc plug-in separate 0.1 experimental",
    .help = "print output program directly to stdout; verbose output to stderr",
};

static void handle_single_block (tree block, int level)
{
    fprintf (stderr, "   ");
    for (int i = 0; i < level; ++i)
        fprintf (stderr, "    ");

    print_node_brief (stderr, "", block, 0);
    fprintf (stderr, "\n");
}

static void handle_block (tree block, int level)
{
    if (!block)
        return;

    handle_single_block (block, level);

    tree subs = BLOCK_SUBBLOCKS (block);
    handle_block (subs, level + 1);

    if (!level)
        fprintf (stderr, "\n");
}

static unsigned int sep_pass_execute (void)
{
    SEP_LOG ("processing function '%s'",
             lang_hooks.decl_printable_name (current_function_decl, 2));

    if (FUNCTION_DECL != TREE_CODE (current_function_decl)) {
        SEP_WARN_UNHANDLED ("TREE_CODE (current_function_decl)");
        return 0;
    }

    tree block = DECL_INITIAL (current_function_decl);
    handle_block (block, 0);

#if 0
    raise (SIGTRAP);
#endif

    return 0;
}

static struct opt_pass sep_pass = {
    .type = GIMPLE_PASS,
    .name = "separate",
    .gate = NULL,
    .execute = sep_pass_execute,

    .properties_required = PROP_cfg,
    // TODO
};

static struct plugin_pass sep_plugin_pass = {
    .pass                     = &sep_pass,
    .reference_pass_name      = "cfg",
    .ref_pass_instance_number = 0,
    .pos_op                   = PASS_POS_INSERT_AFTER,
};

static void cb_finish (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SEP_LOG_FNC;
}

static void cb_start_unit (void *gcc_data, void *user_data)
{
    (void) gcc_data;
    (void) user_data;

    SEP_LOG ("processing input file '%s'", main_input_filename);
}

static void sep_regcb (const char *name) {
    register_callback (name, PLUGIN_PASS_MANAGER_SETUP, NULL, &sep_plugin_pass);
    register_callback (name, PLUGIN_FINISH, cb_finish, NULL);
    register_callback (name, PLUGIN_INFO, NULL, &info);
    register_callback (name, PLUGIN_START_UNIT, cb_start_unit, NULL);
}

int plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version)
{
    plugin_name = plugin_info->full_name;

    SEP_LOG_FNC;
    SEP_LOG ("using gcc %s %s, built at %s", version->basever,
             version->devphase, version->datestamp);

    sep_regcb (plugin_info->base_name);

    SEP_LOG ("'%s' successfully initialized", plugin_info->version);
    return 0;
}
