#include <verifier-builtins.h>
#include <stdlib.h>
#include <stdbool.h>

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

    if (!x)
        return 0;

    struct T* pred, * succ;

    bool sorted = false;

    while (!sorted) {
        sorted = true;
        y = x;
        pred = NULL;
        while (y && y->next) {
            if (!y->next) {
                __VERIFIER_plot(NULL);
                ___sl_error("this should not have happened");
            }

            if (__nondet()) {
                succ = y->next;
                if (pred) pred->next = succ;
                else x = succ;
                y->next = succ->next;
                succ->next = y;
                sorted = false;
            }
            pred = y;
            y = y->next;
        }
    }

    while (x != NULL) {
        y = x;
        x = x->next;
        free(y);
    }

    return 0;
}

/**
 * @file test-0508.c
 *
 * @brief Bubble-Sort operating on NULL-terminated SLL
 *
 * - originally taken from fa/data/sll-bubblesort.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
