#include "../sl.h"
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

    ___sl_plot(NULL);

    // this introduces a memleak
    data.str.c[1] = sizeof data.str.p1;

    // invalid free()
    free(data.p0);

    ___sl_plot(NULL);
    return 0;
}
