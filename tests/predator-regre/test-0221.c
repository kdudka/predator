#include <verifier-builtins.h>

#include <stdlib.h>

#include "list.h"

struct L2 {
    struct list_head    head;
    void               *proto;
};

struct L1 {
    struct list_head    head;
    struct list_head    l2;
};

static void l2_insert(struct list_head *list)
{
    struct L2 *item = malloc(sizeof *item);
    if (!item)
        abort();

    item->proto = malloc(119U);
    if (!item->proto)
        abort();

    list_add(&item->head, list);
}

static void l1_insert(struct list_head *list)
{
    struct L1 *item = malloc(sizeof *item);
    if (!item)
        abort();

    INIT_LIST_HEAD(&item->l2);
    l2_insert(&item->l2);

    list_add(&item->head, list);

    do
        l2_insert(&item->l2);
    while (__VERIFIER_nondet_int());
}

static void create_all_levels(struct list_head *list)
{
    l1_insert(list);
    do
        l1_insert(list);
    while (__VERIFIER_nondet_int());
}

int main()
{
    LIST_HEAD(list);
    create_all_levels(&list);
    __VERIFIER_plot("01-ready", &list);
}

/**
 * @file test-0221.c
 *
 * @brief regression test covering a fixed bug in cloning of prototypes
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
