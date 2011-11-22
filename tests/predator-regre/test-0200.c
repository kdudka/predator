#include <stdlib.h>

#include <verifier-builtins.h>

typedef int (*TMain)(int, char *[]);

int main(int argc, char *argv[])
{
    TMain *const ptr = malloc(sizeof *ptr);
    if (!*ptr)
        ___sl_plot("00-null", &ptr);
    else
        ___sl_plot("01-not-null", &ptr);

    ___sl_plot("02-joint", &ptr);

    if (*ptr == &main)
        ___sl_plot("03-main", &ptr);
    else
        ___sl_plot("04-not-main", &ptr);

    ___sl_plot("05-joint", &ptr);

    free(ptr);
    return 0;
}
