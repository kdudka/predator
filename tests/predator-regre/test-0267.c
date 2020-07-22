#include <stdlib.h>

int main(void)
{
    void *ptr1 = malloc(sizeof(char));
    if (ptr1 == NULL)
        return EXIT_SUCCESS;

    free(ptr1);
    void *ptr2 = realloc(ptr1, 2 * sizeof(char)); /* error */
}

/**
 * @file test-0267.c
 *
 * @brief a regression test for realloc of an invalid pointer
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
