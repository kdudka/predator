// Create a SLL based on the node structure of DLL (only next is used). 
// Then traverse the list while setting values of prev (thus creating a DLL). 
// Finally, traverse the list while resetting next (thus creating a reversed SLL).

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

struct item* create_item(struct item *end)
{
    struct item *pi = alloc_and_zero();
    pi->next = end;
    return pi;
}

struct item* create_sll(void)
{
    struct item *sll = create_item(NULL);
    sll = create_item(sll);
    sll = create_item(sll);
    sll = create_item(sll);
    sll = create_item(sll);
    sll = create_item(sll);
    return sll;
}

void sll_to_dll(struct item *dll)
{
    __VERIFIER_plot("b00");
    while (dll && dll->next) {
        __VERIFIER_plot("b01");
        struct item *prev = dll;
        __VERIFIER_plot("b02");
        dll = dll->next;
        __VERIFIER_plot("b03");
        dll->prev = prev;
        __VERIFIER_plot("b04");
    }
    __VERIFIER_plot("b05");
}

struct item* dll_to_sll(struct item *dll)
{
    __VERIFIER_plot("n00");
    while (dll && dll->next) {
        __VERIFIER_plot("n01");
        struct item *next = dll->next;
        __VERIFIER_plot("n02");
        dll->next = NULL;
        __VERIFIER_plot("n03");
        dll = next;
        __VERIFIER_plot("n04");
    }
    __VERIFIER_plot("n05");
    return dll;
}

int main()
{
    // create a SLL
    struct item *dll = create_sll();
    __VERIFIER_plot("01-sll-ready");

    // convert the SLL to DLL by completing the 'prev' field
    sll_to_dll(dll);
    __VERIFIER_plot("02-dll-got-from-sll");

    // convert the DLL to SLL by zeroing the 'next' field
    dll = dll_to_sll(dll);
    __VERIFIER_plot("03-sll-got-from-dll");

    // finally just destroy the list to silence our garbage collector
    while (dll) {
        struct item *prev = dll->prev;
        free(dll);
        dll = prev;
    }

    __VERIFIER_plot("04-done");

    return 0;
}

/**
 * @file test-0061.c
 *
 * @brief conversion of SLL to DLL and vice versa
 *
 *     1. creates a singly-linked list, using the 'next' selector
 *        for biding
 *
 *     2. goes through the list and completes the missing values
 *        of 'prev' selector, in order to obtain a doubly-linked
 *        list
 *
 *     3. goes through the list and zero the 'next' selector
 *        of each node, in order to get a reversed singly-linked
 *        list
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
