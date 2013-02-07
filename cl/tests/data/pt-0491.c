#include "include/pt.h"

/**
 * Regression test -- this should create one self-pointing node
 */

int main()
{
    int i;
    void *p1, *p2, *p3;

    if (i) {
        p1 = &p2;
        p2 = &p3;
        p3 = &p1;
    }
    else {
        p1 = &p1;
        p2 = &p2;
        p3 = &p3;
    }

    ___cl_pt_points_glob_y(p1, p1);
    ___cl_pt_points_glob_y(p3, p1);
    ___cl_pt_points_glob_y(p2, p1);
}
