#include <stdlib.h>

int main() {
    void **ptr = NULL;

    while (1) {
        void **data = ptr;
        ptr = malloc(sizeof ptr);
        free(ptr);
        ptr = data;
    }

    return 0;
}

/**
 * @file test-0004.c
 *
 * @brief infinite loop, one malloc() and one free() per each iteration
 *
 * - the analysis stops, though the program is infinite
 * - there is no junk and no memory error possible
 * - Predator issues a warning:
 *     "end of function main() has not been reached"
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
