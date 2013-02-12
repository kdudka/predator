#include <stdlib.h>

#include <verifier-builtins.h>

int main()
{
    size_t size = __VERIFIER_nondet_int();
    if (size < 0x10)
        abort();
    if (0x1000 < size)
        abort();

    void *ptr = malloc(size);
    __VERIFIER_plot(NULL, &ptr);

    return 0;
}
