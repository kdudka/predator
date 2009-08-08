#ifndef DLL_H
#define DLL_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dll dll_t;
typedef struct dll_item dll_item_t;
typedef struct dll_data dll_data_t;

struct dll {
    dll_item_t      *beg;
    dll_item_t      *end;
    size_t          size;
};

/* hard-coded for now */
struct dll_data {
    int             fixed_array[7];
    const char      *string;
    dll_t           nested_list;
};

struct Item {
    dll_data_t      data;
    dll_item_t      *next;
    dll_item_t      *prev;
};

extern void dll_die(const char *msg);

void dll_init(dll_t *list);
void dll_destroy(dll_t *list);

void dll_push_back(dll_t *list, const dll_data_t *data);
void dll_push_front(dll_t *list, const dll_data_t *data);
void dll_pop_back(dll_t *list, dll_data_t *data);
void dll_pop_front(dll_t *list, dll_data_t *data);

dll_item_t* dll_beg(dll_t *list);
dll_item_t* dll_end(dll_t *list);
dll_item_t* dll_next(dll_t *list, dll_item_t *item);
dll_item_t* dll_prev(dll_t *list, dll_item_t *item);

void dll_insert_after(dll_t *list, dll_item_t *item, const dll_data_t *data);
void dll_insert_before(dll_t *list, dll_item_t *item, const dll_data_t *data);

void dll_remove_item(dll_t *list, dll_item_t *item);

#ifdef __cplusplus
}
#endif

#endif /* DLL_H */
