#include "include/pt.h"

/**
 * Basic function parameter (local) biding.
 */

int test(int **p1, int **p2)
{
    *p1 = *p2;
}


int main()
{
    int ** pp1;
    int ** pp2;
    int  *  p1;
    int  *  p2;
    int      d;

    pp1 = &p1;
    p1   =  &d;

    pp2 = &p2;

    // p2 is unassigned before this call!
    test(pp1, pp2);

    ___cl_pt_points_glob_y(pp2, d);
    ___cl_pt_points_glob_y(p2, d);
}
