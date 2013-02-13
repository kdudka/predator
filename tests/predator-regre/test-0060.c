// Creating and then deleting a cyclic DLL.

#include <verifier-builtins.h>
#include <stdlib.h>

#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

typedef struct dll_item dll_item_t;
struct dll_item {
    dll_item_t  *next;
    dll_item_t  *prev;
};

static dll_item_t* create_base(void)
{
    dll_item_t *i1= DLL_NEW(dll_item_t);
    if (!i1)
        abort();

    dll_item_t *i2= DLL_NEW(dll_item_t);
    if (!i2)
        abort();

    i1->next = i2;
    i1->prev = i2;

    i2->next = i1;
    i2->prev = i1;

    return i1;
}

void insert_item(dll_item_t *list)
{
    dll_item_t *item = DLL_NEW(dll_item_t);
    if (!item)
        abort();

    item->next = list;
    item->prev = list->prev;

    list->prev->next = item;
    list->prev = item;
}


static void destroy_cyclic_dll(dll_item_t *list)
{
    dll_item_t *ref = list->prev;
    while (list != ref) {
        list = list->next;
        free(list->prev);
    }
    free(ref);
}

int main()
{
    dll_item_t *list = create_base();
    __VERIFIER_plot("01-cyclic-dll-base");

    insert_item(list);
    __VERIFIER_plot("02-cyclic-dll-inserted-one");

    int i;
    for (i = 1; i; ++i)
        insert_item(list);
    __VERIFIER_plot("03-cyclic-dll-inserted-many");

    destroy_cyclic_dll(list);
    __VERIFIER_plot("04-cyclic-dll-destroyed");

    return 0;
}

/**
 * @file test-0060.c
 *
 * @brief DLS abstraction of a cyclic DLL
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
