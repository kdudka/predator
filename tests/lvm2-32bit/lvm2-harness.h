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
