// Linux lists: testing outcasting to the superior structures of (nested) list heads,
// testing aliasing of the first item of a structure with the beginning of the structure. 

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

    struct gitem gi;
    struct list_head *gh0 = &gi.h0;
    struct list_head *gh1 = &gi.h1;
    struct list_head *lh  = &gi.li.lhead;

    struct gitem *pgi0 = ROOT(struct gitem, h0, gh0);
    struct gitem *pgi1 = ROOT(struct gitem, h1, gh1);
    struct litem *pli = ROOT(struct litem, lhead, lh);
    __VERIFIER_plot("01");

    if (pgi1 != &gi)
        free(pgi1);

    if (pli != &gi.li)
        free(pli);

    if (pgi0 != pgi1)
        // aliasing of the first item with address of the root
        free(pgi1);

    if ((void *)&gi != &gh0->next)
        // aliasing of the first item with address of the root (2x)
        free(pgi1);

    return 0;
}

/**
 * @file test-0075.c
 *
 * @brief off-values handling, aliasing of the 1st field with the root
 *
 * - see the checks inside and the generated graph for more info
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
