#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
    struct item *prev;
};

#define __nondet __VERIFIER_nondet_int

int main()
{
    // create a SLL using the 'next' field
    struct item *list = NULL;
    while (__nondet()) {
        struct item *ptr = malloc(sizeof(*ptr));
        if (!ptr)
            abort();

        ptr->next = NULL;
        ptr->prev = NULL;
        ptr->next = list;

        list = ptr;
    }

    // convert the SLL to DLL by completing the 'prev' field
    struct item *pos = list;
    while (pos && pos->next) {
        struct item *prev = pos;
        pos = pos->next;
        pos->prev = prev;

        __VERIFIER_plot(NULL);
    }

    // convert the DLL to SLL by zeroing the 'next' field
    while (list && list->next) {
        struct item *next = list->next;
        list->next = NULL;
        list = next;

        __VERIFIER_plot(NULL);
    }

    // finally just destroy the list to silence our garbage collector
    while (list) {
        struct item *prev = list->prev;
        free(list);
        list = prev;
    }

    return 0;
}

/**
 * @file test-0516.c
 *
 * @brief test-0061 once simplified for Forester
 *
 * - originally taken from fa/data/test-0061.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
