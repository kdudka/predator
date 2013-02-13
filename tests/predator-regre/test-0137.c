#include <verifier-builtins.h>
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#if PREDATOR
    static void dummy_printf(void) { }
    static int dymmy_scanf(int *ptr)
    {
        if (__VERIFIER_nondet_int())
            return 0;

        *ptr = __VERIFIER_nondet_int();
        return 1;
    }
#   define printf(...)      dummy_printf()
#   define scanf(fmt, ptr)  dymmy_scanf(ptr)
#endif

struct node {
    int                         value;
    struct list_head            linkage;
};

LIST_HEAD(gl_list);

static void gl_insert(int value)
{
    struct node *node = malloc(sizeof *node);
    if (!node)
        abort();

    node->value = value;
    list_add(&node->linkage, &gl_list);
}

static void gl_read()
{
    int value;
    while (1 == scanf("%d", &value))
        gl_insert(value);
}

static void gl_write()
{
    struct node *pos;
    printf("seq_write:");

    list_for_each_entry(pos, &gl_list, linkage)
        printf(" %d", pos->value);

    printf("\n");
}

static void gl_destroy()
{
    struct list_head *next;
    while (&gl_list != (next = gl_list.next)) {
        gl_list.next = next->next;
        free(list_entry(next, struct node, linkage));
	}
}

static int val_from_node(struct list_head *head) {
    struct node *entry = list_entry(head, struct node, linkage);
    return entry->value;
}

static struct list_head* gl_seek_max()
{
    struct list_head *pos, *max_pos = NULL;
    int max;

    if (list_empty(&gl_list))
        return NULL;
    else {
        max_pos = gl_list.next;
        max = val_from_node(max_pos);
    }

    // misuse of list_for_each() at this point successfully caught by Predator
    list_for_each(pos, max_pos) {
        const int value = val_from_node(pos);
        if (value < max)
            continue;

        max_pos = pos;
        max = value;
    }

    return max_pos;
}

static void gl_sort()
{
    if (list_empty(&gl_list))
        return;

    LIST_HEAD(dst);

    struct list_head *max_pos;
    while ((max_pos = gl_seek_max()))
        list_move(max_pos, &dst);

    // NOTE: see test-0134.c for a saner solution
    dst.next->prev = &gl_list;
    dst.prev->next = &gl_list;
    gl_list = dst;
}

int main()
{
    gl_read();
    gl_write();
    __VERIFIER_plot(NULL);

    gl_sort();
    gl_write();
    __VERIFIER_plot(NULL);

    gl_destroy();
    __VERIFIER_plot(NULL);

    return 0;
}

/**
 * @file test-0137.c
 *
 * @brief successfully caught misuse of list_for_each() from Linux
 *
 * - contributed by Tomas Vojnar
 *
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
