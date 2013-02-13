#include <verifier-builtins.h>
#include <stdbool.h>
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

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 0; i < 1024; ++i) {
        sll->next = alloc_or_die();
        sll->next->head = head;
        sll->next->next = NULL;

        sll = sll->next;
    }

    return head;
}

bool destroy_by_any(struct item *node)
{
    if (!node)
        // nothing to destroy here
        return false;

    // jump to head and destroy whole list
    node = node->head;
    while (node) {
        struct item *next = node->next;
        free(node);
        node = next;
    }

    return true;
}

int main()
{
    struct item *sll = create_sll();
    __VERIFIER_plot(NULL);

    if (!destroy_by_any(sll->next))
        // we have only head --> destroy the head
        free(sll);

    return 0;
}

/**
 * @file test-0118.c
 *
 * @brief abstraction of SLL, each node contains a pointer to head
 *
 * - improved variant of test-0049
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
