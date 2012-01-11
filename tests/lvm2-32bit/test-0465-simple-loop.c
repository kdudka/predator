# 2 "test-0465.c"

#include <verifier-builtins.h>

#define print_log(level, file, line, errno, ...) /* TODO printf(__VA_ARGS__) */

#include "lvmcache-prep.c"
#include "lvm2-harness.h"

#include "list-bare.c"

#define NULL ((void *)0)

/*
 * Given the address v of an instance of 'struct dm_list' called 'head' 
 * contained in a structure of type t, return the containing structure.
 */
#define dm_list_struct_base(v, t, head) \
    ((t *)((const char *)(v) - (const char *)&((t *) 0)->head))

/*
 * Given the address v of an instance of 'struct dm_list list' contained in
 * a structure of type t, return the containing structure.
 */
#define dm_list_item(v, t) dm_list_struct_base((v), t, list)

/*
 * Given the address v of one known element e in a known structure of type t,
 * return another element f.
 */
#define dm_struct_field(v, t, e, f) \
    (((t *)((uintptr_t)(v) - (uintptr_t)&((t *) 0)->e))->f)

/*
 * Given the address v of a known element e in a known structure of type t,
 * return the list head 'list'
 */
#define dm_list_head(v, t, e) dm_struct_field(v, t, e, list)

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

struct ht_node {
    void *data;
    struct dm_list list;
};

/* overridden simplified implementation of dm_hash_lookup() */
void *dm_hash_lookup(struct dm_hash_table *t, const char *key)
{
    struct dm_list *head = (struct dm_list *) t;
    struct dm_list *pos;
    (void) key;

    /* return either random list node or NULL */
    for (pos = head->n; head != pos; pos = pos->n)
        if (___sl_get_nondet_int())
            return dm_list_item(pos, struct ht_node)->data;

    return NULL;
}

/* overridden simplified implementation of dm_hash_insert() */
int dm_hash_insert(struct dm_hash_table *t, const char *key, void *data)
{
    void *head = t;
    struct dm_list *pos = head;
    (void) key;

    /* seek random list position */
    while (___sl_get_nondet_int())
        pos = pos->p;

    if (head != pos && ___sl_get_nondet_int())
        /* simulate lookup success if we have at least one node in the list */
        return 0;

    /* allocate a new node */
    struct ht_node *node = malloc(sizeof *node);
    if (!node)
        return 0;

    node->data = data;

    /* insert the new node at a random position in the list */
    dm_list_add(pos, &node->list);
    return 1;
}

int main()
{
    static char vgname[32];
    static struct format_type fmt;
    while (___sl_get_nondet_int())
        lvmcache_add_orphan_vginfo(/* FIXME */ vgname, &fmt);

    return 0;
}
