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
    const char str[] = "zero-terminated string";
    char *dup = strdup(str);
    char a[sizeof str], b[sizeof str];

    int i;
    for (i = 0; i < sizeof str; ++i) {
        a[i] = str[i];
        b[i] = dup[i];
    }

    ___sl_plot("strings", &str, &dup, &a, &b);
    return 0;
}
