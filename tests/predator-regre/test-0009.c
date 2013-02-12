#include <verifier-builtins.h>
#include <stdlib.h>
int main()
{
    void *mem = malloc(sizeof(mem));
    void **ok = mem;

    mem = malloc(sizeof(mem));
    char *warn = mem;

    mem = malloc(sizeof(char));
    void **err = mem;

    __VERIFIER_plot(NULL);
    *err = NULL;

    return 0;
}

/**
 * @file test-0009.c
 *
 * @brief checking of size of allocated memory (the easiest case)
 *
 * - based on type information of the pointer
 * - as soon as the (void *) pointer is assigned to a type-safe
 *   pointer, the size of _target_ type is compared with the size
 *   given to malloc() as the argument
 *
 * - we support only constant parameters of malloc(), known in the
 *   time of program's compilation
 *
 * - we don't support arrays at all (nor static, nor dynamic)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
