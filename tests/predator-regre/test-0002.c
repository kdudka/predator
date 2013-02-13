#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    void **undef;
    void **null_value = NULL;

    if (__VERIFIER_nondet_int())
        null_value = *undef;

    void **err = *null_value;

    return 0;
}

/**
 * @file test-0002.c
 *
 * @brief invalid dereference
 *
 * - NULL value
 * - an uninitialized value
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
