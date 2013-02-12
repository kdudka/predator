// Creating a SLL with head pointers.

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *head;
    struct item *next;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (pi)
        return pi;
    else
        abort();
}

struct item* create_sll_head(void)
{
    struct item *head = alloc_or_die();
    head->head = head;
    head->next = NULL;
    return head;
}

struct item* create_sll(void)
{
    struct item *head = create_sll_head();
    struct item *sll = head;
    __VERIFIER_plot("00-sll-head-ready");

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        sll->next = alloc_or_die();
        sll->next->head = head;
        sll->next->next = NULL;

        sll = sll->next;
        __VERIFIER_plot("01-sll-append-done");
    }

    return sll;
}

int main()
{
    struct item *sll = create_sll();
    __VERIFIER_plot("02-sll-ready");
    return 0;
}

/**
 * @file test-0049.c
 *
 * @brief abstraction of SLL, each node contains a pointer to head
 *
 * - a few mistakes have been done in the test
 * - see test-0118.c for the sane variant of this test
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
