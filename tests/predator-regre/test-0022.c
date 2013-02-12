#include <verifier-builtins.h>
#include <stdlib.h>

#define SLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

typedef struct sll_item sll_item_t;
struct sll_item {
    sll_item_t *next;
};

static sll_item_t* create_item(sll_item_t *list)
{
    sll_item_t *item = SLL_NEW(sll_item_t);
    if (!item)
        abort();

    item->next = (list)
        ? list
        : item;

    __VERIFIER_plot("create-done");
    return item;
}

static sll_item_t** insert_item(sll_item_t **plist)
{
    if (*plist) {
        sll_item_t *list = *plist;
        list->next = create_item(list->next);
    } else {
        *plist = create_item(NULL);
    }

    __VERIFIER_plot("insert-done");
    return plist;
}

static void destroy_cyclic_sll(sll_item_t **plist)
{
    __VERIFIER_plot("on-destroy");
    sll_item_t *list = *plist;
    if (list) {
        sll_item_t *item = list->next;
        while (item != list) {
            sll_item_t *next = item->next;
            free(item);
            item = next;
        }
        free(list);
    }
    *plist = NULL;
}

int main()
{
    sll_item_t *list = create_item(NULL);

    int i;
    for (i = 0; i < 128; ++i) {
        if (i)
            insert_item(&list);
        else
            insert_item(insert_item(&list));

        destroy_cyclic_sll(&list);
    }

    __VERIFIER_plot("before-last-free");
    free(list);
    return 0;
}

/**
 * @file test-0022.c
 *
 * @brief a dummy test for symplot
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
