#include <stdlib.h>

int main() {
    void **ptr = NULL;

    while (1) {
        void **data = ptr;
        ptr = malloc(sizeof ptr);
        free(ptr);
        ptr = data;
    }

    return 0;
}
