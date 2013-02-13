#include <verifier-builtins.h>
#include <stdlib.h>

#define __nondet __VERIFIER_nondet_int

struct DItem {
    struct DItem* next;
    int value;
};

struct TLItem {
    struct TLItem* next;
    struct DItem* data;	
};

int main() {

    struct TLItem* data = NULL;
    struct DItem* item, * item2;
    struct TLItem *lItem;

    // fill top level list with single data items
    while (__nondet()) {

        item = malloc(sizeof *item);
        if (!item)
            abort();

        item->next = NULL;
        item->value = __nondet();

        lItem = malloc(sizeof *lItem);
        if (data) {
            lItem->next = data->next;
            data->next = lItem;
        } else {
            lItem->next = lItem;
            data = lItem;
        }
        lItem->data = item;

        item = NULL;
        lItem = NULL;
    }

    if (!data)
        return 0;

    __VERIFIER_plot(NULL);

    // merge subsequent pairs
    while (data->next != data) {

        item = data->data;
        item2 = data->next->data;

        lItem = data->next;
        data->next = lItem->next;
        free(lItem);

        struct DItem** dst = &data->data;

        while (item && item2) {

            if (item->value < item2->value) {
                *dst = item;
                item = item->next;
            } else {
                *dst = item2;
                item2 = item2->next;
            }

            dst = &(*dst)->next;
        }

        if (item) {
            *dst = item;
            item = NULL;
        } else if (item2) {
            *dst = item2;
            item2 = NULL;
        }

        dst = NULL;
        data = data->next;
    }

    __VERIFIER_plot(NULL);

    // release the list
    item = data->data;
    free(data);

    while (item) {
        item2 = item;
        item = item->next;
        free(item2);
    }

    return 0;
}

/**
 * @file test-0513.c
 *
 * @brief Merge-Sort operating on two-level SLL
 *
 * - originally taken from fa/data/sll-mergesort.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
