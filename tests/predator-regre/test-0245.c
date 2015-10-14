#include <stdlib.h>

int main()
{
    void **pp = alloca(sizeof *pp);
    *pp = malloc(sizeof **pp);
    long l = (long) *pp;
}

/**
 * @file test-0245.c
 *
 * @brief a regression test for leaking memory pointed by alloca() obj
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
