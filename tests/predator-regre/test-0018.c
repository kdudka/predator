#include <stdlib.h>

#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

typedef struct sll_item sll_item_t;
struct sll_item {
    sll_item_t *next;
};

static sll_item_t* create_item(sll_item_t *list)
{
    sll_item_t *item = DLL_NEW(sll_item_t);
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

int main()
{
    sll_item_t *list = NULL;

    int i;
    for (i = 0; i < 128; ++i) {
        insert_item(&list);

        if (i)
            insert_item(&list);
        else
            insert_item(insert_item(&list));

        list = NULL;
    }

    return 0;
}
