// Linux lists: a test of writing into a non-existing envelope of a list head,
// resulting into stack smashing (since the head is on the stack).

#include <verifier-builtins.h>

/* #include <linux/stddef.h> */
#include <stddef.h>
#include <stdlib.h>

struct list_head {
	struct list_head *next, *prev;
};

struct litem {
    void *data;
    struct list_head lhead;
};

struct gitem {
    struct list_head    h0;
    struct litem        li;
    struct list_head    h1;
};

int main()
{
#define ROOT(type, field, addr) \
    (type *) ((char *)addr - offsetof(type, field))

    // let's use the following on stack variable to recognize stack smashing
    int i = -1;

    struct litem li;
    struct list_head *lh  = &li.lhead;

    struct litem *pli = ROOT(struct litem, lhead, lh);
    struct gitem *pgi = ROOT(struct gitem, li, pli);

    // this should be OK
    pli->data = pgi;
    pli->lhead.prev = NULL;
    pgi->li.lhead.next = &pli->lhead;
    __VERIFIER_plot("01");

    // this should be out of range --> stack smashing
    if (__VERIFIER_nondet_int()) {
        pgi->h0.next = NULL;
        __VERIFIER_plot("02");
    }
    else {
        pgi->h1.prev = NULL;
        __VERIFIER_plot("02");
    }

    // surprisingly i==0 at this point (because of the stack smashing)
    // ----------
    // - gcc
    // - sparse
    // - valgrind
    // - Invader
    // ----------
    // + Predator
    if (!i)
        free(pli);

    return 0;
}

/**
 * @file test-0077.c
 *
 * @brief stack smashing detection:
 *
 *     test-0077.c:46:18: error: dereference of unknown value
 *     test-0077.c:48:18: error: dereference of unknown value
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
