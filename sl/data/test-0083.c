#include <stdlib.h>

int main() {
    struct T {
        struct T* next;
    };

    struct T* x = NULL;
    struct T* y = NULL;

    int i;
    for (i = 1; i; ++i) {
        y = malloc(sizeof(*y));
        y->next = x;
        x = y;
    }

    struct T* z = NULL;
    while (x != NULL) {
        y = x;
        x = x->next;
        y->next = z;
        z = y;
    }

    return 0;
}
