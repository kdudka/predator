#include <stdlib.h>

typedef void *TItem;

static void dispose_list_using_while(TItem *list) {
    while (list) {
        TItem *item = list;
        list = (TItem *) *list;
        free(item);
    }
}

typedef void (*TDisposeList) (TItem *);

void test(TDisposeList dispose_list) {
    dispose_list(NULL);
}

int main() {
    test(dispose_list_using_while);
    return 0;
}

/**
 * @file test-0007.c
 *
 * @brief indirect function call (trivial case)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
