#include <stdlib.h>

typedef void *TItem;

static void dispose_list(TItem **list) {
    TItem *item;
    TItem **list_var = list;

    while ((item = *list)) {
        TItem *next = (TItem *) *item;
        free(item);
        item = next;
        list = (TItem **) &item;
    }
    *list_var = NULL;
}

int main() {
    TItem *list =  malloc(sizeof(TItem));
    *list = malloc(sizeof(TItem));
    *((TItem *) *list) = NULL;
    dispose_list(&list);
    dispose_list(&list);
    return 0;
}

/**
 * @file test-0006.c
 *
 * @brief another variant of two nodes long SLL destruction
 *
 * - also reports problems caused by unhandled OOM state
 * - silent in "fast" mode
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
