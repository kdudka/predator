#include <stdlib.h>

int main()
{
    void *mem = malloc(sizeof(mem));
    void **ok = mem;

    mem = malloc(sizeof(mem));
    char *warn = mem;

    mem = malloc(sizeof(char));
    void **err = mem;
}
