#include <stdlib.h>
#include <verifier-builtins.h>

struct item {
    struct item *next;
    struct item *data;
};

static void append(struct item **plist)
{
    struct item *item = malloc(sizeof *item);
    item->next = *plist;

    // shared data
    item->data = (item->next)
        ? item->next->data
        : malloc(sizeof *item);

    *plist = item;
}

int main()
{
    struct item *list = NULL;

    // create SLS 1+
    do
        append(&list);
    while (__VERIFIER_nondet_int());

    while (list) {
        struct item *next = list->next;
        // the following statement plugs a memleak
#if 1
        if (!next)
            free(list->data);
#endif
        __VERIFIER_plot(NULL);

        free(list);
        list = next;
    }

    return 0;
}

/**
 * @file test-0230.c
 *
 * @brief non-artificial variant of test-0227.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
