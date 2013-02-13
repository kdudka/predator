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

static void create_list_of_length(struct list_head *head, unsigned length)
{
    INIT_LIST_HEAD(head);
    if (!length)
        // FIXME: the should be inferred from the loop condition
        return;

    unsigned i;
    for (i = 0; i < length; ++i) {
        struct node *node = NEW(struct node);
        node->list_length = length;
        list_add(&node->linkage, head);
    }
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

int main()
{
    create_list_of_length(&list0, 0);
    __VERIFIER_plot(NULL);

    create_list_no(1);
    create_list_no(2);
    create_list_no(3);
    __VERIFIER_plot(NULL);

    destroy_list_no(3);
    destroy_list_no(2);
    destroy_list_no(1);
    __VERIFIER_plot(NULL);

    return 0;
}

/**
 * @file test-0150.c
 *
 * @brief regression test focused on gl variables and integral constants
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
