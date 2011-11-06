#include <stdio.h>
#include "dll.c"

#ifndef VERBOSE
#   define VERBOSE 0
#endif

static void chk_msg(const char *msg)
{
#if VERBOSE
    fprintf(stderr, "--- checking: %s\n", msg);
#else
    (void) msg;
#endif
}

static void chk_dll_size(dll_t *list, size_t size)
{
    (void) list;
    (void) size;
}

void dll_die(const char *msg)
{
#if VERBOSE
    fprintf(stderr, "!!! %s\n", msg);
#else
    (void) msg;
#endif
    abort();
}

int main()
{
    dll_data_t data;
    // TODO: initialize, then modify and also check the data

    chk_msg("dll_init");
    dll_t list;
    dll_init(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_push_back");
    dll_push_back(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_push_front");
    dll_push_front(&list, &data);
    chk_dll_size(&list, 2);

    chk_msg("dll_destroy");
    dll_destroy(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_push_front");
    dll_push_front(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_pop_back");
    dll_pop_back(&list, &data);
    chk_dll_size(&list, 0);

    chk_msg("dll_destroy");
    dll_destroy(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_destroy");
    dll_destroy(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_push_back");
    dll_item_t *item = dll_push_back(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_insert_before");
    item = dll_insert_before(&list, item, &data);
    chk_dll_size(&list, 2);

    chk_msg("dll_insert_after");
    item = dll_insert_after(&list, item, &data);
    chk_dll_size(&list, 3);

    chk_msg("dll_remove");
    dll_remove(&list, dll_next(dll_prev(dll_prev(dll_end(&list)))));
    chk_dll_size(&list, 2);

    chk_msg("dll_pop_front");
    dll_pop_front(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_remove");
    dll_remove(&list, dll_beg(&list));
    chk_dll_size(&list, 0);

    return 0;
}
