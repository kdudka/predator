#include "include/pt.h"

/**
 * Basic function parameter (local) biding.
 */

int test(int *p)
{
    int i = 10;
    // Note that we are accessing local variable!  This is just for points-to
    // test purposes.

    *p = &i;
}


int main(int argc, char **argv)
{
    int i, j;

    i = &j;

    // ___cl_pt_points_glob_y(i, j);

    // ___cl_pt_is_pointed_y(j);
    // ___cl_pt_is_pointed_n(i);

    ___cl_pt_build_fail();
}
