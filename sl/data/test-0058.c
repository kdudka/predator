// Creating and then traversing a DLL.

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
    pi->prev = end;
    end->next = pi;
    return pi;
}

struct item* create_dll(void)
{
    struct item *dll = alloc_and_zero();
    struct item *pi = create_item(dll);
    pi = create_item(pi);
    pi = create_item(pi);
    pi = create_item(pi);
    pi = create_item(pi);
    pi = create_item(pi);
    return dll;
}

struct item* fast_forward_core(struct item *dll)
{
    struct item *next;
    while ((next = dll->next)) {
        //___sl_plot("00-ff-one-step");
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
    ___sl_plot("01-before-fast-forward");

    fast_forward(&dll);
    ___sl_plot("02-after-fast-forward");

    return 0;
}
