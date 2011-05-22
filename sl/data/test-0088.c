#include "../sl.h"
#include <stdlib.h>

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

struct my_item {
    struct list_head    link;
    void                *data;
};

int main()
{
    // head on stack
    LIST_HEAD(my_list);
    struct list_head *head = &my_list;

    // item on heap
    struct my_item *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    // binding
    my_list.next = &ptr->link;
    my_list.prev = &ptr->link;
    ptr->link.next = head;
    ptr->link.prev = head;
    ___sl_plot("00");

    // initialize Linux list traversal
    struct my_item *now = (struct my_item *)(
            (char *)head->next - __builtin_offsetof (struct my_item, link)
            );
    ___sl_plot("01-now");

    // compute next pointer
    struct my_item *next = (struct my_item *)(
            (char *)now->link.next - __builtin_offsetof (struct my_item, link)
            );
    ___sl_plot("02-next");

    return 0;
}
