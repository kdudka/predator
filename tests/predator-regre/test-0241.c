#include <stdlib.h>

int main()
{
    void *p = malloc(sizeof p);
    long l = (long) p;
}

/**
 * @file test-0241.c
 *
 * @brief a regression test for leaking memory pointed by int variables
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
