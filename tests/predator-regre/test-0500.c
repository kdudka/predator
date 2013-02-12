#include <verifier-builtins.h>
#include <stdlib.h>

#define __nondet __VERIFIER_nondet_int

int main() {

    struct T {
        struct T* next;
        struct T* prev;
        int data;
    };

    struct T* x = NULL;
    struct T* y = NULL;

    x = malloc(sizeof(struct T));
    if (!x)
        abort();

    x->next = x;
    x->prev = x;
    x->data = 0;

    while (__nondet()) {
        y = malloc(sizeof(struct T));
        if (!y)
            abort();

        y->next = x->next;
        y->next->prev = y;
        y->prev = x;
        y->data = 0;
        x->next = y;
        y = NULL;
    }
    y = x->next;

    __VERIFIER_plot(NULL);

    while (y != x) {
        struct T* z = y;
        y = y->next;
        free(z);
    }

    free(x);

    return 0;

}

/**
 * @file test-0500.c
 *
 * @brief cyclic DLL containing no useful data
 *
 * - originally taken from fa/data/cdll.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
