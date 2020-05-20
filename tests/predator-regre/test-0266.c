#include <stdlib.h>

int main()
{
    void *ptr = malloc(1U);
    if (!ptr)
        return EXIT_FAILURE;

    void *tmp = realloc(ptr, 0);
    if (tmp)
        free(tmp);

    free(tmp);
}

/**
 * @file test-0266.c
 *
 * @brief a regression test for simulation of realloc(ptr, 0)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
