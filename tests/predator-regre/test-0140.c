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
    // NOTE: two nodes should be enough to trigger DLS abstraction by default
    //       and consequently the spurious memory leak in the end
    LIST_HEAD(list);
    append_node(&list);
    append_node(&list);

    // plot heap in each iteration
    struct node *pos;
    list_for_each_entry(pos, &list, embedded_head)
        __VERIFIER_plot(NULL);

    // plot heap after list_for_each_entry() -- an off-value should be there
    __VERIFIER_plot(NULL);

    // use list_entry()
    free(list_entry(list.next, struct node, embedded_head));
    free(list_entry(list.prev, struct node, embedded_head));
    return 0;
}

/**
 * @file test-0140.c
 *
 * @brief traversal of two nodes length Linux DLL
 *
 * - spurious memory leak caused by the destructor which expects
 *   exactly two nodes
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
