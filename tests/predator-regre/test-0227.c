#include <stdlib.h>
#include <verifier-builtins.h>

struct item {
    struct item *next;
    void *data;
};

static void append(struct item **plist, void *data)
{
    struct item *item = malloc(sizeof *item);
    item->next = *plist;
    item->data = data;
    *plist = item;
}

int main()
{
    // allocate shared data
    void *data = malloc(113U);

    // create SLS 2+
    struct item *list = NULL;
    append(&list, data);
    append(&list, data);
    while (__VERIFIER_nondet_int())
        append(&list, data);

    // var-killer kills 'data' at this point

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

    // return !!data;
}

/**
 * @file test-0227.c
 *
 * @brief leak-less variant of test-0225
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
