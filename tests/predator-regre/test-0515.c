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
    }

    struct T* z = NULL;

    while (x != NULL) {
        y = x;
        x = x->next;
        y->next = z;

        __VERIFIER_plot(NULL);
        z = y;
    }

    while (y != NULL) {
        x = y;
        y = y->next;
        free(x);
    }

    return 0;
}

/**
 * @file test-0515.c
 *
 * @brief reversion of a NULL-terminated SLL
 *
 * - originally taken from fa/data/sll-rev.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
