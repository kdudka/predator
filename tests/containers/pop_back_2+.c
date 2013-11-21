#include <stdlib.h>

struct Node {
    struct Node* n;
    struct Node* b;
};

#define NEW(type) (type *) malloc(sizeof(type))

int __VERIFIER_nondet_int(void);

int main()
{
    struct Node* h = NEW(struct Node);
    struct Node* t = h;
    h->b = NULL;
    t->n = NULL;

    do {
        struct Node *p = NEW(struct Node);
        p->n = NULL;
        p->b = t;
        t->n = p;
        t = p;
    }
    while (__VERIFIER_nondet_int());

    while (h->n != t) {
        struct Node *p = t->b;
        free(t);
        t = p;
        p->n = NULL;
    }

    free(h);
    free(t);

    return 0;
}
