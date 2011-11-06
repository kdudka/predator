#include <stdlib.h>

#define TRIGGER_INV_BUG 1

typedef struct sll          sll_t;
typedef struct sll_item     sll_item_t;

struct sll {
    sll_item_t      *head;
    sll_item_t      **last;
};

struct sll_item {
    sll_item_t      *next;
    sll_t           *list;
};

#define DLL_NULL ((struct sll_item *) 0)
#define DLL_NEW(type) \
    ((type *) malloc(sizeof(type)))

static void append_item(sll_t *list)
{
    sll_item_t **dst = list->last;

    *dst = DLL_NEW(sll_item_t);
    if (!*dst)
        abort();

    (*dst)->next = NULL;
    (*dst)->list = list;

    list->last = &(*dst)->next;
}

int main()
{
    sll_t list;
    list.head = NULL;
    list.last = &list.head;

    append_item(&list);
    append_item(&list);
#if TRIGGER_INV_BUG
    append_item(&list);
#endif
    
    /* destroy list in reverse order - complexity (n^2 / 2) */
    while (list.head) {
        sll_item_t **tmp = &list.head;
        while ((*tmp)->next)
            tmp = &(*tmp)->next;

        free(*tmp);
        *tmp = NULL;
    }

    return 0;
}
