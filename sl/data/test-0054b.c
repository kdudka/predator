// Creating a DLL from the middle, then going to the beginning (wrt. next),
// deleting the DLL.

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

struct item* create_dll(void)
{
    struct item *dll = alloc_and_zero();
    struct item *new;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        // Insert an element behind dll
        new = alloc_and_zero();
        new->next = dll->next;
        new->prev = dll;
        if (new->next) new->next->prev = new;
        dll->next = new;
        // Insert an element in front of dll
        new = alloc_and_zero();
        new->next = dll;
        new->prev = dll->prev;
        if (new->prev) new->prev->next = new;
        dll->prev = new;
    }

    return dll;
}

struct item *destroy_dll(struct item *dll) 
{
    if (!dll) return dll;

    while (dll->prev) dll=dll->prev;

    while (dll) {
        struct item *next = dll->next;
        free(dll);
        dll = next;
    }

    return dll;
} 

int main()
{
    struct item *dll = create_dll();

    ___sl_plot("dll_midlle_created");

    dll = destroy_dll(dll);

    ___sl_plot("dll_middle_deleted");

    return 0;
}
