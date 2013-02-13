#include <verifier-builtins.h>
#include <stdlib.h>

#define __nondet __VERIFIER_nondet_int

int main() {
    struct T {
        struct T* next;
    };

    struct T* x = NULL;
    struct T* y = NULL;

    while (__nondet()) {
        y = malloc(sizeof(*y));
        if (!y)
            abort();

        y->next = x;
        x = y;
        y = malloc(sizeof(*y));
        if (!y)
            abort();

        y->next = x;
        x = y;
    }

    while (y != NULL) {
        __VERIFIER_plot(NULL);
        x = y;
        y = y->next;
        free(x);

        __VERIFIER_plot(NULL);
        x = y;
        y = y->next;
        free(x);
    }

    return 0;
}

/**
 * @file test-0506.c
 *
 * @brief NULL-terminated SLL of length equal to 2n
 *
 * - not supported by Predator since it breaks the SLS abstraction
 * - luckily, this one is kinda theoretical
 * - originally taken from fa/data/listeven.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
