#include <stdlib.h>

int main() {
    void **ptr = NULL;

    while (1) {
        void **data = ptr;
        ptr = malloc(sizeof ptr);
        free(ptr);
        pt = data;
    }

    return 0;
}
