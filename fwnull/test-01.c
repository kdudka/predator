#include <stdlib.h>

void test0(void) {
    void **x = NULL;
    *x = (void *)&x;
}

void test1(void **ptr) {
    if (ptr)
        test0();

    *ptr = NULL;
}

void test2(void **ptr) {
    *ptr = NULL;

    if (ptr)
        test0();
}

void test3(void **ptr) {
    if (!ptr || !*ptr)
        return;

    *ptr = NULL;
}
