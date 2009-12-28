#include <stdlib.h>

void simple_test(void) {
    void *ptr = malloc(sizeof(void *));
    free(ptr);
    ptr = &ptr;
}

void* alloc_struct(void) {
    struct {
        void *p1;
        void *p2;
    } *ptr = malloc(16);
    return ptr;
}

void test_complex(void) {
    void *ptr = alloc_struct();
    ptr = NULL;
}

int main() {
    //simple_test();
    test_complex();
    return 0;
}
