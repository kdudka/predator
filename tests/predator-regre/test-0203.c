#include <verifier-builtins.h>

int main()
{
    int i = ___sl_get_nondet_int();
    if (i < 1) {
        ___sl_plot("less-than-one", &i);
        ___SL_ASSERT(i < 1);
    }
    else if (1 < i) {
        ___sl_plot("more-than-one", &i);
        ___SL_ASSERT(1 < i);
    }
    else {
        ___sl_plot("equal-to-one", &i);
        ___SL_ASSERT(1 == i);
    }

    return 0;
}

/**
 * @file test-0203.c
 *
 * @brief similar to test-0198, but uses values close to one
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
