#include <verifier-builtins.h>

#include <stdbool.h>
#include <stdlib.h>

#define I_WANT_TO_DEBUG_JOIN 0

struct hlist {
    struct hlist *next;
};

static struct hlist* create_hlist(const bool real_list)
{
    struct hlist *list = NULL;

    do {
        struct hlist *node = malloc(sizeof *node);
        if (!node)
            abort();

        node->next = list;
        list = node;
    }
    while (real_list && __VERIFIER_nondet_int());

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

    node->list = create_hlist(real_list);
    node->next = gl_list;
    gl_list = node;
}

static void create_vlist(const bool real_nested_list)
{
    insert_vlist_node(real_nested_list);
    insert_vlist_node(real_nested_list);
    while (__VERIFIER_nondet_int());
        insert_vlist_node(real_nested_list);
}

int main()
{
    if (__VERIFIER_nondet_int()) {
        create_vlist(/* real_nested_list */ true);
        __VERIFIER_plot(NULL);
    }
    else {
        create_vlist(/* real_nested_list */ false);
        __VERIFIER_plot(NULL);
#if I_WANT_TO_DEBUG_JOIN
        ___sl_enable_debugging_of(___SL_SYMJOIN, /* enable */ true);
        ___sl_break("joinSymHeaps() demo");
#endif
    }

    __VERIFIER_plot(NULL);
    return 0;
}

/**
 * @file test-0191.c
 *
 * @brief an easy test for joinSymHeaps(), once used as a demo
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
