#include "../sl.h"
#include <stdlib.h>

#define __nondet ___sl_get_nondet_int

struct T {
    struct T* next;
    struct T* head;
};

int main() {

    struct T* head = malloc(sizeof(struct T));

    head->next = NULL;
    head->head = head;

    struct T* x = head;

    while (__nondet()) {
        x->next = malloc(sizeof(struct T));
        if (!x->next)
            abort();

        x->next->next = NULL;
        x->next->head = head;
        x = x->next;
    }

    x = head;

    while (x != NULL) {
        ___sl_plot(NULL);
        x = x->next;
    }

    x = head;

    while (x != NULL) {
        struct T* y = x;
        x = x->next;
        free(y);
    }

    return 0;
}
