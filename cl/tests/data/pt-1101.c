#include <verifier-builtins.h>
#include "include/pt.h"

extern int *j;
extern int *k;

void touch()
{
    int a, b;
    j = &a;
    k = &b;
}

int main()
{
    int i = 0;

    touch();

    // this should never create black-hole because it is white-listed function
    char *noBlackHole = malloc(10);

    j = &i;
}
