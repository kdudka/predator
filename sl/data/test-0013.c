#include <stdlib.h>

void simple_test(void) {
    void *ptr = malloc(sizeof(void *));
    free(ptr);
    ptr = &ptr;
}

struct Item {
    void *p1;
    void *p2;
};

void* alloc_struct(void) {
    struct Item *ptr = malloc(sizeof(*ptr));
    return ptr;
}

void test_complex(void) {
    void *ptr = alloc_struct();
    free(ptr);

    ptr = alloc_struct();
    ptr = NULL;
}

int main() {
    simple_test();
    test_complex();
    return 0;
}
