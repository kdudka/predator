#include <verifier-builtins.h>
#include <stdlib.h>

typedef struct sll          sll_t;
typedef struct sll_item     sll_item_t;

struct sll {
    sll_item_t      *head;
    sll_item_t      **last;
};

struct sll_item {
    //int x;
    sll_item_t      *next;
};

#define ALLOC(type) \
    ((type *) calloc(1, sizeof(type)))

static void append_item(sll_t *list)
{
    sll_item_t **dst = list->last;

    *dst = ALLOC(sll_item_t);
    if (!*dst)
        abort();

    list->last = &(*dst)->next;
}

int main()
{
    sll_t list = {
        NULL,
        &list.head
    };

    __VERIFIER_plot(NULL);

    int i;
    for(i = 1; i; ++i)
        append_item(&list);

    __VERIFIER_plot(NULL);

    // delete first
    sll_item_t *next = list.head->next;
    free(list.head);
    list.head = next;

    __VERIFIER_plot(NULL);

    while (list.head) {
        sll_item_t *next = list.head->next;
        if (!next)
            __VERIFIER_plot(NULL);

        free(list.head);
        list.head = next;
    }

    __VERIFIER_plot(NULL);
    return 0;
}

/**
 * @file test-0166.c
 *
 * @brief error-free variant of test-0038
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
