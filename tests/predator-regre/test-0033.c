#include <verifier-builtins.h>
#include <stdlib.h>

#define NEW(type) \
    (type *) malloc(sizeof(type))

#define DEFINE_ALLOCATOR(of)            \
static struct of* alloc_ ## of(void)    \
{                                       \
    struct of *ptr = NEW(struct of);    \
    if (ptr)                            \
        return ptr;                     \
    else                                \
        abort();                        \
}

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
DEFINE_ALLOCATOR(h_list_item)

// vertical list
struct v_list_item {
    void                    *data;          // will stay undefined
    void                    *parent;        // will stay undefined
    struct binder           binder;
};
DEFINE_ALLOCATOR(v_list_item)

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
        if (sub) {
            __VERIFIER_plot("test-0033");
            destroy_list(&sub->binder);
        }
        bind = prev;
    }

    destroy_list(&item->binder);
}

// create a descendant list
static struct v_list_item* create_v_list(void)
{
    // create 1st _v_ item
    struct v_list_item *v_item = alloc_v_list_item();
    struct v_list_item *v_list = v_item;
    insert_list_item(0, &v_item->binder, v_item);

#if 0
    // create 2nd _v_ item
    v_item = alloc_v_list_item();
    insert_list_item(&v_list->binder, &v_item->binder, v_item);

    // create 3rd _v_ item
    v_item = alloc_v_list_item();
    insert_list_item(&v_list->binder, &v_item->binder, v_item);
#endif

    return v_list;
}

int main()
{
    // create 1st _h_ item
    struct h_list_item *h_item = alloc_h_list_item();
    struct h_list_item *h_list = h_item;
    insert_list_item(0, &h_item->binder, h_item);
    h_item->sub_list = create_v_list();

    // create 2nd _h_ item
    h_item = alloc_h_list_item();
    insert_list_item(&h_list->binder, &h_item->binder, h_item);
    h_item->sub_list = NULL;

    // create 3rd _h_ item
    h_item = alloc_h_list_item();
    insert_list_item(&h_list->binder, &h_item->binder, h_item);
    h_item->sub_list = create_v_list();

    // create 4th _h_ item
    h_item = alloc_h_list_item();
    insert_list_item(&h_list->binder, &h_item->binder, h_item);

#if 1
    // XXX: now we introduce a heap corruption by the cross-link here :-)
    h_item->sub_list = h_list->sub_list;
#endif

    // destroy all
    destroy_h_list(h_list);

    return 0;
}

/**
 * @file test-0033.c
 *
 * @brief two levels of doubly-linked lists
 *
 * - not many of nodes, some #if 1 inside
 * - mostly for debugging purposes
 * - intentionally injected a bug into the example
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
