#include <stdlib.h>
#include "../sl.h"

int main() {
    char *buf = NULL;
    char *pos1 = buf + 0x07;
    char *pos2 = buf + 0x13;
    char *pos3 = pos2 - (0x13 - 0x07);

    // OK
    ___SL_ASSERT(pos1 != pos2);
    ___SL_ASSERT(pos1 == pos3);

    switch (___sl_get_nondet_int()) {
        case 0:
            // NULL dereference
            *buf = '\0';
            break;

        case 1:
            // invalid dereference
            *pos1 = '\0';
            break;

        case 2:
            // invalid dereference
            *pos2 = '\0';
            break;

        default:
            // invalid dereference
            *pos3 = '\0';
            break;
    }

    ___sl_plot(NULL);
    return !!buf;
}
