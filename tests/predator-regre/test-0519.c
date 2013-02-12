#include <verifier-builtins.h>
#include <stdlib.h>

struct node {
    struct node *l;
    struct node *r;
};

#define __nondet __VERIFIER_nondet_int

int main()
{
    // create root
    struct node *pt = malloc(sizeof(struct node));
    if (!pt)
        abort();

    pt->l = NULL;
    pt->r = NULL;

    while (__nondet()) {
        struct node **pn = &pt;

        if (__nondet()) {
            // seek leftmost node
            while (*pn)
                pn = &(*pn)->l;
        }
        else {
            // seek rightmost node
            while (*pn)
                pn = &(*pn)->r;
        }

        // append a new node
        *pn = malloc(sizeof(struct node));
        if (!pn)
            abort();

        (*pn)->l = NULL;
        (*pn)->r = NULL;
    }

    // destroy left sublist
    while (pt->l) {
        __VERIFIER_plot(NULL);
        struct node *next = pt->l->l;
        free(pt->l);
        pt->l = next;
    }

    // destroy right sublist
    while (pt->r) {
        __VERIFIER_plot(NULL);
        struct node *next = pt->r->r;
        free(pt->r);
        pt->r = next;
    }

    // free root
    __VERIFIER_plot(NULL);
    free(pt);
    return 0;
}

/**
 * @file test-0519.c
 *
 * @brief test-0113 once simplified for Forester
 *
 * - originally taken from fa/data/test-0113.c
 * - known to work with SE_COST1_LEN_THR == 5
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
