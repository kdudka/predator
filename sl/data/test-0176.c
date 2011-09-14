#include "../sl.h"
#include <stdlib.h>

void create_nonsense(void ***pptr)
{
    *pptr = NULL;

    while (___sl_get_nondet_int()) {
        void **data = *pptr;
        *pptr = malloc(sizeof **pptr);
        **pptr = data;
    }
}

int main()
{
    void **ptr;
    create_nonsense(&ptr);
    ___sl_plot(NULL, &ptr);

    while (ptr) {
        void *next = *ptr;
        free(ptr);
        ptr = next;
    }

    return 0;
}
