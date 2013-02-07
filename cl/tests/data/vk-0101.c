#include <verifier-builtins.h>
#include "include/vk.h"

#include <stdlib.h>

int *touch(int *ptr)
{
    return ptr;
}

void killVariable(int **ptrArg) {

    int *remember = *ptrArg;;

    remember = touch(remember);

    *ptrArg = remember;
}

int main()
{
    int d = 10;
    int *p = &d;

    VK_ASSERT(VK_LIVE, &d); // because it is pointed

    killVariable(&p);

    d = *p;

    return EXIT_SUCCESS;
}
