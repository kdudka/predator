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
        y = malloc(sizeof(*y));
        if (!y)
            abort();

        y->next = x;
        x = y;
    }

    while (y != NULL) {
        ___sl_plot(NULL);
        x = y;
        y = y->next;
        free(x);

        ___sl_plot(NULL);
        x = y;
        y = y->next;
        free(x);
    }

    return 0;
}
