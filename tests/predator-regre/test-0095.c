#include <verifier-builtins.h>
#include <stdbool.h>
#include <stdlib.h>

union data {
    long number;

    struct {
        union {
            struct {
                bool b;
            } s0;
            struct {
                int i;
            } s1;
        } u0;
        union {
            struct {
                char c;
            } s0;
            struct {
                void *p;
            } s1;
        } u1;
    };
};

int main()
{
    union data d = {
        .number = 0L
    };
    __VERIFIER_plot(NULL);

    d.u0.s0.b = true;
    __VERIFIER_plot(NULL);

    // infinite loop to detect malfunction
    while (!d.number || !d.u0.s1.i || d.u1.s0.c || d.u1.s1.p)
        // cause "warning: end of function main() has not been reached" in case
        // something goes wrong
        ;

    return EXIT_SUCCESS;
}

/**
 * @file test-0095.c
 *
 * @brief regression test focused on struct/union nesting
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
