#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    void *undef;
    void *val = NULL;
    void *ptr = &val;

    switch (__VERIFIER_nondet_int()) {
        case 0:
            free(undef);
            return 0;

        case 1:
            free(val);
            return 0;

        case 2:
            free(ptr);
            return 0;

        default:
            break;
    }

    void *item = malloc(sizeof(void *));
    free(item);
    free(item);

    return 0;
}

/**
 * @file test-0003.c
 *
 * @brief invalid free()
 *
 * - uninitialized value
 * - non-heap object
 * - double free
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
