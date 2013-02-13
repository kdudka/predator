#include <verifier-builtins.h>
#include <stdlib.h>

#define __nondet __VERIFIER_nondet_int

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

    __VERIFIER_plot(NULL);

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

/**
 * @file test-0520.c
 *
 * @brief test-0128 once simplified for Forester
 *
 * - originally taken from fa/data/test-0128.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
