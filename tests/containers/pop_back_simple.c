#include <stdlib.h>

struct Node {
    struct Node* n;
    struct Node* b;
};

#define NEW(type) (type *) malloc(sizeof(type))

int __VERIFIER_nondet_int(void);

int main()
{
    struct Node* h = NULL;
    struct Node* t = NULL;

    while (__VERIFIER_nondet_int()) {
        struct Node *p = NEW(struct Node);
        if (NULL == h)
            h = p;
        else
            t->n = p;
        p->n = NULL;
        p->b = t;
        t = p;
    }

    while (t) {
        struct Node *p = t->b;
        if (p)
            p->n = NULL;
        else
            h = NULL;
        free(t);
        t = p;
    }

    return 0;
}
