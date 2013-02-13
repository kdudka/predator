#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item             *next;
    struct item             *prev;
};

struct master_item {
    struct master_item      *next;
    struct item             *dll;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (pi)
        return pi;
    else
        abort();
}

struct master_item* alloc_or_die_master(void)
{
    struct master_item *pm = malloc(sizeof(*pm));
    if (pm)
        return pm;
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

struct item* create_dll_item(struct item *dll)
{
    dll->next = alloc_and_zero();
    dll->next->prev = dll;
    return dll->next;
}

struct item* create_dll(void)
{
    struct item *dll = alloc_and_zero();
    dll = create_dll_item(dll);
    dll = create_dll_item(dll);
    dll = create_dll_item(dll);
    dll = create_dll_item(dll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        dll = create_dll_item(dll);

    //__VERIFIER_plot(NULL);
    return dll;
}

struct master_item* create_sll_item(struct master_item *next) {
    struct master_item *pm = alloc_or_die_master();
    pm->next = next;
    pm->dll = create_dll();
    return pm;
}

struct master_item* create_sll_of_dll(void *end)
{
    struct master_item *sll = create_sll_item(end);
    __VERIFIER_plot("00");
    sll = create_sll_item(sll);
    __VERIFIER_plot("01");
    sll = create_sll_item(sll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        sll = create_sll_item(sll);

    // the return will trigger further abstraction (stack frame destruction)
    return sll;
}

int main()
{
    struct master_item *sll = create_sll_of_dll(NULL);
    __VERIFIER_plot(NULL);
    return 0;
}

/**
 * @file test-0066.c
 *
 * @brief SLS of DLSs
 *
 * - a (possibly infinite) SLL
 * - each node points to a standalone (possibly infinite) DLL
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
