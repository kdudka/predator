#include "include/pt.h"

/**
 * Basic operation --> take an address of a variable.
 */

int main(int argc, char *argv)
{
    int  **  pp;
    int   *   p;
    int       d;

    p = &d;

    ___cl_pt_points_glob_y(p,  d);
    ___cl_pt_points_glob_n(pp, p);
    ___cl_pt_points_glob_n(d, p);
    ___cl_pt_points_glob_n(d, pp);

    ___cl_pt_is_pointed_y(d);
    ___cl_pt_is_pointed_n(p);
    ___cl_pt_is_pointed_n(pp);
}
