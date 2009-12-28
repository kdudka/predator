#include <stdlib.h>

int main() {
    void **ptr = NULL;
    while (1) {
        void **data = ptr;
        ptr = malloc(sizeof ptr);
        if (!ptr)
            // OOM
            return -1;

        *ptr = data;
    }

    return 0;
}
