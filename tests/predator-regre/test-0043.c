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
