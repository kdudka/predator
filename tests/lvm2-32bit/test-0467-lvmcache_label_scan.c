# 2 "test-0467.c"

#include <verifier-builtins.h>

#define print_log(level, file, line, errno, ...) /* TODO printf(__VA_ARGS__) */

#include "lvmcache-prep.c"
#include "lvm2-harness.h"

#include "dev-cache-bare.c"
#include "label-bare.c"
#include "list-bare.c"

int dummy_passes_filter(struct dev_filter * f, struct device * dev) {
    return 1;
}

int main()
{
    static struct dev_filter filter = {
        .passes_filter = dummy_passes_filter
    };

    static struct cmd_context ctx = {
        .filter = &filter
    };

    if (!dev_cache_init(&ctx))
        ___sl_error("dev_cache_init() failed");

    static char nul;
    if (!dev_cache_add_loopfile(&nul))
        ___sl_error("dev_cache_add_loopfile() failed");

    return lvmcache_label_scan(&ctx, 0);
}
