#include "../sl.h"
#include <stdlib.h>

struct pair {
    void *p0;
    void *p1;
};

struct T {
    void        *self;
    struct pair inner;
};

static int gl_int = 7 * 13;

static struct T local = {
    .self = &local,
    .inner = {
        .p0 = &local.inner.p0,
        .p1 = &local.inner.p1
    }
};

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head *head)
{
    //___sl_plot(NULL);
	return head->next == head;
}

LIST_HEAD(gl_list);

void* foo(struct T *ptr)
{
    return ptr;
}

int main()
{
    LIST_HEAD(lc_list);
    if (!list_empty(&lc_list))
        // this should not happen as long as lc initializers work
        main();

    if (!list_empty(&gl_list))
        // this should not happen as long as gl initializers work
        main();

    if (!gl_int)
        // this should not happen as long as gl initializers work
        main();

    ___sl_plot(NULL);

    return !!foo(&local);
}
