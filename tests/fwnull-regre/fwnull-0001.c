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

struct str {
    int num;
};

int test4(struct str *c)
{
    if ((NULL == c) || (0 == c->num))
        return 0;

    if (c)
        return 1;

    return 0;
}
