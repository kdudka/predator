#include <stdlib.h>

typedef void *TItem;

int main() {
    TItem *list = malloc(sizeof(TItem));
    *list = malloc(sizeof(TItem));
    *((TItem *) *list) = NULL;

    while (list) {
        TItem *item = list;
        list = (TItem *) *list;
        free(item);
    }

    return 0;
}

/**
 * @file test-0005.c
 *
 * @brief non-recursive destruction of a SLL that consists of two nodes
 *
 * - OOM state is not handled properly and the analysis detects a
 *   dereference of NULL value and possible junk as the consequence
 *   of it
 *
 * - when analysed in "fast" mode, no flaws are detected
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
