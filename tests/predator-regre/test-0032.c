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

struct binder {
    void                    *self;
    struct binder           *prev;
    struct binder           *next;
};

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

    // initialize the successor
    dst_binder->prev = src_binder;
}

struct v_list_item;

struct h_list_item {
    void                    *data;
    struct v_list_item      *subList;
    struct binder           binder;
};
DEFINE_ALLOCATOR(h_list_item)

struct v_list_item {
    void                    *data;
    struct h_list_item      *parent;
    struct binder           binder;
};
DEFINE_ALLOCATOR(v_list_item)

int main()
{
    struct h_list_item *item = alloc_h_list_item();
    insert_list_item(0, &item->binder, item);
    __VERIFIER_plot("test-0032");
    return 0;
}

/**
 * @file test-0032.c
 *
 * @brief a regression test for the garbage collector
 *
 * - focused on composite types nesting
 * - inspired by an already fixed bug, see 2327f5c for details
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
