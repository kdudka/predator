#include <verifier-builtins.h>
#include <stdlib.h>

struct slave_item {
    struct slave_item       *next;
    struct slave_item       *prev;
};

struct slave_item* alloc_or_die_slave(void)
{
    struct slave_item *ptr = malloc(sizeof(*ptr));
    if (!ptr)
        abort();

    ptr->next = NULL;
    ptr->prev = NULL;
    return ptr;
}

struct master_item {
    struct master_item      *next;
    struct master_item      *prev;
    struct slave_item       *slave;
};

struct master_item* alloc_or_die_master(void)
{
    struct master_item *ptr = malloc(sizeof(*ptr));
    if (!ptr)
        abort();

    ptr->next = NULL;
    ptr->prev = NULL;
    ptr->slave = NULL;
    return ptr;
}

void dll_insert_slave(struct slave_item **dll)
{
    struct slave_item *item = alloc_or_die_slave();
    struct slave_item *next = *dll;
    item->next = next;
    if (next)
        next->prev = item;

    *dll = item;
}

void* dll_create_generic(void (*insert_fnc)())
{
    void *dll = NULL;
    insert_fnc(&dll);
    insert_fnc(&dll);
    insert_fnc(&dll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        insert_fnc(&dll);

    return dll;
}

struct slave_item* dll_create_slave(void)
{
    return dll_create_generic(dll_insert_slave);
}

void dll_destroy_slave(struct slave_item *dll)
{
    // rewind
    struct slave_item *prev;
    while (dll && (prev = dll->prev))
        dll = prev;

    while (dll) {
        struct slave_item *next = dll->next;
        free(dll);
        dll = next;
    }
}

void dll_destroy_nested_lists(struct master_item *dll)
{
    // rewind
    struct master_item *prev;
    while (dll && (prev = dll->prev))
        dll = prev;

    while (dll) {
        dll_destroy_slave(dll->slave);
        dll->slave = NULL;
        dll = dll->next;
    }
}

void dll_destroy_master(struct master_item *dll)
{
    // rewind
    struct master_item *prev;
    while (dll && (prev = dll->prev))
        dll = prev;

    while (dll) {
        struct master_item *next = dll->next;
        free(dll);
        dll = next;
    }
}

void dll_insert_master(struct master_item **dll)
{
    struct master_item *item = alloc_or_die_master();
    struct master_item *next = *dll;
    item->next = next;
    if (next)
        next->prev = item;

    item->slave = dll_create_slave();
    *dll = item;
}

struct master_item* dll_create_master(void)
{
    return dll_create_generic(dll_insert_master);
}

int main()
{
    struct master_item *dll = dll_create_master();
    __VERIFIER_plot("01");

    dll_destroy_nested_lists(dll);
    __VERIFIER_plot("02");

    // just silence the gc
    dll_destroy_master(dll);
    __VERIFIER_plot("03");

    return 0;
}

/**
 * @file test-0072.c
 *
 * @brief DLL, each node points to a standalone DLL
 *
 * - indirect function call (not so synthetic use case here)
 * - bounded recursion of depth 2
 * - see the function dll_create_generic() for details
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
