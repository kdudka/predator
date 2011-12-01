#include <stdlib.h>

#include <verifier-builtins.h>

int main()
{
    size_t size = ___sl_get_nondet_int();
    if (size < 0x10)
        abort();
    if (0x1000 < 0x10)
        abort();

    void *ptr = malloc(size);
    ___sl_plot(NULL, &ptr);

    return 0;
}
