/*
 * Doubly linked list destroing from the middle
 *
 * boxes:
 */

#include <stdlib.h>

struct node {
    struct node *l;
    struct node *r;
};

int __nondet(void);

int main()
{
    // create root
    struct node *pt = malloc(sizeof(struct node));
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
        (*pn)->l = NULL;
        (*pn)->r = NULL;
    }

    // destroy left sublist
    while (pt->l) {
        struct node *next = pt->l->l;
        free(pt->l);
        pt->l = next;
    }

    // destroy right sublist
    while (pt->r) {
        struct node *next = pt->r->r;
        free(pt->r);
        pt->r = next;
    }

    // free root
    free(pt);
    return 0;
}
