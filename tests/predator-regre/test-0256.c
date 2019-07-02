#include <verifier-builtins.h>

int main()
{
    const char i = (long) "XXX";
    __VERIFIER_plot("lost", &i);
}

/**
 * @file test-0256.c
 *
 * @brief test for handling of pointers to string literals
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
