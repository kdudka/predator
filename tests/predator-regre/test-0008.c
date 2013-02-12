#include <verifier-builtins.h>
#include <stdlib.h>

typedef void *TItem;

static void dispose_list_using_while(TItem *list) {
    while (list) {
        TItem *item = list;
        list = (TItem *) *list;
        free(item);
    }
}

static void dispose_list_recursively(TItem *list) {
    __VERIFIER_plot(NULL);
    if (list) {
        dispose_list_recursively((TItem *) *list);
        free(list);
    }
}

typedef void (*TDisposeList) (TItem *);

void test(TDisposeList dispose_list) {
    dispose_list(NULL);

    TItem *list = malloc(sizeof(TItem));
    *list = NULL;
    dispose_list(list);

    list = malloc(sizeof(TItem));
    *list = malloc(sizeof(TItem));
    *((TItem *) *list) = NULL;
    dispose_list(list);
}

int main() {
    test(dispose_list_using_while);
    test(dispose_list_recursively);

    return 0;
}

/**
 * @file test-0008.c
 *
 * @brief indirect function call (more realistic case)
 *
 * - generic procedure that creates a fixed length SLL and call
 *   a destruction routine, using a function pointer given as
 *   argument
 *
 * - called two times with two different functions given as the
 *   argument
 *
 * - once the test uses a loop-based SLL destruction routine
 * - once the test uses a recursive SLL destruction routine
 * - also a bit chatty when OOM errors are analyzed
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
