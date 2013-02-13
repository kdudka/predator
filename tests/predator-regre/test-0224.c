#include <verifier-builtins.h>
#include <stdlib.h>
#include "list.h"
#include "list_add.h"

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

    if (!item)
        /* XXX: _unreachable_ misuse of list_add() */
        list_add(item, &list);

    do
        list_add((item = malloc(sizeof *item)), &list);
    while (__VERIFIER_nondet_int());

    destroy(!!__VERIFIER_nondet_int());
    return 0;
}

/**
 * @file test-0224.c
 *
 * @brief error-free variant of test-0223 (the error is unreachable)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
