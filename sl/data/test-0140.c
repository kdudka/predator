#include "../sl.h"
#include "list.h"
#include <stdlib.h>

struct node {
    int                 value;
    struct list_head    embedded_head;
};

static void append_node(struct list_head *list)
{
    struct node *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->value = 0;
    list_add(&ptr->embedded_head, list);
}

int main()
{
    // NOTE: two nodes should be enough to trigger DLS abstraction by default
    //       and consequently the spurious memory leak in the end
    LIST_HEAD(list);
    append_node(&list);
    append_node(&list);

    // plot heap in each iteration
    struct node *pos;
    list_for_each_entry(pos, &list, embedded_head)
        ___sl_plot(NULL);

    // plot heap after list_for_each_entry() -- an off-value should be there
    ___sl_plot(NULL);

    // use list_entry()
    free(list_entry(list.next, struct node, embedded_head));
    free(list_entry(list.prev, struct node, embedded_head));
    return 0;
}
