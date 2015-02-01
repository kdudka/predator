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

    do {
        struct Node* it = h;
        while (it && __VERIFIER_nondet_int()) {
            printf("%d\n", it->d);
            it = it->n;
        }

        if (!it || !it->b)
            continue;

        struct Node *p = NEW(struct Node);
        p->b = it->b;
        p->n = it;
        it->b->n = p;
        it->b = p;
        p->d = __VERIFIER_nondet_int();
    }
    while (__VERIFIER_nondet_int());

    return !h;
}
