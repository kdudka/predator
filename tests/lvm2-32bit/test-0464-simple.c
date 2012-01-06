# 2 "test-0464.c"

#define print_log(level, file, line, errno, ...) printf(__VA_ARGS__)

#include "lvmcache-prep.c"
#include "lvm2-harness.h"

/* hide identifiers that are going to be overridden */
#define dm_hash_create __hidden__dm_hash_create
#define dm_hash_insert __hidden__dm_hash_insert
#define dm_hash_lookup __hidden__dm_hash_lookup

#include "hash-bare.c"

/* release identifiers that are going to be overridden */
#undef dm_hash_create
#undef dm_hash_insert
#undef dm_hash_lookup

#include "list-bare.c"

#define NULL ((void *)0)

/* overridden simplified implementation of dm_hash_create() */
struct dm_hash_table *dm_hash_create(unsigned size_hint)
{
    (void) size_hint;

    void *ptr = malloc(sizeof(struct dm_list));
    if (!ptr)
        abort();

    /* we approximate hash table using an unordered list */
    dm_list_init(ptr);
    return ptr;
}

/* overridden simplified implementation of dm_hash_lookup() */
void *dm_hash_lookup(struct dm_hash_table *t, const char *key)
{
    struct dm_list *head = (struct dm_list *) t;
    struct dm_list *pos;
    (void) key;

    /* return either random list node or NULL */
    for (pos = head->n; head != pos; pos = pos->n)
        if (___sl_get_nondet_int())
            return pos;

    return NULL;
}

/* overridden simplified implementation of dm_hash_insert() */
int dm_hash_insert(struct dm_hash_table *t, const char *key, void *data)
{
    void *head = t;
    struct dm_list *pos = head;
    (void) key;
    (void) data;

    /* seek random list position */
    while (___sl_get_nondet_int())
        pos = pos->p;

    if (head != pos && ___sl_get_nondet_int())
        /* simulate lookup success if we have at least one node in the list */
        return 0;

    /* allocate a new node */
    struct dm_list *node = malloc(sizeof *node);
    if (!node)
        return 0;

    /* insert the new node at a random position in the list */
    dm_list_add(pos, node);
    return 1;
}

int main()
{
    static char vgname[32];
    static struct format_type fmt;
    return lvmcache_add_orphan_vginfo(/* FIXME */ vgname, &fmt);
}
