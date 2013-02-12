#include <verifier-builtins.h>
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
    LIST_HEAD(list);
    append_node(&list);
    append_node(&list);

    struct node *pos;
    list_for_each_entry(pos, &list, embedded_head)
        __VERIFIER_plot(NULL);

    __VERIFIER_plot(NULL);

    // insane -- better to use list_for_each_entry_safe()
    while (&list != list.prev) {
        list.next = list.prev->prev;
        free(list_entry(list.prev, struct node, embedded_head));
        list.prev = list.next;
    }

    return 0;
}

/**
 * @file test-0139.c
 *
 * @brief traversal of two nodes length Linux DLL
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
