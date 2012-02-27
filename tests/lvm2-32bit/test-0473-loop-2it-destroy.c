# 2 "test-0473.c"

#include <verifier-builtins.h>

#define print_log(level, file, line, errno, ...) /* TODO printf(__VA_ARGS__) */

#include "lvmcache-prep.c"
#include "lvm2-harness.h"

#include "list-bare.c"

int main()
{
    static char vgname[32];
    static struct format_type fmt;

    int i;
    for (i = 0; i < 2; ++i)
        lvmcache_add_orphan_vginfo(/* FIXME */ vgname, &fmt);

    lvmcache_destroy(NULL, 0);
    return 0;
}
