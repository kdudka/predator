#include "include/pt.h"

int **connect(int **param)
{
    int **copy = malloc(sizeof(int **));
    copy = param;

    ___cl_pt_points_loc_y("copy", "d");

    return copy;
}

int main()
{
    int  **  pp;
    int   *   p;
    int       d;

    p = &d;

    // after that the 'pp' pointer should point to 'p'
    pp = connect(&p);

    ___cl_pt_points_loc_y("pp", "p");

    return 0;
}
