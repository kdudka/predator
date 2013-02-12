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
    struct list_head    link;
    void                *data;
};

void append_one(struct list_head *head)
{
    struct my_item *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->data = NULL;
    list_add_tail(&ptr->link, head);
}

void destroy(struct list_head *head)
{
    struct my_item *now = (struct my_item *)(
            (char *)head->next - __builtin_offsetof (struct my_item, link)
            );

    __VERIFIER_plot("00");
    while (&now->link != (head)) {
        __VERIFIER_plot("01");
        struct my_item *next = (struct my_item *)(
                (char *)now->link.next - __builtin_offsetof (struct my_item, link)
                );

        __VERIFIER_plot("02");
        free(now);
        __VERIFIER_plot("03");
        now = next;
    }
    __VERIFIER_plot("04");
}

int main()
{
    LIST_HEAD(my_list);

    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);
    append_one(&my_list);

    destroy(&my_list);
    __VERIFIER_plot(NULL);

    return 0;
}

/**
 * @file test-0087.c
 *
 * @brief alternation of test-0086.c where the list head is placed at
 *
 *   zero offset within each node (surprisingly it causes the
 *   analysis to be much more difficult as the pointer-plus
 *   operation vanishes when the offset is zero)
 *
 * - plots heap graphs of Linux like DLS destruction in progress
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
