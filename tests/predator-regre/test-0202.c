#include <verifier-builtins.h>

int main()
{
    int i = __VERIFIER_nondet_int();
    if (i < 0x7 || 0x213 < i)
        return 1;

    __VERIFIER_plot("01-continuous-range", &i);

    i &= ~(sizeof(double) - 1);
    __VERIFIER_plot("02-aligned", &i);
    __VERIFIER_assert(!(i & (sizeof(double) - 1)));

    i &= ~0x6;
    __VERIFIER_plot("03-pseudo-aligned", &i);
    __VERIFIER_assert(!(i & 0x6));

    return 0;
}

/**
 * @file test-0202.c
 *
 * @brief a tiny regression test for alignment of integral ranges
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
