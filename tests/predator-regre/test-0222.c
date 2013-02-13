#include <verifier-builtins.h>
#include <stdlib.h>
#include "list.h"

LIST_HEAD(list);

void destroy(int dir)
{
    struct list_head *pos;
    __VERIFIER_plot(NULL);

    if (dir) {
        pos = list.next;

        while (pos != &list) {
            struct list_head *next = pos->next;
            free(pos);
            pos = next;
        }
    }
    else {
        pos = list.prev;

        while (pos != &list) {
            struct list_head *prev = pos->prev;
            free(pos);
            pos = prev;
        }
    }
}

int main()
{
    struct list_head *item;

    do
        list_add((item = malloc(sizeof *item)), &list);
    while (__VERIFIER_nondet_int());

    if (__VERIFIER_nondet_int())
        /* XXX: misuse of list_add() */
        list_add(item, &list);

    do
        list_add((item = malloc(sizeof *item)), &list);
    while (__VERIFIER_nondet_int());

    struct list_head *const beg = list.next;
    struct list_head *const end = list.prev;

    struct list_head *now;
    for (now = list.prev; &list != now; now = now->prev) {
        __VERIFIER_assert(now == end || now->next != &list);
        __VERIFIER_assert(now == beg || now->prev != &list);
    }

    /* FIXME: why are beg/end still alive at this point?? */
    destroy(!!__VERIFIER_nondet_int());
    return 0;
}

/**
 * @file test-0222.c
 *
 * @brief misuse of list_add(), suggested by Vadim Mutilin
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
