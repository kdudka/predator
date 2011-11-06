#include <stdlib.h>

int main()
{
    void **ptr = malloc(sizeof *ptr);
    free(ptr);
    ptr = malloc(sizeof *ptr);
}
