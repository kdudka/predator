#include "../sl.h"
#include <stdlib.h>

int main() {
    void **undef;
    void **null_value = NULL;

    if (___sl_get_nondet_int())
        null_value = *undef;

    void **err = *null_value;

    return 0;
}
