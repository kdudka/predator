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

    // NULL dereference
    *buf = '\0';

    // invalid dereference
    *pos1 = '\0';

    // invalid dereference
    *pos2 = '\0';

    // invalid dereference
    *pos3 = '\0';

    ___sl_plot(NULL);
    return !!buf;
}
