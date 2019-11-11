#include <stdlib.h>

int main()
{
    int **ppi = malloc(sizeof *ppi);
    *ppi = malloc(sizeof **ppi);
    ppi = realloc(ppi, 2 * sizeof *ppi);
    free(*ppi);
    free(ppi);
}

/**
 * @file test-0261.c
 *
 * @brief avoid memory leak false positive with realloc()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
