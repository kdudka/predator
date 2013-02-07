#include <verifier-builtins.h>
#include "include/vk.h"

#include <stdlib.h>

int d = 0;

void killPtrTarget(int **ptr) {
    *ptr = &d;
}

int main()
{
    int d = 10;
    int *p = &d;

    killPtrTarget(&p);

    return EXIT_SUCCESS;
}
