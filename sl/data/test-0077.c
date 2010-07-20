#include "../sl.h"

#include <linux/stddef.h>
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

    // let's use the following on stack variable to recognize stack smashing
    int i = -1;

    struct litem li;
    struct list_head *lh  = &li.lhead;

    struct litem *pli = ROOT(struct litem, lhead, lh);
    struct gitem *pgi = ROOT(struct gitem, li, &li);

    // this should be OK
    pli->data = pgi;
    pli->lhead.prev = NULL;
    pgi->li.lhead.next = &pli->lhead;
    ___sl_plot("01");

    // this should be out of range --> stack smashing
    pgi->h0.next = NULL;
    pgi->h1.prev = NULL;

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
