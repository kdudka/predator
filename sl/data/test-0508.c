#include "../sl.h"
#include <stdlib.h>
#include <stdbool.h>

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
                ___sl_plot(NULL);
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
