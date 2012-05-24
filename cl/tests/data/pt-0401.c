#include "include/pt.h"

/**
 * void 4-loop
 */

int main(int argc, char *argv)
{
    void *a, *b, *c, *d;

    a = &b;
    b = &c;
    c = &d;
    d = &a;

    ___cl_pt_points_glob_y(d, a);
    ___cl_pt_points_glob_y(d, b);
    ___cl_pt_points_glob_y(d, c);

    ___cl_pt_is_pointed_y(a);
    ___cl_pt_is_pointed_y(b);
    ___cl_pt_is_pointed_y(c);
    ___cl_pt_is_pointed_y(d);
}
