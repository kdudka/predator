#include <stdlib.h>

void foo(void) {
    void **x = NULL;
    *x = (void *)&x;
}
