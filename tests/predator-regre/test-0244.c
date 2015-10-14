#include <verifier-builtins.h>

extern void *y;
void *x = &y;
void *y = &x;

int main()
{
    __VERIFIER_assert(x == &y);
    __VERIFIER_assert(y == &x);
}

/**
 * @file test-0244.c
 *
 * @brief cyclic dependency of gl var initializers
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
