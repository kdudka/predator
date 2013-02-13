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

    // create SLS 0+
    if (list) {
        struct item *next = list->next;
        free(list);
        list = next;
    }

    __VERIFIER_plot(NULL);

    // a memleak is reported while traversing the loop condition
    while (list) {
        struct item *next = list->next;
        if (!next)
            // this does not cover the case where the list has exactly one item
            free(list->data);

        free(list);
        list = next;
    }

    return 0;
}

/**
 * @file test-0231.c
 *
 * @brief test-0229 forced to create SLS 0+ prior to entering the loop
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
