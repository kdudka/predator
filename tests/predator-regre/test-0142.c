#include <verifier-builtins.h>
#include "list.h"
#include <stdlib.h>
#include <stdio.h>

struct node {
    int                         value;
    struct list_head            linkage;
};

LIST_HEAD(gl_list);

static void gl_insert(void)
{
    struct node *node = malloc(sizeof *node);
    if (!node)
        abort();

    list_add(&node->linkage, &gl_list);
}

static int foo(int value)
{
    return value;
}

int main()
{
    gl_insert();
    gl_insert();

    struct list_head *pos;
    for (pos = gl_list.next; &gl_list != pos; pos = pos->next)
    {
        struct node *entry = list_entry(pos, struct node, linkage);
        const int value = entry->value;
        foo(value);
    }

    return 0;
}

/**
 * @file test-0142.c
 *
 * @brief simplification of test-0133.c (only some offset manipulation)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
