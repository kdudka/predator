#include <verifier-builtins.h>
#include <stdlib.h>

int main()
{
    union {
        void *p0;

        struct {
            char c[2];
            void *p1;
            void *p2;
        } str;

    } data;

    // alloc 37B on heap
    data.p0 = malloc(37);

    // this should be fine
    data.str.p2 = &data;

    __VERIFIER_plot(NULL);

    // this introduces a memleak
    data.str.c[1] = sizeof data.str.p1;

    // invalid free()
    free(data.p0);

    __VERIFIER_plot(NULL);
    return 0;
}

/**
 * @file test-0158.c
 *
 * @brief memory leak induced by data reinterpretation
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
