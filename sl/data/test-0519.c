#include "../sl.h"
#include <stdlib.h>

struct node {
    struct node *l;
    struct node *r;
};

#define __nondet ___sl_get_nondet_int

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
        ___sl_plot(NULL);
        struct node *next = pt->l->l;
        free(pt->l);
        pt->l = next;
    }

    // destroy right sublist
    while (pt->r) {
        ___sl_plot(NULL);
        struct node *next = pt->r->r;
        free(pt->r);
        pt->r = next;
    }

    // free root
    ___sl_plot(NULL);
    free(pt);
    return 0;
}
