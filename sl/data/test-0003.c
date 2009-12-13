#include <stdlib.h>

int main() {
    void *val = NULL;
    void *ptr = &val;

    free(val);
    free(ptr);

    return 0;
}
