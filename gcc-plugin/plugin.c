#include <gcc-plugin.h>

#define PLUGIN_LOG(...) do { \
    fprintf (stderr, "--- %s: ", plugin_name); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

#define PLUGIN_LOG_FNC \
    PLUGIN_LOG ("%s", __FUNCTION__)

int plugin_is_GPL_compatible;

static const char *plugin_name = "[uninitialized]";

static struct plugin_info info = {
    .version = "plug-in 0.1 experimental",
    .help = "print output program direct to stdout; verbose output to stderr",
};

int plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version)
{
    plugin_name = plugin_info->full_name;

    PLUGIN_LOG_FNC;
    PLUGIN_LOG ("using gcc %s %s, built at %s", version->basever,
                version->devphase, version->datestamp);

    register_callback(plugin_info->base_name, PLUGIN_INFO, NULL, &info);

    PLUGIN_LOG ("%s successfully initialized", plugin_info->version);
    return 0;
}
