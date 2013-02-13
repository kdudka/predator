#include <stdlib.h>

#include <verifier-builtins.h>

typedef int (*TMain)(int, char *[]);

int main(int argc, char *argv[])
{
    TMain *const ptr = malloc(sizeof *ptr);
    if (!*ptr)
        __VERIFIER_plot("00-null", &ptr);
    else
        __VERIFIER_plot("01-not-null", &ptr);

    __VERIFIER_plot("02-joint", &ptr);

    if (*ptr == &main)
        __VERIFIER_plot("03-main", &ptr);
    else
        __VERIFIER_plot("04-not-main", &ptr);

    __VERIFIER_plot("05-joint", &ptr);

    free(ptr);
    return 0;
}
