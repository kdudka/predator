#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    int *a,num;

    a = (int *) realloc(NULL,sizeof(int));
    if (a == NULL) 
        return 0;
    __VERIFIER_plot("1-realloc");

    if (a[0]) // track_uninit: uninitialized value
        num = 0;
    if (__VERIFIER_nondet_int())
        a[0] = 1;

    int *tmp = (int *) realloc(a, sizeof(int) * 3);
    if (tmp == NULL)
        return 1; // oom: memory leak

    a = tmp;
    __VERIFIER_plot("2-realloc");

    if (a[0]) // track_uninit: possibly uninitialized value
        num = 0;

    if (a[1]) // track_uninit: uninitialized value
        num = 1;

    free(a);
    return num;
}

/**
 * @file test-0257.c
 *
 * @brief a regression test for uninitialized realloc()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
 