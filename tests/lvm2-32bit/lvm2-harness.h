#include <verifier-builtins.h>

void *dm_malloc_aux(size_t s, const char *file, int line)
{
    (void) file;
    (void) line;
    return malloc(s);
}

void *dm_zalloc_aux(size_t s, const char *file, int line)
{
    (void) file;
    (void) line;
    return calloc(s, 1);
}

char *__strdup (__const char *__string)
{
    size_t len = 1UL + strlen(__string);
    char *dup = malloc(len);
    memmove(dup, __string, len);
    return dup;
}

char *strncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
{
    return memcpy(__dest, __src, __n);
}

int strncmp (__const char *__s1, __const char *__s2, size_t __n)
{
    strlen(__s1);
    strlen(__s2);
    (void) __n;
    return ___sl_get_nondet_int();
}

int is_orphan_vg(const char *vg_name)
{
    (void) vg_name;
    return ___sl_get_nondet_int();
}
