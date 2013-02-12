#include <verifier-builtins.h>

int main()
{
    int a = 1;
    int b = 2;

    if (a == b)
        return 1;

    // check if Neq predicates are still traversed and plotted correctly
    __VERIFIER_plot("test-0046-a");
    __VERIFIER_plot("test-0046-b");

    return 0;
}

/**
 * @file test-0046.c
 *
 * @brief visualization of Neq predicates
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
