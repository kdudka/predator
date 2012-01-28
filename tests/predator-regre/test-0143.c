#include <verifier-builtins.h>
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

/**
 * @file test-0143.c
 *
 * @brief test-0032.c reduced to a minimal example while debugging a bug
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
