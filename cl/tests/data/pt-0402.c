#include "include/pt.h"

/**
 * Regression test -- this should create two self-pointing nodes + one pointing
 * to the second self-pointing one.
 */

int main()
{
    int i;
    void *p;
    void *p1, *p2;

    if (i) {
        p1 = &p1;
    }
    else {
        p2 = &p2;
    }

    p = &p2;

    ___cl_pt_is_pointed_n(p1);
    ___cl_pt_is_pointed_y(p2);
}
