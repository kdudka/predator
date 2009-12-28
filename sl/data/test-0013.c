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

    // direct memory leak
    ptr = alloc_struct();
#if 0
    ptr = NULL;
#endif
    free(ptr);

    struct Item *item = alloc_struct();
    // try to remove the following condition ;-)
#if 1
    if (!item)
        abort();
#endif

    // indirect memory leak
    item->p1 = malloc(1);
    item->p2 = malloc(2);
#if 1
    free(item->p1);
    free(item->p2);
#endif
    free(item);

    while (1) {
        void *anon = malloc(80);
#if 1
        free(anon);
#endif
    }
}

int main() {
    simple_test();
    test_complex();
    return 0;
}
