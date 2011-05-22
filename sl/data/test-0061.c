// Create a SLL based on the node structure of DLL (only next is used). 
// Then traverse the list while setting values of prev (thus creating a DLL). 
// Finally, traverse the list while resetting next (thus creating a reversed SLL).

#include "../sl.h"
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
    ___sl_plot("b00");
    while (dll && dll->next) {
        ___sl_plot("b01");
        struct item *prev = dll;
        ___sl_plot("b02");
        dll = dll->next;
        ___sl_plot("b03");
        dll->prev = prev;
        ___sl_plot("b04");
    }
    ___sl_plot("b05");
}

struct item* dll_to_sll(struct item *dll)
{
    ___sl_plot("n00");
    while (dll && dll->next) {
        ___sl_plot("n01");
        struct item *next = dll->next;
        ___sl_plot("n02");
        dll->next = NULL;
        ___sl_plot("n03");
        dll = next;
        ___sl_plot("n04");
    }
    ___sl_plot("n05");
    return dll;
}

int main()
{
    // create a SLL
    struct item *dll = create_sll();
    ___sl_plot("01-sll-ready");

    // convert the SLL to DLL by completing the 'prev' field
    sll_to_dll(dll);
    ___sl_plot("02-dll-got-from-sll");

    // convert the DLL to SLL by zeroing the 'next' field
    dll = dll_to_sll(dll);
    ___sl_plot("03-sll-got-from-dll");

    // finally just destroy the list to silence our garbage collector
    while (dll) {
        struct item *prev = dll->prev;
        free(dll);
        dll = prev;
    }

    ___sl_plot("04-done");

    return 0;
}
