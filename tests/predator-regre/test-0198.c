#include <verifier-builtins.h>

int main()
{
    int i = __VERIFIER_nondet_int();
    if (i < 0)
        __VERIFIER_plot("less-than-zero", &i);
    else if (0 < i)
        __VERIFIER_plot("more-than-zero", &i);
    else {
        __VERIFIER_plot("equal-to-zero", &i);
        __VERIFIER_assert(!i);
    }

    return 0;
}

/**
 * @file test-0198.c
 *
 * @brief a regression test for reasoning about integral ranges
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
