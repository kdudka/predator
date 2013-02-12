#include <verifier-builtins.h>
#include <stdlib.h>
#include <stdbool.h>

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

void traverse(struct list_head *head)
{
    struct my_item *now = (struct my_item *)(
            (char *)head->next - __builtin_offsetof (struct my_item, link)
            );

    while (&now->link != (head)) {
        now = (struct my_item *)(
                (char *)now->link.next - __builtin_offsetof (struct my_item, link)
                );
    }
}

struct master_item {
    struct master_item      *next;
    struct list_head        dll;
};

struct master_item* alloc_or_die_master(void)
{
    struct master_item *pm = malloc(sizeof(*pm));
    if (pm)
        return pm;
    else
        abort();
}

void init_dll(struct list_head *head)
{
    head->prev = head;
    head->next = head;
}

struct master_item* create_sll_item(struct master_item *next, bool two)
{
    struct master_item *pm = alloc_or_die_master();
    pm->next = next;
    init_dll(&pm->dll);
    append_one(&pm->dll);
    if (two)
        append_one(&pm->dll);
    return pm;
}

struct master_item* create_sll(bool two)
{
    return create_sll_item(create_sll_item(NULL, two), two);
}

struct master_item* create_sll_pe(void)
{
    struct master_item *item = create_sll(/* two */ false);
    struct master_item *next = item->next;
    free(item);
    return next;
}

struct master_item* create_shape(void)
{
    struct master_item *sll = create_sll_pe();
    struct master_item *beg = create_sll_item(sll, /* two */ true);
    beg->next = sll;
    return beg;
}

int main()
{
    struct master_item *shape = create_shape();
    __VERIFIER_plot(NULL);
    //___sl_break();
    return 0;
}

/**
 * @file test-0121.c
 *
 * @brief alternation of test-0097
 *
 * - once used as demo in slides about symdiscover
 * - we needed something that is easy to plot
 * - abstraction is triggerred only with SE_COST2_LEN_THR == 2
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
