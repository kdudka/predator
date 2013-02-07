#include "include/pt.h"

/**
 * Make a chain of pointers like: ppp -> pp -> p -> d
 */

int main()
{
    int     *   p;
    int    **  pp;
    int   *** ppp;
    int         d;

    ppp = &pp;
    pp  =  &p;
    p   =  &d;

    ___cl_pt_points_glob_y(ppp, d);
    ___cl_pt_points_glob_y(pp,  d);
    ___cl_pt_points_glob_y(p,   d);

    ___cl_pt_points_glob_n(d,   p);
    ___cl_pt_points_glob_n(p,   pp);
    ___cl_pt_points_glob_n(pp,  ppp);

    ___cl_pt_is_pointed_n(ppp);
    ___cl_pt_is_pointed_y(pp);
    ___cl_pt_is_pointed_y(p);
    ___cl_pt_is_pointed_y(d);
}
