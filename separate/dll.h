#ifndef DLL_H
#define DLL_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DLL_NULL ((dll_item_t *) 0)

typedef struct dll dll_t;
typedef struct dll_item dll_item_t;
typedef struct dll_data dll_data_t;

struct dll {
    size_t          size;
    dll_item_t      *beg;
    dll_item_t      *end;
};

/* hard-coded for now */
struct dll_data {
    int             fixed_array[7];
    const char      *string;
    dll_t           nested_list;
};

struct dll_item {
    dll_data_t      data;
    dll_item_t      *next;
    dll_item_t      *prev;
};

extern void dll_die(const char *msg);

void dll_init(dll_t *list)
        __attribute((__nonnull__(1)));

void dll_destroy(dll_t *list)
        __attribute((__nonnull__(1)));

int dll_empty(dll_t *list)
        __attribute((__nonnull__(1)));

size_t dll_size(dll_t *list)
        __attribute((__nonnull__(1)));

dll_item_t* dll_beg(dll_t *list)
        __attribute((__nonnull__(1)));

dll_item_t* dll_end(dll_t *list)
        __attribute((__nonnull__(1)));

dll_item_t* dll_next(dll_item_t *item) \
        __attribute((__nonnull__(1)));

dll_item_t* dll_prev(dll_item_t *item)\
        __attribute((__nonnull__(1)));

dll_data_t* dll_data(dll_item_t *item);

dll_item_t* dll_push_back(dll_t *list, const dll_data_t *data)
        __attribute((__nonnull__(1, 2)));

dll_item_t* dll_push_front(dll_t *list, const dll_data_t *data)
        __attribute((__nonnull__(1, 2)));

void dll_pop_back(dll_t *list, dll_data_t *data)
        __attribute((__nonnull__(1, 2)));

void dll_pop_front(dll_t *list, dll_data_t *data)
        __attribute((__nonnull__(1, 2)));

dll_item_t* dll_insert_after(dll_t *list, dll_item_t *item,
                           const dll_data_t *data)
        __attribute((__nonnull__(1, 2, 3)));

dll_item_t* dll_insert_before(dll_t *list, dll_item_t *item,
                            const dll_data_t *data)
        __attribute((__nonnull__(1, 2, 3)));

void dll_remove(dll_t *list, dll_item_t *item)
        __attribute((__nonnull__(1)));

#ifdef __cplusplus
}
#endif

#endif /* DLL_H */
