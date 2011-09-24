#include "../sl.h"
#include <stdlib.h>

int main()
{
    // define a user type
    struct T {
        struct T *next;
        struct T *prev;
    };

    // allocate an instance of it
    struct T *null = (struct T *)0;
    struct T *data = malloc(sizeof *data);
    if (null == data)
        // OOM
        return EXIT_FAILURE;

    // now introduce some generic pointers
    void *const pd = data;
    void *const i0 = &data->next;
    void *const i1 = &data->prev;

    if (!pd || !i0 || !i1)
        // something went wrong, yell
        null->prev = i0;

    if (i0 == i1)
        // something went wrong, yell
        null->prev = i0;

    if (pd == i1)
        // something went wrong, yell
        null->next = i1;

    if (pd != i0)
        // something went wrong, yell
        null->next = pd;

    if (___sl_get_nondet_int())
        // this is always bad idea
        free(i1);

    // this is ugly, but it works :-)
    free(i0);

    // this should be considered as double-free
    free(pd);

    return EXIT_SUCCESS;
}
