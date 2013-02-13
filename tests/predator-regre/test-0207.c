#include <verifier-builtins.h>

#include <stdlib.h>

struct node {
    struct node     *next;
    int             value;
};

struct list {
    struct node     *slist;
    struct list     *next;
};

static void inspect_before(struct list *shape)
{
    // we should get a list of sub-lists of length exactly one
    __VERIFIER_assert(shape);

    for (; shape->next; shape = shape->next) {
        __VERIFIER_assert(shape);
        __VERIFIER_assert(shape->next);
        __VERIFIER_assert(shape->slist);
        __VERIFIER_assert(shape->slist->next == NULL);
    }

    // check the last node separately to make the exercising more fun
    __VERIFIER_assert(shape);
    __VERIFIER_assert(shape->next == NULL);
    __VERIFIER_assert(shape->slist);
    __VERIFIER_assert(shape->slist->next == NULL);
}

static void inspect_after(struct list *shape)
{
    // we should get exactly one node at the top level and one nested list
    __VERIFIER_assert(shape);
    __VERIFIER_assert(shape->next == NULL);
    __VERIFIER_assert(shape->slist != NULL);

    // the nested list should be zero terminated (iterator back by one node)
    struct node *pos;
    for (pos = shape->slist; pos->next; pos = pos->next);
    __VERIFIER_assert(!pos->next);
}

static void merge_single_node(struct node ***ppdst,
                              struct node **psrc)
{
    // pick up the current item and jump to the next one
    struct node *node = *psrc;
    *psrc = node->next;
    node->next = NULL;

    // insert the item into dst and move cursor
    **ppdst = node;
    *ppdst = &node->next;
}

static void merge_pair(struct node **pdst,
                       struct node *sub1,
                       struct node *sub2)
{
    // merge two sorted sub-lists into one
    while (sub1 || sub2) {
        if (!sub2 || (sub1 && sub1->value < sub2->value))
            merge_single_node(&pdst, &sub1);
        else
            merge_single_node(&pdst, &sub2);
    }
}

static struct list* seq_sort_core(struct list *data)
{
    struct list *dst = NULL;

    while (data) {
        struct list *next = data->next;
        if (!next) {
            // take any odd/even padding as it is
            data->next = dst;
            dst = data;
            break;
        }

        // take the current sub-list and the next one and merge them into one
        merge_pair(&data->slist, data->slist, next->slist);
        data->next = dst;
        dst = data;

        // free the just processed sub-list and jump to the next pair
        data = next->next;
        free(next);
    }

    return dst;
}

void seq_sort(struct list **ptr_to_seq)
{
    struct list *tmp = *ptr_to_seq;
#ifndef HAVE_ADVANCED_VAR_KILLER
    *ptr_to_seq = NULL;
#endif

    // do O(log N) iterations
    while (tmp->next)
        tmp = seq_sort_core(tmp);

    *ptr_to_seq = tmp;
}

int main()
{
    struct list *seq = NULL;
    while (__VERIFIER_nondet_int()) {
        struct node *node = malloc(sizeof *node);
        if (!node)
            abort();

        node->next = NULL;
        node->value = __VERIFIER_nondet_int();

        struct list *item = malloc(sizeof *item);
        if (!item)
            abort();

        item->slist = node;
        item->next = seq;
        seq = item;
    }

    if (!seq)
        return EXIT_SUCCESS;

    inspect_before(seq);

    seq_sort(&seq);

    inspect_after(seq);

    struct node *node = seq->slist;
    free(seq);

    while (node) {
        struct node *snext = node->next;
        free(node);
        node = snext;
    }

    return EXIT_SUCCESS;
}

/**
 * @file test-0207.c
 *
 * @brief var killer benchmark (merge of test-0168 and test-0177)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
