#include <verifier-builtins.h>
#include <stdlib.h>

void* fnc(void) {
    void *var;
    void *ptr = &var;
    return ptr;
}

int main()
{
    void *ptr = fnc();
    if (__VERIFIER_nondet_int())
        free(ptr);

    struct { void *v1; void *v2; } *s = malloc(sizeof(*s));
    if (s) {
        free(&s->v1);
        free(&s->v2);
    }

    return 0;
}

/**
 * @file test-0027.c
 *
 * @brief regression test for detection of some errors on call of free()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
