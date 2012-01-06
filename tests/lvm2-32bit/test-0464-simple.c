#include "lvmcache-prep.c"
#include "lvm2-harness.h"
#include "hash-bare.c"
#include "list-bare.c"

# 5 "test-0464.c"

#define NULL ((void *)0)

int main()
{
    return lvmcache_add_orphan_vginfo("vgname", NULL);
}
