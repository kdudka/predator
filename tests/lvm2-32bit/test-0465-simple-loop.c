# 2 "test-0465.c"

#include <verifier-builtins.h>

#define print_log(level, file, line, errno, ...) /* TODO printf(__VA_ARGS__) */

#include "lvmcache-prep.c"
#include "lvm2-harness.h"

#include "list-bare.c"

int main()
{
    static char vgname[32];
    static struct format_type fmt;
    while (__VERIFIER_nondet_int())
        lvmcache_add_orphan_vginfo(/* FIXME */ vgname, &fmt);

    return 0;
}
