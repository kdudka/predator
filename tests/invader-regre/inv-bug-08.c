#include <stdlib.h>

#define TRIGGER_INV_BUG 1
#define TRIGGER_SIL_BUG 0

typedef struct dll_item dll_item_t;

struct dll_item {
    dll_item_t      *next;
    dll_item_t      *prev;
};

#if TRIGGER_SIL_BUG
typedef struct dll_item *dll_t;
#else
typedef dll_item_t *dll_t;
#endif

#define DLL_NULL ((struct dll_item *) 0)
#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

static dll_item_t* create_item(void)
{
    dll_item_t *item = DLL_NEW(dll_item_t);
    if (!item)
        abort();

    item->next = DLL_NULL;
    item->prev = DLL_NULL;

    return item;
}

void dll_remove(dll_t *list, dll_item_t *item)
{
    if (item->next)
        item->next->prev = item->prev;

    if (item->prev)
        item->prev->next = item->next;
    else
        *list = item->next;

    free(item);
}

int main()
{
    /* create dll of size 2 */
    dll_t list = create_item();
    list->next = create_item();
    list->next->prev = list;

    /* attempt to remove the first item from dll */
#if TRIGGER_INV_BUG
    dll_remove(&list, list);
#else
    {
        dll_item_t *item = list;
        if (item->next)
            item->next->prev = item->prev;
        if (item->prev)
            item->prev->next = item->next;
        else
            list = item->next;
        free(item);
    }
#endif

    /* destroy remainder of dll */
    while (list) {
        dll_item_t *next = list->next;
        free(list);
        list = next;
    }

    return 0;
}
