# 2 "test-0464.c"

#define print_log(level, file, line, errno, ...) printf(__VA_ARGS__)

#include "lvmcache-prep.c"
#include "lvm2-harness.h"
#include "list-bare.c"

int main()
{
    static char vgname[32];
    static struct format_type fmt;
    return lvmcache_add_orphan_vginfo(/* FIXME */ vgname, &fmt);
}
