#include <stdlib.h>

int main()
{
    void **ptr = malloc(sizeof *ptr);
    free(ptr);
    ptr = malloc(sizeof *ptr);
    return 0;
}

/**
 * @file test-0185.c
 *
 * @brief a regression test focused on a fixed SIGSEGV bug in SymProc
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
