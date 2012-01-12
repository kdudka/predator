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
    strlen(vg_name);
    return ___sl_get_nondet_int();
}

/* verbatim copy from uuid-prep.c */
int id_write_format(const struct id *id, char *buffer, size_t size)
{
 int i, tot;

 static const unsigned group_size[] = { 6, 4, 4, 4, 4, 4, 6 };

 ((32 == 32) ? (void) (0) : __assert_fail ("32 == 32", "uuid/uuid.c", 163, __PRETTY_FUNCTION__));


 if (size < (32 + 6 + 1)) {
  print_log(3, "uuid/uuid.c", 167 , -1,"Couldn't write uuid, buffer too small.");
  return 0;
 }

 for (i = 0, tot = 0; i < 7; i++) {
  memcpy(buffer, id->uuid + tot, group_size[i]);
  buffer += group_size[i];
  tot += group_size[i];
  *buffer++ = '-';
 }

 *--buffer = '\0';
 return 1;
}
