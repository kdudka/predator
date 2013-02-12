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

struct top_list {
    struct list_head    link;
    struct list_head    sub;
};

struct sub_list {
    int                 number;
    struct list_head    link;
};

void destroy_sub(struct list_head *head)
{
    struct sub_list *now = (struct sub_list *)(
            (char *)head->next - __builtin_offsetof (struct sub_list, link)
            );

    while (&now->link != (head)) {
        struct sub_list *next = (struct sub_list *)((char *)now->link.next
                - __builtin_offsetof (struct sub_list, link));

        free(now);
        now = next;
    }
}

void destroy_top(struct list_head *head)
{
    struct top_list *now = (struct top_list *)(
            (char *)head->next - __builtin_offsetof (struct top_list, link)
            );

    while (&now->link != (head)) {
        struct top_list *next = (struct top_list *)((char *)now->link.next
                - __builtin_offsetof (struct top_list, link));

        destroy_sub(&now->sub);
        free(now);
        now = next;
    }
}

void insert_sub(struct list_head *head)
{
    struct sub_list *sub = malloc(sizeof(*sub));
    if (!sub)
        abort();

    sub->number = 0;

    list_add_tail(&sub->link, head);
}

void insert_top(struct list_head *head)
{
    struct top_list *top = malloc(sizeof(*top));
    if (!top)
        abort();

    top->sub.prev = &top->sub;
    top->sub.next = &top->sub;

    if (__VERIFIER_nondet_int()) {
        insert_sub(&top->sub);
        insert_sub(&top->sub);
        insert_sub(&top->sub);
        insert_sub(&top->sub);

        while (__VERIFIER_nondet_int())
            insert_sub(&top->sub);
    }

    list_add_tail(&top->link, head);
}

void create_top(struct list_head *top)
{
    insert_top(top);
    insert_top(top);
    insert_top(top);
    insert_top(top);

    while (__VERIFIER_nondet_int())
        insert_top(top);
}

int main()
{
    LIST_HEAD(top);

    create_top(&top);

    __VERIFIER_plot(NULL);
    destroy_top(&top);

    return 0;
}

/**
 * @file test-0105.c
 *
 * @brief Linux DLS with nested 0+ Linux DLS
 *
 * - a bit time consuming test-case for now
 * - we should probably handle cases like this one somehow more
 *   generically
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
