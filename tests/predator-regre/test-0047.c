// A basic test of creating a SLL, starting with a possibly undefined list pointer.

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    int         nth;
    struct item *next;
    void        *data;
};

void insert_item(struct item **ppi)
{
    struct item *pi = malloc(sizeof(*pi));
    if (!pi)
        abort();

    if (*ppi) {
        pi->nth     = (*ppi)->nth + 1;
        pi->next    = (*ppi);
    }
    else {
        pi->nth     = 0;
        pi->next    = NULL;
    }
    *ppi = pi;
}

int main()
{
    struct item *list/* = NULL*/;

    int i;
    for(i = 1; i; ++i) {
        insert_item(&list);
        __VERIFIER_plot("snapshot-01");
    }

    return 0;
}

/**
 * @file test-0047.c
 *
 * @brief SLL creation, starting with uninitialized list pointer
 *
 * - because of the injected bug, it has to explore two times more
 *   states
 *
 * - plots two heap graphs per each iteration (two variants
 *   considered because of the uninitialized value mentioned above)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
