#include "../sl.h"
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

struct list_head_s {
    struct list_head_s *next;
};

static void list_add_tail_s(struct list_head_s *new, struct list_head_s *head)
{
    new->next = head->next;
    head->next = new;
}

struct top_list {
    struct list_head    link;
    struct list_head_s  sub;
};

struct sub_list {
    int                 number;
    struct list_head_s  link;
};

void insert_sub(struct list_head_s *head)
{
    struct sub_list *sub = malloc(sizeof(*sub));
    if (!sub)
        abort();

    sub->number = 0;

    list_add_tail_s(&sub->link, head);
}

int main()
{
    struct top_list *top = malloc(sizeof(*top));
    if (!top)
        abort();

    top->sub.next = &top->sub;

    insert_sub(&top->sub);
    insert_sub(&top->sub);

    // drop the first object
    void *ptr = top->sub.next;
    top->sub.next = top->sub.next->next;
    free(((char *)ptr) - __builtin_offsetof (struct sub_list, link));
    return 0;
}
