#include "../sl.h"
#include <stdlib.h>

#define __nondet ___sl_get_nondet_int

struct node_top {
    struct node_top *next;
    struct node_low *data;
};

struct node_low {
    struct node_low *next;
};

int main()
{
    struct node_top *top = malloc(sizeof *top);
    struct node_top *now = top;
    top->next = NULL;
    top->data = NULL;
    if (__nondet()) {
        struct node_low *ptr = malloc(sizeof *ptr);
        if (!ptr)
            abort();

        ptr->next = NULL;
        top->data = ptr;
    }

    while (__nondet()) {
        struct node_top *pi = malloc(sizeof *pi);
        if (!pi)
            abort();

        pi->next = NULL;
        pi->data = NULL;

        if (__nondet()) {
            struct node_low *ptr = malloc(sizeof *ptr);
            if (!ptr)
                abort();

            ptr->next = NULL;
            pi->data = ptr;
        }

        now->next = pi;
        now = now->next;
    }

    ___sl_plot(NULL);

    while (top) {
        now = top;
        top = top->next;
        if (now->data) {
            free(now->data);
        }
        free(now);
    }

    return 0;
}
