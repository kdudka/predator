#include <stdlib.h>

#define NEW(type) \
    (type *) malloc(sizeof(type))

struct list_a {
    struct list_b *next;
};

struct list_b {
    /* some dummy content introducing an offset of next */
    void            *v;

    struct list_a   *next;
};

int main() {
    struct list_a *a = NEW(struct list_a);
    struct list_b *b = NEW(struct list_b);
    a->next = b;
    b->next = a;

    struct list_a *c = /* the following cast causes analyzer to be confused */
                       (struct list_a *) b;

    /* grab an uninitialized value and compare with a */
    void *d = c->next;
    if (a != d) {
        free(a);
        /* *** glibc detected *** ./a.out: double free or corruption */
        free(a);
    }

    return 0;
}
