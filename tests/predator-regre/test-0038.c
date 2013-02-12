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
    ((type *) malloc(sizeof(type)))

static void append_item(sll_t *list)
{
    sll_item_t **dst = list->last;

    *dst = ALLOC(sll_item_t);
    if (!*dst)
        abort();

    (*dst)->next = NULL;

    list->last = &(*dst)->next;
}

int main()
{
    __VERIFIER_plot("test38-00");
    sll_t list;
    list.head = NULL;
    list.last = &list.head;
    __VERIFIER_plot("test38-01");    // print empty list state

    int i;
    for(i=0; i<100; i++)
        append_item(&list);

    __VERIFIER_plot("test38-02");
    {
        // delete first
        sll_item_t *next = list.head->next;
        free(list.head);
        list.head = next;
    }

    __VERIFIER_plot("test38-03");

    while (list.head) {
        sll_item_t *next = list.head->next;
        if (!next)
            __VERIFIER_plot("test38-04");    // print if single item
        free(list.head);
        list.head = next;
    }

    __VERIFIER_plot("test38-05");            // print final state
    return 0;
}

/**
 * @file test-0038.c
 *
 * @brief simple SLL creation/destruction
 *
 * - one false alarm caused by the poor support of integral values
 * - contributed by Petr Peringer
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
