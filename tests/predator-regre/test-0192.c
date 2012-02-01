#include <verifier-builtins.h>

#include <stdbool.h>
#include <stdlib.h>

#define I_WANT_TO_DEBUG_JOIN 0

struct hlist {
    struct hlist *next;
};

static struct hlist* create_hlist(void)
{
    struct hlist *list = NULL;

    do {
        struct hlist *node = malloc(sizeof *node);
        if (!node)
            abort();

        node->next = list;
        list = node;
    }
    while (___sl_get_nondet_int());

    return list;
}

struct vlist {
    struct hlist *list;
    struct vlist *next;
} *gl_list;

static void insert_vlist_node(const bool real_list)
{
    struct vlist *node = malloc(sizeof *node);
    if (!node)
        abort();

    node->list = (real_list) ? create_hlist() : NULL;
    node->next = gl_list;
    gl_list = node;
}

static void create_vlist(const bool real_nested_list)
{
    insert_vlist_node(real_nested_list);
    insert_vlist_node(real_nested_list);
    while (___sl_get_nondet_int());
        insert_vlist_node(real_nested_list);
}

int main()
{
    if (___sl_get_nondet_int()) {
        create_vlist(/* real_nested_list */ true);
        ___sl_plot(NULL);
    }
    else {
        create_vlist(/* real_nested_list */ false);
        ___sl_plot(NULL);
#if I_WANT_TO_DEBUG_JOIN
        ___sl_enable_debugging_of(___SL_SYMJOIN, /* enable */ true);
        ___sl_break("joinSymHeaps() demo");
#endif
    }

    ___sl_plot(NULL);
    return 0;
}

/**
 * @file test-0192.c
 *
 * @brief modified test-0191, works with SE_ALLOW_THREE_WAY_JOIN == 3
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
