#include "../sl.h"
#include <stdlib.h>

#define __nondet ___sl_get_nondet_int

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

        ___sl_plot(NULL);

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
