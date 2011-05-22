// Creating and destroying a cyclic SLL. The creation is by repeated insertion.

#include "../sl.h"
#include <stdlib.h>

#define SLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

typedef struct sll_item sll_item_t;
struct sll_item {
    void        *data;
    sll_item_t  *next;
};

static sll_item_t* create_item(sll_item_t *list)
{
    sll_item_t *item = SLL_NEW(sll_item_t);
    if (!item)
        abort();

    item->next = (list)
        ? list
        : item;

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

    return plist;
}

static void destroy_cyclic_sll(sll_item_t **plist)
{
    sll_item_t *list = *plist;
    if (list) {
        sll_item_t *item = list->next;
        while (item != list) {
            sll_item_t *next = item->next;
#if 0 // would you like to try our garbage collector? :-)
            free(item);
#endif
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
    for (i = 1; i; ++i) {
        insert_item(insert_item(&list));
    }

    ___sl_plot("01-cyclic-sll-ready");

    for (i = 1; i; ++i) {
        destroy_cyclic_sll(&list);
    }

    ___sl_plot("02-cyclic-sll-gone");

    return 0;
}
