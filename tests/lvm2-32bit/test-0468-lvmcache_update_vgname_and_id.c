# 2 "test-0468.c"

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

int dummy_initialise_label(struct labeller * l, struct label * label) {
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

    if (!lvmcache_label_scan(&ctx, 0))
        ___sl_error("lvmcache_label_scan() failed");

    static struct label_ops ops = {
        .initialise_label = dummy_initialise_label
    };

    static struct labeller labeller = {
        .ops = &ops
    };

    static struct device dev;
    static char pvid_s[32 + 1];
    struct lvmcache_info *info = lvmcache_add(&labeller, pvid_s, &dev, pvid_s, pvid_s, 0);
    if (!info)
        ___sl_error("lvmcache_add");

    dm_list_init(&info->mdas);

    if (!lvmcache_update_vgname_and_id(info, &nul, &nul, 0, &nul))
        ___sl_error("lvmcache_update_vgname_and_id");
}
