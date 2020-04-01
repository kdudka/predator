#include <string.h>

int main()
{
    const char src[] = "XE";
    char dst[sizeof src];
    strncpy(dst, src, sizeof dst);
}

/**
 * @file test-0262.c
 *
 * @brief internal error while dereferencing a short string literal
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
