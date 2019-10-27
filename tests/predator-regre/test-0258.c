#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    int num;
    if (__VERIFIER_nondet_int())
        num = 2;
    else
        num = 5;

    int *a = (int *) malloc(sizeof(int) * num);
    if (a == NULL) 
        return 0;
    int *b = &(a[1]);
    int *tmp = (int *) realloc(a, sizeof(int) * 3);
    if (tmp == NULL) {
        free(a);
        return 0;
    }
    if (a != tmp)
        tmp[1] = 2;
    else 
        *b = 3; // valid
    free(tmp);
    return 0;
}

/**
 * @file test-0258.c
 *
 * @brief a regression test for reused memory by realloc()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
 