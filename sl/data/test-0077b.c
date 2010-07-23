// Linux lists: a test of upcasting with a wrong type. 

#include "../sl.h"

#include <linux/stddef.h>
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
    struct list_head    h1;
    struct litem        li;
};

struct gitem2 {
    struct litem        li;
    struct litem        li2;
    struct list_head    h0;
    struct list_head    h1;
};

int main()
{
#define ROOT(type, field, addr) \
    (type *) ((char *)addr - offsetof(type, field))

    struct gitem gi;
    struct list_head *lh  = &gi.h1;

    struct gitem2 *pgi2 = ROOT(struct gitem2, h1, &lh);

    // this should be out of range --> stack smashing
    pgi2->li.data = NULL;
    ___sl_plot("01");

    return 0;
}
