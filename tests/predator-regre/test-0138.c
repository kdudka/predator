#define TRIGGER_INVALID_FREE 1

#include <verifier-builtins.h>
#include "list.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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
#if TRIGGER_INVALID_FREE
        free(next);
#else
        free(list_entry(next, struct node, linkage));
#endif
	}
}

static int val_from_node(struct list_head *head) {
    struct node *entry = list_entry(head, struct node, linkage);
    return entry->value;
}

static bool gl_sort_pass()
{
    bool any_change = false;

    struct list_head *pos0 = gl_list.next;
    struct list_head *pos1;
    while (&gl_list != (pos1 = pos0->next)) {
        const int val0 = val_from_node(pos0);
        const int val1 = val_from_node(pos1);
        if (val0 <= val1) {
            // jump to next
            pos0 = pos1;
            continue;
        }

        any_change = true;
        list_move(pos0, pos1);
    }

    return any_change;
}

static void gl_sort()
{
    while (gl_sort_pass())
        ;
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
 * @file test-0138.c
 *
 * @brief test-0135 extended by invalid free (forgot to use list_entry)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
