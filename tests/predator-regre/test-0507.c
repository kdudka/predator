#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    struct T {
        struct T* next;
    };

    struct T* x = NULL;
    struct T* y = NULL;

    y = malloc(sizeof(*y));
    y->next = x;
    x = y;


    y = malloc(sizeof(*y));
    y->next = x;
    x = y;

    y = x;
    while (y != NULL)
        y = y->next;

    y = x;

    x = y;
    y = y->next;
    free(x);

    __VERIFIER_plot(NULL);

    if (y) {
        x = y;
        y = y->next;
        free(x);
    }

    return 0;
}

/**
 * @file test-0507.c
 *
 * @brief NULL-terminated SLL of length equal to 2
 *
 * - Predator reports junk unless SE_COST_OF_SEG_INTRODUCTION == 2
 * - originally taken from fa/data/listref.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
