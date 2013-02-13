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
        if (__nondet()) {
            if (z)
                z->next = x->next;
            else
                y = y->next;

            __VERIFIER_plot(NULL);
            free(x);
            break;
        }
        z = x;
        x = x->next;
    }

    while (y != NULL) {
        x = y;
        y = y->next;
        free(x);
    }

    return 0;
}

/**
 * @file test-0509.c
 *
 * @brief random node deletion from a NULL-terminated SLL
 *
 * - originally taken from fa/data/sll-delete.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
