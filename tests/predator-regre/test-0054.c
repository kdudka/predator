// Creating a DLL.

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
        __VERIFIER_plot("01-dll-append-done");
    }

    return dll;
}

int main()
{
    struct item *dll = create_dll();
    __VERIFIER_plot("02-dll-ready");
    return 0;
}

/**
 * @file test-0054.c
 *
 * @brief simple DLL creation
 *
 * - seen infinite because we abstract out integral values
 * - the fix-point calculation terminates thanks to DLS abstraction
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
