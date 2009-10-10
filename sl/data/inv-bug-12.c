#include <stdlib.h>

#define TRIGGER_INV_BUG 1

typedef struct sll_item sll_item_t;
typedef sll_item_t *sll_t;

typedef struct sll_item_data {
    sll_t           *head;
} sll_item_data_t;

struct sll_item {
    sll_item_t      *next;
    sll_item_data_t data;
};

#define DLL_NULL ((struct sll_item *) 0)
#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

static sll_item_t* create_item(const sll_item_data_t *data)
{
    sll_item_t *item = DLL_NEW(sll_item_t);
    if (!item)
        abort();

    item->next = DLL_NULL;
#if TRIGGER_INV_BUG
    item->data = *data;
#else
    (void) data;
#endif

    return item;
}

int main()
{
    sll_t list = DLL_NULL;
    sll_item_data_t data;
    data.head = &list;

    /* attempt to create sll of size 1 */
    list = create_item(&data);

    /* destroy sll */
    free(list);

    return 0;
}
