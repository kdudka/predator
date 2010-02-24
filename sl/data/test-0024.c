#include "../sl.h"
#include <stdlib.h>

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
    sl_plot("test24-00");
    sll_t list;
    list.head = NULL;
    list.last = &list.head;
    sl_plot("test24-01");

    append_item(&list);
    append_item(&list);
    append_item(&list);
    append_item(&list);
    append_item(&list);
    append_item(&list);
    append_item(&list);
    append_item(&list);
    
    sl_plot("test24-02");
    {
        sll_item_t *next = list.head->next;
        free(list.head);
        list.head = next;
    }

    append_item(&list);
    sl_plot("test24-03");

    while (list.head) {
        sll_item_t *next = list.head->next;
        sl_plot("test24-04");
        free(list.head);
        list.head = next;
    }

    sl_plot("test24-05");
    return 0;
}
