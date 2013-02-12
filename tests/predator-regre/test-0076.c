// Linux lists: testing outcasting to the superior structures of (nested) list heads,
// testing outcasting for the case when the superior structure does not exist. 

#include <verifier-builtins.h>

/* #include <linux/stddef.h> */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define prefetch(x) ((void) 0)
#define typeof(x) __typeof__(x)

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

    struct litem li;
    struct list_head *lh  = &li.lhead;

    struct litem *pli = ROOT(struct litem, lhead, lh);
    struct gitem *pgi = ROOT(struct gitem, li, pli);
    __VERIFIER_plot("01");

    if (pli != &li)
        free(pli);

    if (pli != &pgi->li)
        free(pli);

    return 0;
}

/**
 * @file test-0076.c
 *
 * @brief another test/example focused on off-values handling
 *
 *
 *     test-0077b.c- a successfully caught wrong usage of an off-value
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
