#include <stdlib.h>

#define TRIGGER_INV_BUG 1

typedef struct dll_item dll_item_t;
typedef dll_item_t *dll_t;

struct dll_item {
    dll_item_t      *next;
    dll_item_t      *prev;
};

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

int main()
{
    /* attempt to create dll of size 3 */
    dll_t list = create_item();
    list->next = create_item();
    list->next->prev = list;

#if TRIGGER_INV_BUG
    /* oops, Invader seems to have problem with two ->next per one line */
    list->next->next = create_item();
    list->next->next->prev = list->next;
#else
    {
        dll_t tmp = list->next;
        tmp->next = create_item();
        tmp->next->prev = list->next;
    }
#endif

    /* destroy dll */
    while (list) {
        dll_item_t *next = list->next;
        free(list);
        list = next;
    }

    return 0;
}
