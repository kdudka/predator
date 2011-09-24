#include "../sl.h"
#include <stdlib.h>

void* fnc(void) {
    void *var;
    void *ptr = &var;
    return ptr;
}

int main()
{
    void *ptr = fnc();
    if (___sl_get_nondet_int())
        free(ptr);

    struct { void *v1; void *v2; } *s = malloc(sizeof(*s));
    if (s) {
        free(&s->v1);
        free(&s->v2);
    }

    return 0;
}
