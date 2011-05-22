#include "../sl.h"
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
    ___sl_plot(NULL);

    d.u0.s0.b = true;
    ___sl_plot(NULL);

    // infinite loop to detect malfunction
    while (!d.number || !d.u0.s1.i || d.u1.s0.c || d.u1.s1.p)
        // cause "warning: end of function main() has not been reached" in case
        // something goes wrong
        ;

    return EXIT_SUCCESS;
}
