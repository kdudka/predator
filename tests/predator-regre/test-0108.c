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

void destroy(struct list_head *head)
{
    struct my_item *now = (struct my_item *)(
            (char *)head->next - __builtin_offsetof (struct my_item, link)
            );

    while (&now->link != (head)) {
        struct my_item *next = (struct my_item *)(
                (char *)now->link.next - __builtin_offsetof (struct my_item, link)
                );

        free(now);
        now = next;
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

void create_dll(struct list_head *dll)
{
    dll->prev = dll;
    dll->next = dll;

    append_one(dll);
    append_one(dll);
}

struct master_item* create_sll_item(struct master_item *next) {
    struct master_item *pm = alloc_or_die_master();
    pm->next = next;
    create_dll(&pm->dll);
    return pm;
}

struct master_item* create_sll_of_dll(void *end)
{
    struct master_item *sll = create_sll_item(end);
    sll = create_sll_item(sll);
    sll = create_sll_item(sll);
    sll = create_sll_item(sll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        sll = create_sll_item(sll);

    // the return will trigger further abstraction (stack frame destruction)
    return sll;
}

int main()
{
    struct master_item *sll = create_sll_of_dll(NULL);
    __VERIFIER_plot(NULL);
    //___sl_break();

    while (sll) {
        struct master_item *next = sll->next;
        destroy(&sll->dll);
        free(sll);
        sll = next;
    }

    return 0;
}

/**
 * @file test-0108.c
 *
 * @brief Linux SLS with nested Linux DLLs consisting of _two_ items
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
