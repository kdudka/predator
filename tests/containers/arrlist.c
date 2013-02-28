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
    struct List A[2] = {{0,0},{0,0}};
    int i = 0;
    int j = 0;

    while (i < 10) {
        if (++j == 2) j = 0;
        if (NULL == A[j].h) {
            A[j].t = NEW(struct Node);
            A[j].t->d = i;
            A[j].t->n = NULL;
            A[j].t->b = NULL;
            A[j].h = A[j].t;
        }
        else {
            struct Node *p = NEW(struct Node);
            p->d = i;
            p->n = NULL;
            p->b = A[j].t;
            A[j].t->n = p;
            A[j].t = p;
        }

        // ++i;
    }

    return 0;
}
