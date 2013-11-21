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
        p->n = NULL;
        p->d = __VERIFIER_nondet_int();
        p->b = t;
        if (NULL == h)
            h = p;
        else
            t->n = p;
        t = p;
    }
    while (__VERIFIER_nondet_int());

    do {
        struct Node *p = t->b;
        free(t);
        t = p;
        if (p)
            p->n = NULL;
        else
            h = NULL;
    }
    while (t);

    return 0;
}
