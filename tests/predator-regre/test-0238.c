#include <verifier-builtins.h>
#include <stdlib.h>

int main() {

    struct T {
        struct T* next;
    };

    struct T* x = NULL;
    struct T* y = NULL;

    y = malloc(sizeof(*y));

    free(y);

    x = malloc(sizeof(*x));

    if (x == y)
    {
        ___sl_error("if the second malloc returns the same value as the first, I should get here");
    }

    free(x);

    return 0;
}

/**
 * @file test-0238.c
 *
 * @brief regression test covering a fixed bug in the prover
 *
 * - Predator response was unsound when comparing freed pointers
 * - contributed by Ondra Lengal
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
