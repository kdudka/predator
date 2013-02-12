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

static void l2_destroy(struct L2 *list)
{
    // XXX: tricky white-box test focused on dealing with L2 prototypes
    free(list->proto);

    do {
        struct L2 *next = list->next;
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

        l2_destroy(list->l2);

        free(list);
        list = next;
    }
    while (list);
}

/**
 * @file test-0220.c
 *
 * @brief tricky white-box test focused on dealing with L2 prototypes
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
