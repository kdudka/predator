#include "../sl.h"
#include <stdlib.h>

struct binder {
    void                    *self;
    struct binder           *next;
};

void insert_list_item(struct binder *binder, void *item)
{
    binder->self = item;
}

int main()
{
    struct h_list_item {
        void                    *data;
        struct binder           binder;
    };

    struct h_list_item *item = malloc(sizeof *item);
    insert_list_item(&item->binder, item);
    return 0;
}
