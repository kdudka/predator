/**
 * check for phase 2
 *
 * BindLoc test
 *
 * TODO: better testing possiblities.. (only in particular graph)
 */

#include "include/pt.h"

int *ptrA, *ptrB;

int test()
{
    int A;
    ptrA = &A;
}

int main()
{
    int B;
    ptrB = &B;

    ptrA = ptrB;

    test();
}
