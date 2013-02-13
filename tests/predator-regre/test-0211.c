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

char *strrev(const char *str)
{
    char *rev = strdup(str);
    if (!rev)
        return NULL;

    char *beg = rev;
    char *end = beg + strlen(rev) - 1;
    while (beg < end) {
        __VERIFIER_plot("single-step", &beg, &end);
        char c = *beg;
        *beg++ = *end;
        *end-- = c;
    }

    return rev;
}

void seek_last_char(const char **ppos)
{
    const char *pos = *ppos;

    while (pos[1])
        ++pos;

    *ppos = pos;
}

void exercise(const char *a, const char *b)
{
    for(seek_last_char(&b); *a; ++a, --b)
        __VERIFIER_assert(*a == *b);
}

int main()
{
    static const char str[] = "string";

    char *rev = strrev(str);
    if (!rev)
        return 1;

    __VERIFIER_plot("strrev-done", &str, &rev);
    __VERIFIER_assert(!rev[sizeof str - 1]);

    exercise(str, rev);
    exercise(rev, str);

    free(rev);
    return 0;
}

/**
 * @file test-0211.c
 *
 * @brief in-place reversal of a short zero-terminated string
 *
 * - works fine with (0 == SE_ALLOW_OFF_RANGES)
 *
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
