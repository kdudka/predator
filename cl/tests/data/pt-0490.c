#include "include/pt.h"

/**
 * Regression test -- this should create one self-pointing node and two
 * nodes containing temporary variable pointing to this also.
 */

int main()
{
    void *p1, *p2, *p3;

    if (p1) {
        p1 = &p1;
        p2 = &p2;
    }

    if (p2) {
        p1 = &p3;
        p2 = &p1;
        p3 = &p2;
    }

    ___cl_pt_points_glob_y(p1, p2);
    ___cl_pt_points_glob_y(p2, p2);
    ___cl_pt_points_glob_y(p1, p3);
    ___cl_pt_points_glob_y(p3, p2);

    ___cl_pt_is_pointed_y(p1);
    ___cl_pt_is_pointed_y(p2);
    ___cl_pt_is_pointed_y(p3);
}
