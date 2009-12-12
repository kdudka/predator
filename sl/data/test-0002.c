#include <stdlib.h>

int main() {
    void **undef;
    void **null_value = NULL;
    void **err = *null_value;

    null_value = *undef;

    return 0;
}
