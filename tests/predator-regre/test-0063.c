// Creating an SLL of SLLs which is in the end forced to be a successor of some
// regular SLL (dirty).

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
};

struct master_item {
    struct item             *slave;
    struct master_item      *next;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (pi)
        return pi;
    else
        abort();
}

struct master_item* alloc_or_die_master(void)
{
    struct master_item *pm = malloc(sizeof(*pm));
    if (pm)
        return pm;
    else
        abort();
}

struct item* create_sll_item(struct item *next) {
    struct item *pi = alloc_or_die();
    pi->next = next;
    return pi;
}

struct item* create_sll(void *end)
{
    struct item *sll = create_sll_item(end);
    sll = create_sll_item(sll);
    sll = create_sll_item(sll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        sll = create_sll_item(sll);

    // the return will trigger further abstraction (stack frame destruction)
    return sll;
}

struct item* create_slseg(void *end)
{
    struct item *list = create_sll(end);
    struct item *sls = list->next;
    free(list);
    return sls;
}

struct master_item* create_master_item(struct master_item *next) {
    struct master_item *pm = alloc_or_die_master();
    pm->slave = create_slseg(NULL);
    pm->next  = next;
    return pm;
}

struct master_item* create_shape(void)
{
    struct master_item *item = create_master_item(NULL);
    item = create_master_item(item);
    item = create_master_item(item);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        item = create_master_item(item);

    // the return will trigger further abstraction (stack frame destruction)
    return item;
}

struct master_item* create_sane_shape(void)
{
    struct master_item *list = create_shape();
    struct master_item *shape = list->next;
    free(list);
    return shape;
}

struct item* demo(void) {
    struct master_item *shape = create_sane_shape();
    struct item *all = create_slseg(shape);
    __VERIFIER_plot(NULL);
    return all;
}

int main()
{
    struct item *shape = demo();
    __VERIFIER_plot("00-shape");

    // trigger a memory leak
    free(shape);

    __VERIFIER_plot(NULL);
    return 0;
}

/**
 * @file test-0063.c
 *
 * @brief SLS of SLSs at end of another SLS
 *
 * - connected through incompatible pointer conversion
 * - plots some heap graphs that explain a lot
 * - mind the [prototype] prefix used in the SLS node visualization
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
