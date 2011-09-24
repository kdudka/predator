#include "../sl.h"
#include <stdlib.h>

int main() {
    void *undef;
    void *val = NULL;
    void *ptr = &val;

    switch (___sl_get_nondet_int()) {
        case 0:
            free(undef);
            return;

        case 1:
            free(val);
            return;

        case 2:
            free(ptr);
            return;

        default:
            break;
    }

    void *item = malloc(sizeof(void *));
    free(item);
    free(item);

    return 0;
}
