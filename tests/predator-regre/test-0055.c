// Creating a DLL whose node structure is for testing purposes split and the prev
// pointer is in a sub-structure of the main node structure.

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
    struct item *null;
    struct {
        void        *data;
        struct item *prev;
    } ex;
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
    pi->null = NULL;
    pi->ex.prev = NULL;
    pi->ex.data = NULL;

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
        now->next->ex.prev = now;
        now = now->next;
        __VERIFIER_plot("01-dll-append-done");
    }

    return dll;
}

int main()
{
    struct item *sll = create_dll();
    __VERIFIER_plot("02-dll-ready");
    return 0;
}

/**
 * @file test-0055.c
 *
 * @brief a regression test focused on DLS discovery
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
