#include <stdlib.h>

void* safe_alloc(void) {
    void *ptr = malloc(sizeof(void *));
    if (ptr)
        return ptr;

    abort();
}

int main() {
    // call cache miss
    void *a = safe_alloc();
    free(a);

    // call cache miss
    a = safe_alloc();
    free(a);

    // call cache hit
    void *b = safe_alloc();

    // 05badbb claims there is a 'free() called on uninitialized value' when
    // compiled with SE_BYPASS_CALL_CACHE == 1
    free(b);

    return 0;
}

/**
 * @file test-0043.c
 *
 * @brief regression test focused on call cache
 *
 * - inspired by an already fixed bug there
 * - the comments inside are valid only with SE_DISABLE_SYMCUT
 * - with symcut enabled, we have even more cache hits!
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
