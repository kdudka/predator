#include "include/pt.h"

int test(void *ptr1, void *ptr2) {
    ptr1 = &ptr2;

    return 0;
}

int main(int argc, char *argv)
{
    int  **  pp;
    int   *   p;
    int       d;

    p = &d;

    test(pp, p);

    ___cl_pt_points_glob_y(pp, d);
    ___cl_pt_points_glob_n(pp, p);
}
