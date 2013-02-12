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

    struct T* sorted = NULL;
    struct T* pred = NULL;
    struct T* z = NULL;

    while (x) {
        y = x;
        x = x->next;
        pred = NULL;
        z = sorted;

        while (z && __nondet()) {
            pred = z;
            z = z->next;
        }

        __VERIFIER_plot(NULL);

        y->next = z;
        if (pred) pred->next = y;
        else sorted = y;
    }

    while (sorted != NULL) {
        x = sorted;
        sorted = sorted->next;
        free(x);
    }

    return 0;

}

/**
 * @file test-0511.c
 *
 * @brief Insert-Sort operating on NULL-terminated SLL
 *
 * - originally taken from fa/data/sll-insertsort.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
