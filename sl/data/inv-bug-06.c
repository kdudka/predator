#include <stdio.h>
#include <stdlib.h>

#define TRIGGER_INV_BUG 1

typedef struct dll dll_t;
typedef struct dll_item dll_item_t;

struct dll {
    dll_item_t      *beg;
    dll_item_t      *end;
};

struct dll_item {
    dll_item_t      *next;
    dll_item_t      *prev;
};

#define DLL_NULL ((dll_item_t *) 0)

#define DLL_DIE do { \
    /* this call should never return */ \
    dll_die(__FUNCTION__); \
    abort(); \
} while (0)

#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

void dll_die(const char *msg)
{
    (void) msg;
    abort();
}

static dll_item_t* create_item(void)
{
    dll_item_t *item = DLL_NEW(dll_item_t);
    if (!item)
        DLL_DIE;

    item->next = DLL_NULL;
    item->prev = DLL_NULL;

    return item;
}

void dll_init(dll_t *list)
{
    list->beg = DLL_NULL;
    list->end = DLL_NULL;
}

dll_item_t* dll_beg(dll_t *list)
{
    return list->beg;
}

dll_item_t* dll_next(dll_item_t *item)
{
    return item->next;
}

dll_item_t* dll_push_front(dll_t *list)
{
    dll_item_t *item = create_item();
    if ((item->next = list->beg))
        item->next->prev = item;
    list->beg = item;
    if (!list->end)
        list->end = item;
    return item;
}


void dll_remove(dll_t *list, dll_item_t *item)
{
    if (!item)
        return;

    if (item->next)
        item->next->prev = item->prev;
    else
        list->end = item->prev;

    if (item->prev)
        item->prev->next = item->next;
    else
        list->beg = item->next;

    free(item);
}

int main()
{
    dll_t list;
    dll_init(&list);

    dll_push_front(&list);
    dll_push_front(&list);

#if TRIGGER_INV_BUG
    dll_remove(&list, dll_next(dll_beg(&list)));
#else
    dll_remove(&list, dll_beg(&list));
#endif
    dll_remove(&list, dll_beg(&list));

    return 0;
}
