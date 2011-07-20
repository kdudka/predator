#include "../sl.h"
#include "list.h"
#include <stdlib.h>

struct my_node {
    void (*data)(void);
    struct list_head head;
};

static void append(struct list_head *list)
{
    struct my_node *node = malloc(sizeof *node);
    if (!node)
        abort();

    list_add(&node->head, list);
}

int main()
{
    LIST_HEAD(head);

    append(&head);
    append(&head);

    ___sl_plot(NULL, &head);
}
