#include <stdlib.h>

struct Node {
    int d;
    struct Node* n;
    struct Node* b;
};

struct List {
    struct Node* h;
    struct Node* t;
};

#define NEW(type) (type *) malloc(sizeof(type))

int main()
{
    struct List P = {0,0};
    struct List Q = {0,0};
    int i = 0;

    while (i < 10) {
        if (NULL == P.h) {
            P.t = NEW(struct Node);
            P.t->d = i;
            P.t->n = NULL;
            P.t->b = NULL;
            P.h = P.t;
        }
        else {
            struct Node *p = NEW(struct Node);
            p->d = i;
            p->n = NULL;
            p->b = P.t;
            P.t->n = p;
            P.t = p;
        }

        // ++i;
    }
    while (P.h != 0)
    {
        struct Node *r = P.t;
        struct Node *p = r->b;
        if (p == 0)
        {
            P.h = P.t = 0;
        }
        else
        {
            P.t = p;
            p->n = 0;
        }
        if (Q.h == 0)
        {
            r->b = 0;
            Q.h = Q.t = r;
        }
        else
        {
            r->b = Q.t;
            Q.t->n = r;
            Q.t = r;
        }
    }

    return 0;
}
