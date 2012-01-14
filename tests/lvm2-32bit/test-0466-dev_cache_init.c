# 2 "test-0466.c"

#include <verifier-builtins.h>

#define print_log(level, file, line, errno, ...) /* TODO printf(__VA_ARGS__) */

#include "lvmcache-prep.c"
#include "lvm2-harness.h"

#include "dev-cache-bare.c"
#include "list-bare.c"

int main()
{
    static struct dev_filter filter;
    static struct cmd_context ctx = {
        .filter = &filter
    };

    if (!dev_cache_init(&ctx))
        return 1;

    return lvmcache_label_scan(&ctx, 0);
}
