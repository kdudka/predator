#include <stdlib.h>

int main() {
    struct T {
        struct T* next;
    };

    struct T* x = NULL;
    struct T* y = NULL;

    int i;
    for (i = 1; i; ++i) {
        y = malloc(sizeof(*y));
        y->next = x;
        x = y;
    }

    struct T* z = NULL;
    while (x != NULL) {
        y = x;
        x = x->next;
        y->next = z;
        z = y;
    }

    return 0;
}

/**
 * @file test-0083.c
 *
 * @brief an example taken from fa_analysis
 *
 * - seen infinite because we abstract out integral values
 * - it ignores the OOM state
 * - no flaws reported with -fplugin-arg-libsl-symexec-args=fast
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
