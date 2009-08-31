#include "dll.h"

#include <assert.h>

#define DLL_ASSERT_NON_EMPTY(list) \
    assert(!dll_empty(list))

#define DLL_DIE do { \
    /* this call should never return */ \
    dll_die(__FUNCTION__); \
    abort(); \
} while (0)

#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

#define DLL_SET_IF_NULL(dst, src) \
    if (!(dst)) dst = (src)

static dll_item_t* create_item(const dll_data_t *data)
{
    dll_item_t *item = DLL_NEW(dll_item_t);
    if (!item)
        DLL_DIE;

    item->data = *data;
    item->next = DLL_NULL;
    item->prev = DLL_NULL;

    return item;
}

void dll_init(dll_t *list)
{
    list->beg = DLL_NULL;
    list->end = DLL_NULL;
    list->size = 0;
}

void dll_destroy(dll_t *list)
{
    /* destroy items in reverse order */
    dll_item_t *item = list->end;
    while (item) {
        dll_item_t *prev = item->prev;
        free(item);
        item = prev;
    }

    dll_init(list);
}

int dll_empty(dll_t *list)
{
    return !(list->beg);
}

size_t dll_size(dll_t *list)
{
    return list->size;
}

dll_item_t* dll_beg(dll_t *list)
{
    return list->beg;
}

dll_item_t* dll_end(dll_t *list)
{
    return list->end;
}

dll_item_t* dll_next(dll_item_t *item)
{
    return item->next;
}

dll_item_t* dll_prev(dll_item_t *item)
{
    return item->prev;
}

dll_data_t* dll_data(dll_item_t *item)
{
    return &(item->data);
}

dll_item_t* dll_push_back(dll_t *list, const dll_data_t *data)
{
    dll_item_t *item = create_item(data);
    if ((item->prev = list->end))
        item->prev->next = item;
    DLL_SET_IF_NULL(list->beg, item);
    list->end = item;
    list->size ++;
    return item;
}

dll_item_t* dll_push_front(dll_t *list, const dll_data_t *data)
{
    dll_item_t *item = create_item(data);
    if ((item->next = list->beg))
        item->next->prev = item;
    list->beg = item;
    DLL_SET_IF_NULL(list->end, item);
    list->size ++;
    return item;
}

void dll_pop_back(dll_t *list, dll_data_t *data)
{
    DLL_ASSERT_NON_EMPTY(list);

    dll_item_t *item = list->end;
    *data = item->data;
    if ((list->end = item->prev))
        list->end->next = DLL_NULL;
    else
        list->beg = DLL_NULL;

    list->size --;
    free(item);
}

void dll_pop_front(dll_t *list, dll_data_t *data)
{
    DLL_ASSERT_NON_EMPTY(list);

    dll_item_t *item = list->beg;
    *data = item->data;
    if ((list->beg = item->next))
        list->beg->prev = DLL_NULL;
    else
        list->end = DLL_NULL;

    list->size --;
    free(item);
}

dll_item_t* dll_insert_after(dll_t *list, dll_item_t *item, const dll_data_t *data)
{
    dll_item_t *new_item = create_item(data);
    new_item->next = item->next;
    new_item->prev = item;

    if (item->next)
        item->next->prev = new_item;
    else
        list->end = new_item;

    item->next = new_item;

    list->size ++;
    return new_item;
}

dll_item_t* dll_insert_before(dll_t *list, dll_item_t *item, const dll_data_t *data)
{
    dll_item_t *new_item = create_item(data);
    new_item->next = item;
    new_item->prev = item->prev;

    if (item->prev)
        item->prev->next = new_item;
    else
        list->beg = new_item;

    item->prev = new_item;

    list->size ++;
    return new_item;
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

    list->size --;
    free(item);
}

