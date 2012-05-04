#include <verifier-builtins.h>

#include <stdlib.h>

struct L4 {
    struct L4    *next;
    struct L5    *down;
};

struct L3 {
    struct L4    *down;
    struct L3    *next;
};

struct L2 {
    struct L2    *next;
    struct L3    *down;
};

struct L1 {
    struct L2    *down;
    struct L1    *next;
};

struct L0 {
    struct L0    *next;
    struct L1    *down;
};

static void* zalloc_or_die(unsigned size)
{
    void *ptr = calloc(1U, size);
    if (ptr)
        return ptr;

    abort();
}

static void l4_insert(struct L4 **list)
{
    struct L4 *item = zalloc_or_die(sizeof *item);
    item->down = zalloc_or_die(119U);

    item->next = *list;
    *list = item;
}

static void l3_insert(struct L3 **list)
{
    struct L3 *item = zalloc_or_die(sizeof *item);

    do
        l4_insert(&item->down);
    while (___sl_get_nondet_int());

    item->next = *list;
    *list = item;
}

static void l2_insert(struct L2 **list)
{
    struct L2 *item = zalloc_or_die(sizeof *item);

    do
        l3_insert(&item->down);
    while (___sl_get_nondet_int());

    item->next = *list;
    *list = item;
}

static void l1_insert(struct L1 **list)
{
    struct L1 *item = zalloc_or_die(sizeof *item);

    do
        l2_insert(&item->down);
    while (___sl_get_nondet_int());

    item->next = *list;
    *list = item;
}

static void l0_insert(struct L0 **list)
{
    struct L0 *item = zalloc_or_die(sizeof *item);

    do
        l1_insert(&item->down);
    while (___sl_get_nondet_int());

    item->next = *list;
    *list = item;
}

static void l4_destroy(struct L4 *list, int level)
{
    do {
        if (5 == level)
            free(list->down);

        struct L4 *next = list->next;
        if (4 == level)
            free(list);

        list = next;
    }
    while (list);
}

static void l3_destroy(struct L3 *list, int level)
{
    do {
        if (3 < level)
            l4_destroy(list->down, level);

        struct L3 *next = list->next;
        if (3 == level)
            free(list);

        list = next;
    }
    while (list);
}

static void l2_destroy(struct L2 *list, int level)
{
    do {
        if (2 < level)
            l3_destroy(list->down, level);

        struct L2 *next = list->next;
        if (2 == level)
            free(list);

        list = next;
    }
    while (list);
}

static void l1_destroy(struct L1 *list, int level)
{
    do {
        if (1 < level)
            l2_destroy(list->down, level);

        struct L1 *next = list->next;
        if (1 == level)
            free(list);

        list = next;
    }
    while (list);
}

static void l0_destroy(struct L0 *list, int level)
{
    do {
        if (0 < level)
            l1_destroy(list->down, level);

        struct L0 *next = list->next;
        if (0 == level)
            free(list);

        list = next;
    }
    while (list);
}

int main()
{
    static struct L0 *list;

    do
        l0_insert(&list);
    while (___sl_get_nondet_int());

    ___sl_plot("00-ready");

    l0_destroy(list, /* level */ 5);
    ___sl_plot("01-removed-l5");

    l0_destroy(list, /* level */ 4);
    ___sl_plot("02-removed-l4");

    l0_destroy(list, /* level */ 3);
    ___sl_plot("03-removed-l3");

    l0_destroy(list, /* level */ 2);
    ___sl_plot("04-removed-l2");

    l0_destroy(list, /* level */ 1);
    ___sl_plot("05-removed-l1");

    l0_destroy(list, /* level */ 0);
    ___sl_plot("06-removed-l0");

    return !!list;
}

/**
 * @file test-0235.c
 *
 * @brief hierarchically nested SLS up to L5, bottom-up destructor
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
