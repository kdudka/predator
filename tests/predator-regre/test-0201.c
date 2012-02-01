#include <verifier-builtins.h>

int main()
{
    int i = ___sl_get_nondet_int();
    if (i < 0x100 || 0x1000 < i)
        return 1;

    ___sl_plot("01-continuous-range");

    int j = 0x10 * i;
    int k = ~0xFF & i;
    ___sl_plot("02-with-aligment", &i, &j, &k);

    // this should be OK
    ___SL_ASSERT(!(i & 0x0));
    ___SL_ASSERT(!(j & 0xF));
    ___SL_ASSERT(!(k & 0xFF));

    // now trigger some assertion failures
    switch (___sl_get_nondet_int()) {
        case -3:
            ___SL_ASSERT(!(i & 0x1));
            break;

        case -2:
            ___SL_ASSERT(!(j & 0x10));
            break;

        case -1:
            ___SL_ASSERT(!(k & 0x100));
            break;

        case 0:
            ___SL_ASSERT(!(k & ~0xFF));
    }

    i += 16;
    j += 16;
    k += 16;
    ___sl_plot("03-shifted", &i, &j, &k);

    // this should be OK
    ___SL_ASSERT(!(i & 0x0));
    ___SL_ASSERT(!(j & 0xF));
    ___SL_ASSERT(!(k & 0xF));

    if (___sl_get_nondet_int())
        ___SL_ASSERT(!(k & 0xFF));

    ++i, ++j, ++k;
    ___sl_plot("04-badly-shifted", &i, &j, &k);

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
