#include "include/pt.h"

/**
 * function call that forces the caller's pointer with its following nodes to be
 * joined into one node.
 */

int test(void *p)
{
    p = &p;
}


int main()
{
    int  *** ppp;
    int   **  pp;
    int    *   p;
    int        d;

    ppp = &pp;
    pp  =  &p;
    p   =  &d;

    test(ppp);

    ___cl_pt_points_glob_y(d, pp);

    // this check is disabled --> it modifies PT-graph
    // ___cl_pt_points_glob_y(d, ppp);
    ___cl_pt_is_pointed_y(d);
}
