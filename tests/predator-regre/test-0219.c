#include <verifier-builtins.h>

#include <stdlib.h>

struct L2 {
    void        *proto;
    struct L2   *next;
};

struct L1 {
    struct L1   *next;
    struct L2   *l2;
};

static void l2_insert(struct L2 **list)
{
    struct L2 *item = calloc(1U, sizeof *item);
    if (!item)
        abort();

    item->proto = malloc(119U);
    if (!item->proto)
        abort();

    item->next = *list;
    *list = item;
}

static void l2_inspect(struct L2 *list)
{
    __VERIFIER_plot("02-inspect");

    void *last = list->proto;
    struct L2 *next = list->next;

    while ((list = next)) {
        __VERIFIER_plot("03-inspect-loop");
        __VERIFIER_assert(list->proto != last);

        last = list->proto;
        next = list->next;
    }
}

static void l2_destroy(struct L2 *list)
{
    do {
        struct L2 *next = list->next;
        free(list->proto);
        free(list);
        list = next;
    }
    while (list);
}

static void l1_insert(struct L1 **list)
{
    struct L1 *item = calloc(1U, sizeof *item);
    if (!item)
        abort();

    do
        l2_insert(&item->l2);
    while (__VERIFIER_nondet_int());

    item->next = *list;
    *list = item;
}

int main()
{
    static struct L1 *list;

    do
        l1_insert(&list);
    while (__VERIFIER_nondet_int());

    __VERIFIER_plot("01-ready");

    do {
        struct L1 *next = list->next;

        l2_inspect(list->l2);
        l2_destroy(list->l2);

        free(list);
        list = next;
    }
    while (list);
}

/**
 * @file test-0219.c
 *
 * @brief explains why we need to track level for layered prototypes
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
