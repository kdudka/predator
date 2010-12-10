#include "../sl.h"
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_SEQ 0

#if PREDATOR
    static void dummy_printf(void) { }
    static int dymmy_scanf(int *ptr)
    {
        int a = 1, b = 2, x;
        if (a < b)
            return 0;

        *ptr = x;
        return 1;
    }
#   define printf(...)      dummy_printf()
#   define scanf(fmt, ptr)  dymmy_scanf(ptr)
#endif

struct node {
    int             value;
    struct node     *next;
};

struct list {
    struct node     *slist;
    struct list     *next;
};

struct iterator {
    struct list     *list;
    struct node     *node;
};

#if DEBUG_SEQ
static void seq_debug(struct list *list)
{
    printf("seq_debug:");

    while (list) {
        printf(" (");

        struct node *node = list->slist;
        if (node) {
            printf("%d", node->value);
            node = node->next;
            while (node) {
                printf(" %d", node->value);
                node = node->next;
            }
        }

        printf(")");
        list = list->next;
    }

    printf("\n");
}
#else
static void seq_debug(struct list *list)
{
    (void) list;
}
#endif

static void setup_iterator(struct iterator *iter, struct list *list)
{
    if ((iter->list = list))
        iter->node = list->slist;
}

static struct node* get_next(struct iterator *iter)
{
    if (!iter->list)
        return NULL;

    struct node *current = iter->node;
    if ((iter->node = current->next))
        return current;

    if ((iter->list = iter->list->next))
        iter->node = iter->list->slist;

    return current;
}

static void seq_insert(struct list **data, int value)
{
    struct node *node = malloc(sizeof *node);
    if (!node)
        abort();

    node->next = NULL;
    node->value = value;

    struct list *item = malloc(sizeof *item);
    if (!item)
        abort();

    item->slist = node;
    item->next = *data;
    *data = item;
}

static void seq_read(struct list **data)
{
    int value;
    while (1 == scanf("%d", &value))
        seq_insert(data, value);
}

static void seq_write(struct list *data)
{
    struct iterator iter;
    struct node *node;

    setup_iterator(&iter, data);
    printf("seq_write:");

    while ((node = get_next(&iter)))
        printf(" %d", node->value);

    printf("\n");
}

static void seq_destroy(struct list *data)
{
    while (data) {
        struct list *next = data->next;

        struct node *node = data->slist;
        while (node) {
            struct node *next = node->next;
            free(node);
            node = next;
        }

        free(data);
        data = next;
    }
}

static void seq_sort_push_one(struct node ***dst,
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

static void seq_sort_pair_core(struct node **dst,
                               struct node *data,
                               struct node *next)
{
    // merge two sorted sub-lists into one
    while (data || next) {
        if (next && (!data || next->value < data->value))
            seq_sort_push_one(&dst, &next);
        else
            seq_sort_push_one(&dst, &data);
    }
}

static void seq_sort_pair(struct list **dst,
                          struct list *data,
                          struct list *next)
{
    // allocate a new empty sub-list for the result
    struct list *list = malloc(sizeof *list);
    if (!list)
        abort();

    // merge the given pair of sub-lists and insert the result into 'dst'
    list->next = *dst;
    *dst = list;
    seq_sort_pair_core(&list->slist, data->slist, next->slist);
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
        seq_sort_pair(&dst, data, next);

        // free the just processed pair of sub-lists and jump to the next one
        free(data);
        data = next->next;
        free(next);
    }

    return dst;
}

static void seq_sort(struct list **data)
{
    struct list *list = *data;

    // do O(log N) iterations
    while (list && list->next) {
        list = seq_sort_core(list);
        seq_debug(list);
    }

    *data = list;
}

int main()
{
    struct list *data = NULL;
    seq_read(&data);
    seq_write(data);
    ___sl_plot(NULL);

    // NOTE: you may mix seq_insert/seq_sort as you like, we'll take care of it
    seq_sort(&data);
    seq_write(data);
    ___sl_plot(NULL);

    seq_destroy(data);
    ___sl_plot(NULL);

    return 0;
}
