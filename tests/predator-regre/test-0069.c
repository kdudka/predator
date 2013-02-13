#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
    struct item *prev;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (pi)
        return pi;
    else
        abort();
}

struct item* alloc_and_zero(void)
{
    struct item *pi = alloc_or_die();
    pi->next = NULL;
    pi->prev = NULL;

    return pi;
}

void create_dll(struct item **beg, struct item **end)
{
    struct item *now = alloc_and_zero();
    *beg = now;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        now->next = alloc_and_zero();
        now->next->prev = now;
        now = now->next;
    }

    *end = now;
}

void cut_dll(struct item **pbeg, struct item **pend)
{
    struct item *beg = *pbeg;
    struct item *end = *pend;

    *pbeg = beg->next;
    *pend = end->prev;
}

int main()
{
    struct item *beg, *end;
    create_dll(&beg, &end);
    cut_dll(&beg, &end);

    __VERIFIER_plot("00");
    free(beg);
    __VERIFIER_plot("01");

    if (end != beg) {
        __VERIFIER_plot("02");
        free(end);
        __VERIFIER_plot("03");
    }

    return 0;
}

/**
 * @file test-0069.c
 *
 * @brief a regression test focused on handling of 1+/2+ DLS
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
