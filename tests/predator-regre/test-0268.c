#include <verifier-builtins.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int *p = malloc(sizeof(int));

    if (__VERIFIER_nondet_int()) {
label:
        abort();
    }

    free(p);
    return 0;
}

/**
 * @file test-0268.c
 *
 * @brief regression test for empty block before aborted block
 *
 * - contributed by Tomas Jasek
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
