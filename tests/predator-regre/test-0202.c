#include <verifier-builtins.h>

int main()
{
    int i = ___sl_get_nondet_int();
    if (i < 0x7 || 0x213 < i)
        return 1;

    ___sl_plot("01-continuous-range", &i);

    i &= ~(sizeof(double) - 1);
    ___sl_plot("02-aligned", &i);
    ___SL_ASSERT(!(i & (sizeof(double) - 1)));

    i &= ~0x6;
    ___sl_plot("03-pseudo-aligned", &i);
    ___SL_ASSERT(!(i & 0x6));

    return 0;
}
