#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    int *a,*b,num;

    if (__VERIFIER_nondet_int())
        b = NULL;

    b = (int *) realloc(b, sizeof(int)); // error, if b is undef
    if (b == NULL)
        return 0;

    a = (int *) malloc(sizeof(int));
    if (a == NULL) {
        free(b);
        return 0;
	}

    a[0] = 1;

    int *tmp = (int *) realloc(a, sizeof(int) * 3);
    if (tmp == NULL) {
        free(b);
        return 0; // oom: memory leak
    }
    a = tmp;
    if (a[0] == 1) {
        a[1] = 2;
        a[2] = 3;
    }
    tmp = (int *) realloc(a, sizeof(int) * 2);
    if (tmp == NULL) {
        num = a[2]; // old allocation still valid
    } else {

        if (__VERIFIER_nondet_int())
            num = a[1]; // error: deref to deallocated memory

        a = tmp;
        if (__VERIFIER_nondet_int())
            num = a[2]; // error: invalid deref out of bounds
    }

    free(a);
    b = (int *) realloc(b,0); // size 0 ==> free b
    return 0;
}

/**
 * @file test-0256.c
 *
 * @brief a regression test for realloc()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
 
