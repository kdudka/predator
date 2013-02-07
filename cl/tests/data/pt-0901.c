#include "include/pt.h"

/**
 * Basic function parameter (local) biding.
 */

int test(int **p)
{
    int i = 10;
    // Note that we are accessing local variable!  This is just for points-to
    // test purposes.
    *p = &i;

    ___cl_pt_is_pointed_y(i);
    ___cl_pt_is_pointed_n(p);
}


int main(int argc, char **argv)
{
    void *argcP = &argc;
    void *argvP = &argv;

    ___cl_pt_points_glob_y(argvP, argv);
    ___cl_pt_points_glob_y(argcP, argc);

    ___cl_pt_is_pointed_y(argv);
    ___cl_pt_is_pointed_y(argc);
}
