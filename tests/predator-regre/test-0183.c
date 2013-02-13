#include <stdlib.h>

extern int __VERIFIER_nondet_int(void);

static void fail(void) {
ERROR:
    goto ERROR;
}

#define __VERIFIER_assert(cond) do {     \
    if (!(cond))                    \
        fail();                     \
} while (0)

#ifndef PREDATOR
#   define __VERIFIER_plot(...) do { } while (0)
#endif

struct node {
    struct node *next;
    struct node *prev;
};

static struct node* alloc_node(void)
{
    struct node *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->next = NULL;
    ptr->prev = NULL;
    return ptr;
}

static void chain_node(struct node **ppnode)
{
    struct node *node = alloc_node();
    node->next = *ppnode;
    *ppnode = node;
}

static struct node* create_sll(const struct node **pp1, const struct node **pp2)
{
    struct node *list = NULL;

    do
        chain_node(&list);
    while (__VERIFIER_nondet_int());

    *pp2 = list;

    do
        chain_node(&list);
    while (__VERIFIER_nondet_int());

    *pp1 = list;

    do
        chain_node(&list);
    while (__VERIFIER_nondet_int());

    return list;
}

void init_back_link(struct node *list) {
    for (;;) {
        struct node *next = list->next;
        if (!next)
            return;

        next->prev = list;
        list = next;
    }
}

void reverse_dll(struct node *list) {
    while (list) {
        struct node *next = list->next;
        list->next = list->prev;
        list->prev = next;
        list = next;
    }
}

void remove_fw_link(struct node *list) {
    while (list) {
        struct node *next = list->next;
        list->next = NULL;
        list = next;
    }
}

void check_seq_next(const struct node *beg, const struct node *const end) {
    __VERIFIER_assert(beg);
    __VERIFIER_assert(end);

    for (beg = beg->next; end != beg; beg = beg->next)
        __VERIFIER_assert(beg);
}

void check_seq_prev(const struct node *beg, const struct node *const end) {
    __VERIFIER_assert(beg);
    __VERIFIER_assert(end);

    for (beg = beg->prev; end != beg; beg = beg->prev)
        __VERIFIER_assert(beg);
}

int main()
{
    const struct node *p1, *p2;

    struct node *list = create_sll(&p1, &p2);
    __VERIFIER_plot(NULL, &list, &p1, &p2);
    check_seq_next(p1, p2);
    __VERIFIER_assert(!p1->prev);
    __VERIFIER_assert(!p2->prev);

    init_back_link(list);
    __VERIFIER_plot(NULL, &list, &p1, &p2);
    check_seq_next(p1, p2);
    check_seq_prev(p2, p1);

    reverse_dll(list);
    __VERIFIER_plot(NULL, &list, &p1, &p2);
    check_seq_prev(p1, p2);
    check_seq_next(p2, p1);

    remove_fw_link(list);
    __VERIFIER_plot(NULL, &list, &p1, &p2);
    check_seq_prev(p1, p2);

    while (list) {
        struct node *prev = list->prev;
        free(list);
        list = prev;
    }

    return 0;
}

/**
 * @file test-0183.c
 *
 * @brief test-0061 extended by explicit checks of certain shape props
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
