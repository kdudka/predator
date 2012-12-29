/*
 * Linux kernel doubly linked lists
 *
 * boxes: test-0098.boxes
 */

#include <stdlib.h>
#include <verifier-builtins.h>

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
#define __list_add(new, p, n)\
{\
    (n)->prev = new;\
    (new)->next = n;\
    (new)->prev = p;\
    (p)->next = new;\
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
/*
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}
*/
struct my_item {
    struct list_head    link;
    void                *data;
};

#define append_one(head)\
{\
    now = malloc(sizeof *now);\
    now->data = NULL;\
    lh2 = &now->link;\
    lh3 = (head)->prev;\
    __list_add(lh2, lh3, head);\
    lh2 = NULL;\
    lh3 = NULL;\
    now = NULL;\
}
/*
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
*/
#define destroy(head)\
{\
    now = (struct my_item *)(\
            (char *)head->next - __builtin_offsetof (struct my_item, link)\
            );\
\
    while (&now->link != (head)) {\
        next = (struct my_item *)(\
                (char *)now->link.next - __builtin_offsetof (struct my_item, link)\
                );\
\
        free(now);\
        now = next;\
    }\
}

struct master_item {
    struct master_item      *next;
    struct list_head        nested1;
    struct list_head        nested2;
};


#define create_dll(dll)\
{\
    (dll)->prev = dll;\
    (dll)->next = dll;\
    append_one(dll);\
    append_one(dll);\
    append_one(dll);\
    while (__VERIFIER_nondet_int())\
        append_one(dll);\
\
}


#define create_sll_item(sll) {\
    pm = malloc(sizeof *pm);\
    pm->next = sll;\
    lh = &pm->nested1;\
    create_dll(lh);\
    lh = &pm->nested2;\
    create_dll(lh);\
    lh = NULL;\
    sll = pm;\
    pm = NULL;\
}

#define destroy_sll_item(sll) {\
    pm = sll;\
    sll = sll->next;\
    lh = &pm->nested1;\
    destroy(lh);\
    lh = &pm->nested2;\
    destroy(lh);\
    lh = NULL;\
    free(pm);\
    pm = NULL;\
}

int main()
{
    struct master_item *sll = NULL;
    struct master_item* pm;
    struct list_head* lh, * lh2, *lh3;
    struct my_item *now, *next;

    create_sll_item(sll);
    create_sll_item(sll);
 
    while (__VERIFIER_nondet_int())
        create_sll_item(sll);

    while (sll)
        destroy_sll_item(sll);

    return 0;
}
