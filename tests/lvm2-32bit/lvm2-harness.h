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
    puts(__string);
    return (void *) ___sl_get_nondet_int();
}

char *strncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
{
    return memmove(__dest, __src, __n);
}

size_t strlen (__const char *__s)
{
    puts(__s);
    return ___sl_get_nondet_int();
}

int strncmp (__const char *__s1, __const char *__s2, size_t __n)
{
    // TODO
#if 0
    puts(__s1);
    puts(__s2);
#else
    (void) __s1;
    (void) __s1;
#endif
    (void) __n;
    return ___sl_get_nondet_int();
}

int is_orphan_vg(const char *vg_name)
{
    (void) vg_name;
    return ___sl_get_nondet_int();
}
