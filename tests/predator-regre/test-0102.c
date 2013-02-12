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
    struct list_head    sub1;
    struct list_head    sub2;
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

        destroy_sub(&now->sub1);
        // Oops, we forgot to destroy &now->sub2...
        // Please point us to this line, so that we can fix it!
#if 0
        destroy_sub(&now->sub2);
#endif
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

void create_sub_list(struct list_head *sub)
{
    sub->prev = sub;
    sub->next = sub;

    insert_sub(sub);
    insert_sub(sub);
    insert_sub(sub);
    insert_sub(sub);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        insert_sub(sub);
}

void insert_top(struct list_head *head)
{
    struct top_list *top = malloc(sizeof(*top));
    if (!top)
        abort();

    create_sub_list(&top->sub1);
    create_sub_list(&top->sub2);

    list_add_tail(&top->link, head);
}

void create_top(struct list_head *top)
{
    insert_top(top);
    insert_top(top);
    insert_top(top);
    insert_top(top);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
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
 * @file test-0102.c
 *
 * @brief a Linux DLS with two nested independent Linux DLSs
 *
 * - while destroying the whole data structure, we forgot
 *   to destroy one of the sub-lits
 *
 * - Predator shows us where the call of a destructor belongs:
 *
 *     test-0102.c:84:13: warning: killing junk
 *     test-0102.c:148:16: note: from call of destroy_top()
 *     test-0102.c:141:5: note: from call of main()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
