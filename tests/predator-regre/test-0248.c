#include <string.h>
#include <stdlib.h>

char a[sizeof(int *)];

void foo(void)
{
    int *p = (int *) malloc(10);
#if 1
    memcpy(a, &p, sizeof p);
#else
    *((int **) &a) = p;
#endif
}

int main(void)
{
    foo();
    void *p;
    memcpy(&p, a, sizeof p);
    free(p);
    return 0;
}

/**
 * @file test-0248.c
 *
 * @brief reinterpretation PTR<-->INT of equal size
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
