/**
 * check for phase 2
 *
 * join the function parameter with global variable based on called function
 */

#include "include/pt.h"

int *ptrGlob; // global variable

void test(int **ptr)
{
    *ptr = ptrGlob;
}

int main()
{
    int **ptrMain;
    int  *p;
    int   d;

    ptrMain = &p;
    p = &d;

    // if there was not following instruction there wouldn't exist 'ptrGlob'
    // node in 'main' PT-graph.
    test(ptrMain);

    ___cl_pt_points_glob_y(ptrMain, d);
}
