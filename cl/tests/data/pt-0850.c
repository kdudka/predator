/**
 * check for phase 2
 *
 * join two globals by main
 *
 * TODO: better testing possiblities.. (only in particular graph)
 */

#include "include/pt.h"

int *ptrA, *ptrB, *ptrC;

int main()
{
    ptrA = ptrB;
    ptrB = &ptrC;
}
