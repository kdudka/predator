#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
    struct item *prev;
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
    pi->next = NULL;
    pi->prev = NULL;

    return pi;
}

struct item* create_dll(void)
{
    struct item *dll = alloc_and_zero();
    struct item *now = dll;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        now->next = alloc_and_zero();
        now->next->prev = now;
        now = now->next;
    }

    return dll;
}

struct item* fast_forward_core(struct item *dll)
{
    __VERIFIER_plot("00-ff-begin");
    struct item *next;
    while ((next = dll->next)) {
        __VERIFIER_plot("01-ff-one-step");
        dll = next;
    }

    return dll;
}

void fast_forward(struct item **pDll)
{
    *pDll = fast_forward_core(*pDll);
}

int main()
{
    struct item *dll = create_dll();
    fast_forward(&dll);
    __VERIFIER_plot("03-ff-done");

    return 0;
}

/**
 * @file test-0057.c
 *
 * @brief concretization of DLS
 *
 * - it creates a (possibly infinite) DLL
 * - the abstraction is triggered
 * - the list is traversed from its one end to its other end
 *   (concretization/abstraction)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
