/**
 * little bit more complicated global variable test
 */
#include "include/pt.h"

void *a, *b;

int main()
{
    void *c, *d;

    a = b;

    a = &c;

    c = &d;
    d = &c;


    ___cl_pt_points_glob_y(a, d);

    ___cl_pt_points_glob_y(a, c);
    ___cl_pt_points_glob_y(b, c);

    ___cl_pt_points_glob_y(c, d);
    ___cl_pt_points_glob_y(d, c);

    ___cl_pt_points_glob_n(c, a);
    ___cl_pt_points_glob_n(c, b);
}
