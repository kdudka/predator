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

    while (__nondet()) {
        y = malloc(sizeof(struct T));
        if (!y)
            abort();

        y->next = x;
        y->prev = NULL;
        y->data = 0;
        if (x)
            x->prev = y;
        x = y;
    }

    y = x;
    x = NULL;

    while (y != NULL) {
        struct T* z = y;
        y = y->next;
        if (y)
            y->prev = NULL;

        z->next = x;
        z->prev = NULL;
        if (x)
            x->prev = z;

        __VERIFIER_plot(NULL);
        x = z;
    }

    while (x != NULL) {
        y = x;
        x = x->next;
        free(y);
    }

    return 0;

}

/**
 * @file test-0505.c
 *
 * @brief reversion of a NULL-terminated DLL
 *
 * - originally taken from fa/data/dll-rev.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
