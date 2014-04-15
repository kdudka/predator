#include <stdlib.h>

struct Node {
    int d;
    struct Node* n;
    struct Node* b;
};

#define NEW(type) (type *) malloc(sizeof(type))

int main()
{
    struct Node* h = NULL;
    struct Node* t = NULL;

    for (;;) {
        struct Node *p = NEW(struct Node);
        p->b = p;

        if (NULL == h)
            h = p;
        else
            t->n = p;

        p->n = NULL;
        p->d = 0;
        p->b = t;

        t = p;
    }

    return 0;
}
