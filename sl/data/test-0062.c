#include "../sl.h"
#include <stdlib.h>

struct item {
    struct item *next;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (pi)
        return pi;
    else
        abort();
}

struct item* create_sll_item(struct item *next) {
    struct item *pi = alloc_or_die();
    pi->next = next;
    return pi;
}

struct item* create_sll(void)
{
    struct item *sll = create_sll_item(NULL);
    sll = create_sll_item(sll);
    sll = create_sll_item(sll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        sll = create_sll_item(sll);

    // the return will trigger further abstraction (stack frame destruction)
    ___SL_PLOT_FNC(create_sll);
    return sll;
}

struct item* create_slseg(void)
{
    struct item *sls = create_sll()->next;
    ___SL_PLOT_FNC(create_slseg);
    return sls;
}

int main()
{
    struct item *sls = create_slseg();
    ___SL_PLOT_FNC(main);
    return 0;
}
