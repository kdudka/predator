#include <verifier-builtins.h>
#include <stdlib.h>

#define __nondet __VERIFIER_nondet_int

struct T {
    struct T* next;
    struct T* head;
};

int main() {

    struct T* head = malloc(sizeof(struct T));

    head->next = NULL;
    head->head = head;

    struct T* x = head;

    while (__nondet()) {
        x->next = malloc(sizeof(struct T));
        if (!x->next)
            abort();

        x->next->next = NULL;
        x->next->head = head;
        x = x->next;
    }

    x = head;

    while (x != NULL) {
        __VERIFIER_plot(NULL);
        x = x->next;
    }

    x = head;

    while (x != NULL) {
        struct T* y = x;
        x = x->next;
        free(y);
    }

    return 0;
}

/**
 * @file test-0510.c
 *
 * @brief traversal of a NULL-terminated SLL
 *
 * - each node of the SLL contains a pointer to head
 * - originally taken from fa/data/sll-headptr.c
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
