#include <stdlib.h>
#include <verifier-builtins.h>

int main() {
    char *buf = NULL;
    char *pos1 = buf + 0x07;
    char *pos2 = buf + 0x13;
    char *pos3 = pos2 - (0x13 - 0x07);

    // OK
    __VERIFIER_assert(pos1 != pos2);
    __VERIFIER_assert(pos1 == pos3);

    switch (__VERIFIER_nondet_int()) {
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

    __VERIFIER_plot(NULL);
    return !!buf;
}

/**
 * @file test-0160.c
 *
 * @brief off-values using NULL as the reference and some reasoning
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
