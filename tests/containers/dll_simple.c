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
    int i = 0;

    while (i < 10) {
        if (NULL == h) {
            t = NEW(struct Node);
            t->d = i;
            t->n = NULL;
            t->b = NULL;
            h = t;
        }
        else {
            struct Node *p = NEW(struct Node);
            p->d = i;
            p->n = NULL;
            p->b = t;
            t->n = p;
            t = p;
        }

        // ++i;
    }

    return 0;
}
