#include <gcc-plugin.h>

// auxiliary gcc headers
#define IN_GCC
#include <coretypes.h> // needed by tree-pass.h, gcc bug?
#include <tm.h>
#include <function.h>
#include <input.h>
#include <tree-pass.h>

#define SEP_LOG(...) do { \
    fprintf (stderr, "--- %s: ", plugin_name); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

#define SEP_LOG_FNC \
    SEP_LOG ("%s", __FUNCTION__)

int plugin_is_GPL_compatible;

static const char *plugin_name = "[uninitialized]";

static struct plugin_info info = {
    .version = "gcc plug-in separate 0.1 experimental",
    .help = "print output program direct to stdout; verbose output to stderr",
};

unsigned int sep_pass_execute (void);

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

unsigned int sep_pass_execute (void)
{
    fprintf (stderr, "--- %s:", plugin_name);
    print_node_brief (stderr, "", cfun->decl, 0);
    fprintf (stderr, "\n");

    return 0;
}

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

    SEP_LOG ("processing '%s'", main_input_filename);
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
