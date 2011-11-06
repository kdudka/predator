#include <stdlib.h>

// set to zero to work around Invader bug
#define REVEAL_INV_BUG 1

typedef void *TItem;

static void dispose_list(TItem **list) {
    TItem *item;
    TItem **list_var = list;

#if REVEAL_INV_BUG
    while ((item = *list))
#else
    item = *list;
    while (item)
#endif
    {
        TItem *next = (TItem *) *item;
        free(item);
        item = next;
        list = (TItem **) &item;
    }
    *list_var = NULL;
}

int main() {
    TItem *list =  malloc(sizeof(TItem));
    *list = malloc(sizeof(TItem));
    *((TItem *) *list) = NULL;
    dispose_list(&list);
    dispose_list(&list);
    return 0;
}
