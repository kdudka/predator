#include <verifier-builtins.h>

#include <stdlib.h>
#include <string.h>

char *strdup(const char *str)
{
    size_t len = 1U + strlen(str);
    char *dup = malloc(len);
    return memcpy(dup, str, len);
}

int main()
{
    const char str[] = "string";
    char *dup = strdup(str);
    char a[sizeof str], b[sizeof str];

    int i;
    for (i = 0; i < sizeof str; ++i) {
        __VERIFIER_plot("loop_body");
        a[i] = str[i];
        b[i] = dup[i];
    }

    __VERIFIER_plot("loop_done", &str, &dup, &a, &b);
    return 0;
}

/**
 * @file test-0209.c
 *
 * @brief byte-level access to zero-terminated strings
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
