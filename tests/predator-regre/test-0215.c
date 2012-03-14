#include <stdlib.h>

typedef void *item_t[2];
typedef enum { ITEM_NEXT } direction_t;

typedef struct { item_t head; } user_item_t;

int main()
{
    const direction_t link_field = ITEM_NEXT;
    user_item_t item;
    item.head[link_field] = NULL;
    return 0;
}

/**
 * @file test-0215.c
 *
 * @brief test-0214 reduced to a minimal example showing a bug in killer
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
