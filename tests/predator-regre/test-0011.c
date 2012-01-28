#include <stdlib.h>

int main() {
    // unchecked result of malloc --> possible dereference of NULL
    void **v = malloc(sizeof(*v));
    *v = NULL;

    v = malloc(sizeof(*v));
    if (v)
        // this should be OK
        *v = NULL;

    return 0;
}

/**
 * @file test-0011.c
 *
 * @brief trivial test-case focused on OOM states handling
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
