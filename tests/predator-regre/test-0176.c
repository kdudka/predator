#include <verifier-builtins.h>
#include <stdlib.h>

void create_nonsense(void ***pptr)
{
    *pptr = NULL;

    while (__VERIFIER_nondet_int()) {
        void **data = *pptr;
        *pptr = malloc(sizeof **pptr);
        **pptr = data;
    }
}

int main()
{
    void **ptr;
    create_nonsense(&ptr);
    __VERIFIER_plot(NULL, &ptr);

    while (ptr) {
        void *next = *ptr;
        free(ptr);
        ptr = next;
    }

    return 0;
}

/**
 * @file test-0176.c
 *
 * @brief a possibly infinite SLL where nodes are just void pointers
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
