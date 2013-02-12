// Creating a DLL with all elements pointing to a shared data element (sentinel).

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *prev;
    struct item *shared;
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

struct item* alloc_and_zero(void)
{
    struct item *pi = alloc_or_die();
    pi->prev   = NULL;
    pi->shared = NULL;
    pi->next   = NULL;

    return pi;
}

struct item* create_dll(void)
{
    struct item *dll = alloc_and_zero();
    struct item *now = dll;
    dll->shared = alloc_and_zero();

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        now->next = alloc_and_zero();
        now->next->prev = now;
        now->next->shared = dll->shared;
        now = now->next;
        __VERIFIER_plot("test-0056-one-step");
    }

    return dll;
}

int main()
{
    return !!create_dll();
}

/**
 * @file test-0056.c
 *
 * @brief a regression test focused on DLS shared data
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
