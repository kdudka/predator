#include <verifier-builtins.h>

int main() {
    char buf[0x10];
    char *pos1 = buf + 0x07;
    char *pos2 = buf + 0x13;
    char *pos3 = pos2 - (0x13 - 0x07);

    // OK
    ___SL_ASSERT(buf != pos1);
    ___SL_ASSERT(buf != pos2);
    ___SL_ASSERT(buf != pos3);

    // OK
    ___SL_ASSERT(pos1 != pos2);
    ___SL_ASSERT(pos1 == pos3);

    // OK
    *pos3 = '\0';

    // out of bounds
    *pos2 = '\0';

    ___sl_plot((char *) 0);
    return !!buf;
}

/**
 * @file test-0159.c
 *
 * @brief off-values handling, delayed killing of pointed variables
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
