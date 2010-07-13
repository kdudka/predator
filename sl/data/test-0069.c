#include "../sl.h"
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

    ___sl_plot("00");
    free(beg);
    ___sl_plot("01");

    if (end != beg) {
        ___sl_plot("02");
        free(end);
        ___sl_plot("03");
    }

    return 0;
}
