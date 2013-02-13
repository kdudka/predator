#include <verifier-builtins.h>
#include <stdlib.h>

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

struct my_item {
    void                *data;
    struct list_head    link;
};

void append_one(struct list_head *head)
{
    struct my_item *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->data = NULL;

    list_add_tail(&ptr->link, head);
}

void append_many(struct list_head *head)
{
    append_one(head);
    append_one(head);
    append_one(head);
}

int main()
{
    LIST_HEAD(my_list);
    __VERIFIER_plot("00");

    append_many(&my_list);
    __VERIFIER_plot("01");

    append_many(&my_list);
    __VERIFIER_plot("02");

    append_many(&my_list);
    __VERIFIER_plot("03");

    return 0;
}

/**
 * @file test-0079.c
 *
 * @brief simplified variant of test-0078.c
 *
 * - useful mainly for debugging purposes
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
