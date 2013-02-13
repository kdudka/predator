#include <verifier-builtins.h>
#include <stdlib.h>

int main()
{
    void *x;
    void *y;
    void *z;

    if (x != z)
        y = z;

    __VERIFIER_plot(NULL);

    return 0;
}

/**
 * @file test-0126.c
 *
 * @brief trivial test-case focused on three-way join of unknown values
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
