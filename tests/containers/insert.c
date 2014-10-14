#include <stdio.h>
#include <stdlib.h>

struct Node {
    int d;
    struct Node* n;
    struct Node* b;
};

#define NEW(type) (type *) malloc(sizeof(type))

int __VERIFIER_nondet_int(void);

int main()
{
    struct Node* h = NULL;
    struct Node* t = NULL;

    do {
        struct Node *p = NEW(struct Node);
        p->d = __VERIFIER_nondet_int();
        if (NULL == h)
            h = p;
        else
            t->n = p;
        p->n = NULL;
        p->b = t;
        t = p;
    }
    while (__VERIFIER_nondet_int());

    struct Node* it = h;
    while (it->n && __VERIFIER_nondet_int()) {
        printf("%d\n", it->d);
        it = it->n;
    }

    if (!it->n || !it->b)
        return 1;

    struct Node *p = NEW(struct Node);
    p->d = __VERIFIER_nondet_int();
    p->b = it->b;
    p->n = it;
    it->b->n = p;
    it->b = p;

    return !p;
}
