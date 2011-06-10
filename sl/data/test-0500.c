#include "../sl.h"
#include <stdlib.h>

#define __nondet ___sl_get_nondet_int

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

    ___sl_plot(NULL);

    while (y != x) {
        struct T* z = y;
        y = y->next;
        free(z);
    }

    free(x);

    return 0;

}
