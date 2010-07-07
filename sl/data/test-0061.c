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
    ___SL_PLOT_STACK_FRAME(sll_to_dll, "b00");
    while (dll && dll->next) {
        ___SL_PLOT_STACK_FRAME(sll_to_dll, "b01");
        struct item *prev = dll;
        ___SL_PLOT_STACK_FRAME(sll_to_dll, "b02");
        dll = dll->next;
        ___SL_PLOT_STACK_FRAME(sll_to_dll, "b03");
        dll->prev = prev;
        ___SL_PLOT_STACK_FRAME(sll_to_dll, "b04");
    }
    ___SL_PLOT_STACK_FRAME(sll_to_dll, "b05");
}

struct item* dll_to_sll(struct item *dll)
{
    ___SL_PLOT_STACK_FRAME(dll_to_sll, "n00");
    while (dll && dll->next) {
        ___SL_PLOT_STACK_FRAME(dll_to_sll, "n01");
        struct item *next = dll->next;
        ___SL_PLOT_STACK_FRAME(dll_to_sll, "n02");
        dll->next = NULL;
        ___SL_PLOT_STACK_FRAME(dll_to_sll, "n03");
        dll = next;
        ___SL_PLOT_STACK_FRAME(dll_to_sll, "n04");
    }
    ___SL_PLOT_STACK_FRAME(dll_to_sll, "n05");
    return dll;
}

int main()
{
    // create a SLL
    struct item *dll = create_sll();
    ___sl_plot_by_ptr(&dll, "01-sll-ready");

    // convert the SLL to DLL by completing the 'prev' field
    sll_to_dll(dll);
    ___sl_plot_by_ptr(&dll, "02-dll-got-from-sll");

    // convert the DLL to SLL by zeroing the 'next' field
    dll = dll_to_sll(dll);
    ___sl_plot_by_ptr(&dll, "03-sll-got-from-dll");

    // finally just destroy the list to silence our garbage collector
    while (dll) {
        struct item *prev = dll->prev;
        free(dll);
        dll = prev;
    }

    ___sl_plot("04-done");

    return 0;
}
