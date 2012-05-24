/**
 * check for phase 2
 *
 * join the function parameter with global variable based on called function
 * (more complicated example than pt-0802.c -- there is assigned into
 * dereferenced operand)
 */

#include "include/pt.h"

int *ptrGlob; // global variable

void test(int *ptrTest)
{
    ptrGlob = ptrTest;
}


int main()
{
    int d;
    int *ptrMain = &d;

    // if there was not following instruction there wouldn't exist 'ptrGlob'
    // node in 'main' PT-graph.
    test(ptrMain);

    ___cl_pt_points_glob_y(ptrGlob, d);
}
