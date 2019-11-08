#include <stdlib.h>

int main()
{
    malloc(sizeof(int));
    realloc(NULL, sizeof(int));
    free(realloc(malloc(sizeof(int)), sizeof(int)));
}

/**
 * @file test-0260.c
 *
 * @brief report memory leak if return value of *alloc() is not used
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
