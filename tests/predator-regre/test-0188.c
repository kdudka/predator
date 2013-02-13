#include <verifier-builtins.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    static const char *szMsg[] = {
        "msg0",
        "msg1",
        NULL
    };

    switch (__VERIFIER_nondet_int()) {
        case 0:
            return printf("%s\n", szMsg[0], szMsg[1]);

        case 1:
            // this one is correct
            return printf("%s%s\n", szMsg[0], szMsg[1]);

        case 2:
            return printf("%s%s%s\n", szMsg[0], szMsg[1]);

        case 3:
            return printf("%d%d%d\n", 0, 1);
    }

    return EXIT_FAILURE;
}

/**
 * @file test-0188.c
 *
 * @brief wrong count of arguments given to printf()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
