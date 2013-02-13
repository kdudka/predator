#include <verifier-builtins.h>
#include "list.h"
#include <stdlib.h>

static void* malloc_or_die(unsigned size)
{
    void *ptr = malloc(size);
    if (ptr)
        return ptr;

    abort();
}

#define NEW(type) ((type *) malloc_or_die(sizeof(type)))

struct node {
    unsigned                    list_length;
    struct list_head            linkage;
};

static void insert_node(struct list_head *head, unsigned length)
{
    struct node *node = NEW(struct node);
    node->list_length = length;
    list_add(&node->linkage, head);
}

// the name is misleading as long as we ignore the length
static void create_list_of_length(struct list_head *head, unsigned length)
{
    INIT_LIST_HEAD(head);
    insert_node(head, length);
    insert_node(head, length);

    while (__VERIFIER_nondet_int())
        insert_node(head, length);
}

static void destroy_list(struct list_head *const head)
{
    struct list_head *next;
    while (head != (next = head->next)) {
        head->next = next->next;
        free(list_entry(next, struct node, linkage));
	}
}

struct list_head list0, list1, list2, list3;

static void create_list_no(unsigned no)
{
    struct list_head *list = NULL;

    switch (no) {
        case 0:
            list = &list0;
            break;

        case 1:
            list = &list1;
            break;

        case 2:
            list = &list2;
            break;

        case 3:
            list = &list3;
            break;
    }

    create_list_of_length(list, no);
}

static void destroy_list_no(unsigned no)
{
    switch (no) {
        case 0:
            destroy_list(&list0);
            return;

        case 1:
            destroy_list(&list1);
            return;

        case 2:
            destroy_list(&list2);
            return;

        case 3:
            destroy_list(&list3);
            return;
    }
}

static void create_list_far(unsigned no, unsigned level)
{
    // NOTE: we do not support integer arithmetic for now
    switch (level) {
        case 0:
            create_list_no(no);
            return;

        case 1:
            create_list_far(no, 0);
            return;

        case 2:
            create_list_far(no, 1);
            return;

        case 3:
            create_list_far(no, 2);
            return;

        default:
            ___sl_error("this should not have been reached");
    }
}

int main()
{
    create_list_far(0, 0);
    create_list_far(1, 1);
    create_list_far(2, 2);
    create_list_far(3, 3);
    __VERIFIER_plot(NULL);

    destroy_list_no(3);
    destroy_list_no(2);
    destroy_list_no(1);
    destroy_list_no(0);
    __VERIFIER_plot(NULL);

    return 0;
}

/**
 * @file test-0151.c
 *
 * @brief regression test focused on gl variables, integral constants,
 *
 *   and recursive function calls
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
