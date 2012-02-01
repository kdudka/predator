#include <verifier-builtins.h>

int main()
{
    int i = ___sl_get_nondet_int();
    if (i < 0)
        ___sl_plot("less-than-zero", &i);
    else if (0 < i)
        ___sl_plot("more-than-zero", &i);
    else {
        ___sl_plot("equal-to-zero", &i);
        ___SL_ASSERT(!i);
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
