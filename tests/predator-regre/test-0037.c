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

/**
 * @file test-0037.c
 *
 * @brief glibc claims there is "double free or corruption"
 *
 * - valgrind says:
 *     Conditional jump or move depends on uninitialised value(s)
 *        at 0x4005DA: main (test-0037.c:28)
 *
 *     Invalid free() / delete / delete[]
 *        at 0x4A07100: free (vg_replace_malloc.c:366)
 *        by 0x4005F3: main (test-0037.c:31)
 *     Address 0x4c4a040 is 0 bytes inside a block of size 8 free'd
 *        at 0x4A07100: free (vg_replace_malloc.c:366)
 *        by 0x4005E7: main (test-0037.c:29)
 *
 * - Predator says:
 *     test-0037.c:27:11: error: type of the pointer being
 *     dereferenced does not match type of the target object
 *
 *     test-0037.c:31:13: error: double free() detected
 *     test-0037.c:19:24: warning: killing junk
 *     test-0037.c:19:24: warning: killing junk
 *     test-0037.c:19:24: warning: killing junk
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
