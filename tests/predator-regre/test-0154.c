#include <verifier-builtins.h>
#include <stdlib.h>

static void* alloc_or_die(unsigned size)
{
    void *ptr = calloc(1, size);
    if (ptr)
        return ptr;

    abort();
}

#define NEW(type) ((type *) (alloc_or_die(sizeof(type))))

// a generic doubly-linked list binder
struct binder {
    void                    *self;
    struct binder           *prev;
    struct binder           *next;
};

// insert the given item into the generic doubly-linked
void insert_list_item(struct binder *dst_binder,
                      struct binder *src_binder,
                      void          *src_item)
{
    // connect self with the item which the binder is surrounded by
    src_binder->self = src_item;

    if (!dst_binder) {
        // initialize the first item
        src_binder->prev = NULL;
        src_binder->next = NULL;
        return;
    }

    // initialize self
    src_binder->prev = dst_binder->prev;
    src_binder->next = dst_binder;

    // update the predecessor (if any)
    if (dst_binder->prev)
        dst_binder->prev->next = src_binder;

    // update the successor
    dst_binder->prev = src_binder;
}

// destroy the given generic doubly-linked list
static void destroy_list(struct binder *bind)
{
    while (bind->prev)
        bind = bind->prev;

    while (bind) {
        struct binder *next = bind->next;
        free(bind->self);
        bind = next;
    }
}

// horizontal list
struct h_list_item {
    void                    *data;          // will stay undefined
    struct v_list_item      *sub_list;      // link to a descendant list
    struct binder           binder;
};

// vertical list
struct v_list_item {
    void                    *data;          // will stay undefined
    void                    *parent;        // will stay undefined
    struct binder           binder;
};

// destroy the horizontal list, including all its descendant lists
static void destroy_h_list(struct h_list_item *item)
{
    struct binder *bind = &item->binder;
    while (bind->next)
        bind = bind->next;

    while (bind) {
        struct binder *prev = bind->prev;
        struct h_list_item *self = (struct h_list_item *) bind->self;
        struct v_list_item *sub = self->sub_list;
        if (sub)
            destroy_list(&sub->binder);

        bind = prev;
    }

    destroy_list(&item->binder);
}

// create a descendant list
static struct v_list_item* create_v_list(void)
{
    // create 1st _v_ item
    struct v_list_item *v_item = NEW(struct v_list_item);
    struct v_list_item *v_list = v_item;
    insert_list_item(0, &v_item->binder, v_item);

    // create 2nd _v_ item
    v_item = NEW(struct v_list_item);
    insert_list_item(&v_list->binder, &v_item->binder, v_item);

    // create 3rd _v_ item
    v_item = NEW(struct v_list_item);
    insert_list_item(&v_list->binder, &v_item->binder, v_item);

    return v_list;
}

int main()
{
    // create 1st _h_ item
    struct h_list_item *h_item = NEW(struct h_list_item);
    struct h_list_item *h_list = h_item;
    insert_list_item(0, &h_item->binder, h_item);
    h_item->sub_list = create_v_list();

    // create 2nd _h_ item
    h_item = NEW(struct h_list_item);
    insert_list_item(&h_list->binder, &h_item->binder, h_item);
    h_item->sub_list = NULL;

    // create 3rd _h_ item
    h_item = NEW(struct h_list_item);
    insert_list_item(&h_list->binder, &h_item->binder, h_item);
    h_item->sub_list = create_v_list();

    // create 4th _h_ item
    h_item = NEW(struct h_list_item);
    insert_list_item(&h_list->binder, &h_item->binder, h_item);

#if 0
    // XXX: now we introduce a heap corruption by the cross-link here :-)
    h_item->sub_list = h_list->sub_list;
#endif

    // destroy all
    __VERIFIER_plot(NULL);
    destroy_h_list(h_list);

    return 0;
}

/**
 * @file test-0154.c
 *
 * @brief error-free variant of test-0033
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
