#include "../sl.h"
#include <stdlib.h>

struct list_head {
    struct list_head *next;
};

static void list_add_tail(struct list_head *new, struct list_head *head)
{
    new->next = head->next;
    head->next = new;
}

struct my_item {
    void *data;
    struct list_head link;
};

void append_one(struct list_head *head)
{
    struct my_item *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->data = ((void *)0);
    list_add_tail(&ptr->link, head);
}

void traverse(struct list_head *head)
{
    ___sl_plot("00");
    struct my_item *now = (struct my_item *)(
            (char *)head->next - __builtin_offsetof (struct my_item, link)
            );

    while (&now->link != (head)) {
        ___sl_plot("01");
        now = (struct my_item *)(
            (char *)now->link.next - __builtin_offsetof (struct my_item, link)
            );
    }

    ___sl_plot("02");
}

int main()
{
    struct list_head my_list = { &(my_list) };
    int i;

    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    for (i = 1; i; ++i)
        append_one(&my_list);

    traverse(&my_list);

    return 0;
}
