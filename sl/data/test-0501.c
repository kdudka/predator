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

    while (y != NULL) {
        if (__nondet()) {
            struct T* z = malloc(sizeof(struct T));
            if (!z)
                abort();

            z->next = y->next;
            z->prev = y;
            y->next = z;
            if (z->next)
                z->next->prev = z;

            ___sl_plot(NULL, &z);
            break;
        }
        y = y->next;
    }

    while (x != NULL) {
        y = x;
        x = x->next;
        free(y);
    }

    return 0;

}
