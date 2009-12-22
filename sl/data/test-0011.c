#include <stdlib.h>

int main() {
    // unchecked result of malloc --> possible dereference of NULL
    void **v = malloc(sizeof(*v));
    *v = NULL;

    v = malloc(sizeof(*v));
    if (v)
        // this should be OK
        *v = NULL;

    return 0;
}
