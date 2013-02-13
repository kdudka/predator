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
    __VERIFIER_assert(a != b);
    __VERIFIER_assert(a != str);
    __VERIFIER_assert(b != str);

    char *dup = strdup(str);
    __VERIFIER_assert(dup != str);
    __VERIFIER_assert(dup != a);
    __VERIFIER_assert(dup != b);
    if (!dup)
        abort();

    int i;
    for (i = 0; i < sizeof a; ++i) {
        a[i] = str[i];
        b[i] = dup[i];
    }

    __VERIFIER_plot("loop_done");
    for (i = 0; i < sizeof b; ++i) {
        __VERIFIER_assert(a[i] == b[i]);
        __VERIFIER_assert(a[i] == dup[i]);
        __VERIFIER_assert(b[i] == str[i]);
    }

    __VERIFIER_plot("all_done");
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
