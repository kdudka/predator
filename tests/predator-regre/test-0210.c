#include <verifier-builtins.h>

#include <stdlib.h>
#include <string.h>

char *strdup(const char *str)
{
    size_t len = 1U + strlen(str);
    char *dup = malloc(len);
    if (!dup)
        return NULL;

    return memcpy(dup, str, len);
}

int main()
{
    char str[] = "string", a[sizeof str], b[sizeof str];
    ___SL_ASSERT(a != b);
    ___SL_ASSERT(a != str);
    ___SL_ASSERT(b != str);

    char *dup = strdup(str);
    ___SL_ASSERT(dup != str);
    ___SL_ASSERT(dup != a);
    ___SL_ASSERT(dup != b);
    if (!dup)
        abort();

    int i;
    for (i = 0; i < sizeof a; ++i) {
        a[i] = str[i];
        b[i] = dup[i];
    }

    ___sl_plot("loop_done");
    for (i = 0; i < sizeof b; ++i) {
        ___SL_ASSERT(a[i] == b[i]);
        ___SL_ASSERT(a[i] == dup[i]);
        ___SL_ASSERT(b[i] == str[i]);
    }

    ___sl_plot("all_done");
    free(dup);
    return 0;
}

/**
 * @file test-0210.c
 *
 * @brief extended version of test-0209.c with explicit assertion checks
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
