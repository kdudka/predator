#include <stdlib.h>

void simple_test(void) {
    // this should be OK
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
    // this should be OK
    void *ptr = alloc_struct();
    free(ptr);

#if 0
    // direct memory leak
    ptr = alloc_struct();
    ptr = NULL;
#endif

    struct Item *item = alloc_struct();
    // try to remove the following condition ;-)
    if (!item)
        abort();

    // indirect memory leak
    item->p1 = malloc(1);
#if 1
    free(item->p1);
#endif
    free(item);
}

int main() {
    simple_test();
    test_complex();
    return 0;
}
