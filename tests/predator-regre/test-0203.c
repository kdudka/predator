#include <verifier-builtins.h>

int main()
{
    int i = __VERIFIER_nondet_int();
    if (i < 1) {
        __VERIFIER_plot("less-than-one", &i);
        __VERIFIER_assert(i < 1);
    }
    else if (1 < i) {
        __VERIFIER_plot("more-than-one", &i);
        __VERIFIER_assert(1 < i);
    }
    else {
        __VERIFIER_plot("equal-to-one", &i);
        __VERIFIER_assert(1 == i);
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
