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

        ___sl_plot(NULL);
        x = z;
    }

    while (x != NULL) {
        y = x;
        x = x->next;
        free(y);
    }

    return 0;

}
