#include <verifier-builtins.h>

#include <stdlib.h>

struct node {
    void            *data;
    struct node     *next;
    void            *list;
};

static void insert_node(struct node **pn)
{
    struct node *node = malloc(sizeof *node);
    if (!node)
        abort();

    node->data = malloc(0x100);
    node->next = *pn;
    node->list = pn;

    *pn = node;
    __VERIFIER_plot(NULL);
}

int main()
{
    struct node *list = NULL;
    insert_node(&list);
    insert_node(&list);

    __VERIFIER_plot(NULL, &list);

    while (list) {
        struct node *next = list->next;
        free(list->data);
        free(list);

        list = next;
    }

    return 0;
}

/**
 * @file test-0194.c
 *
 * @brief a trivial example on use of joinData(), once used as a demo
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
