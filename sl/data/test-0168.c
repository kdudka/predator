#include "../sl.h"
#include <stdlib.h>

struct node {
    struct node     *next;
    int             value;
};

struct list {
    struct node     *slist;
    struct list     *next;
};

struct iterator {
    struct list     *list;
    struct node     *node;
};

static void merge_single_node(struct node ***dst,
                              struct node **data)
{
    // pick up the current item and jump to the next one
    struct node *node = *data;
    *data = node->next;
    node->next = NULL;

    // insert the item into dst and move cursor
    **dst = node;
    *dst = &node->next;
}

static void merge_pair(struct node **dst,
                       struct node *sub1,
                       struct node *sub2)
{
    // merge two sorted sub-lists into one
    while (sub1 || sub2) {
        if (!sub2 || (sub1 && sub1->value < sub2->value))
            merge_single_node(&dst, &sub1);
        else
            merge_single_node(&dst, &sub2);
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

int main()
{
    struct list *data = NULL;
    while (___sl_get_nondet_int()) {
        struct node *node = malloc(sizeof *node);
        if (!node)
            abort();

        node->next = NULL;
        node->value = ___sl_get_nondet_int();

        struct list *item = malloc(sizeof *item);
        if (!item)
            abort();

        item->slist = node;
        item->next = data;
        data = item;
    }

    if (!data)
        return EXIT_SUCCESS;

    // do O(log N) iterations
    ___sl_plot(NULL);
    while (data->next)
        data = seq_sort_core(data);

    ___sl_plot(NULL);
    struct node *node = data->slist;
    free(data);

    while (node) {
        struct node *snext = node->next;
        free(node);
        node = snext;
    }

    return 0;
}
