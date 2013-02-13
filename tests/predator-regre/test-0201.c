#include <verifier-builtins.h>

int main()
{
    int i = __VERIFIER_nondet_int();
    if (i < 0x100 || 0x1000 < i)
        return 1;

    __VERIFIER_plot("01-continuous-range");

    int j = 0x10 * i;
    int k = ~0xFF & i;
    __VERIFIER_plot("02-with-aligment", &i, &j, &k);

    // this should be OK
    __VERIFIER_assert(!(i & 0x0));
    __VERIFIER_assert(!(j & 0xF));
    __VERIFIER_assert(!(k & 0xFF));

    // now trigger some assertion failures
    switch (__VERIFIER_nondet_int()) {
        case -3:
            __VERIFIER_assert(!(i & 0x1));
            break;

        case -2:
            __VERIFIER_assert(!(j & 0x10));
            break;

        case -1:
            __VERIFIER_assert(!(k & 0x100));
            break;

        case 0:
            __VERIFIER_assert(!(k & ~0xFF));
    }

    i += 16;
    j += 16;
    k += 16;
    __VERIFIER_plot("03-shifted", &i, &j, &k);

    // this should be OK
    __VERIFIER_assert(!(i & 0x0));
    __VERIFIER_assert(!(j & 0xF));
    __VERIFIER_assert(!(k & 0xF));

    if (__VERIFIER_nondet_int())
        __VERIFIER_assert(!(k & 0xFF));

    ++i, ++j, ++k;
    __VERIFIER_plot("04-badly-shifted", &i, &j, &k);

    return 0;
}

/**
 * @file test-0201.c
 *
 * @brief a regression test for alignment of integral ranges
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
