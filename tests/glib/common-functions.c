// this file needs to be included to tests (after other glib includes)
// 1) eliminates optimized allocation strategies in glib
// 2) eliminates printing of messages (string manipulation)

// extern declarations: TODO: use stdlib.h ?
extern void * calloc (size_t, size_t);
extern void * malloc (size_t);
extern void free(void *);
extern void __attribute__((noreturn)) abort(void);

// malloc with check
void *my_checked_malloc(size_t sz) {
    void *ptr = malloc(sz);
    if(!ptr)
        abort();
    return ptr;
}
// calloc with check
void *my_checked_calloc(size_t sz) {
    void *ptr = calloc(1, sz);
    if(!ptr)
        abort();
    return ptr;
}

// GLIB allocation macros replacement
#undef g_slice_new0
#define  g_slice_new0(type)     ((type*) my_checked_calloc (sizeof (type)))
#undef g_slice_new
#define  g_slice_new(type)      ((type*) my_checked_malloc (sizeof (type)))
#undef g_slice_free
#define  g_slice_free(type,ptr) free(ptr)

void g_assertion_message_expr (const char *domain, const char *file,
     int line, const char *func, const char *expr) /*G_GNUC_NORETURN*/ {
    abort();
}
void    g_assertion_message_cmpnum      (const char     *domain,
                                         const char     *file,
                                         int             line,
                                         const char     *func,
                                         const char     *expr,
                                         long double     arg1,
                                         const char     *cmp,
                                         long double     arg2,
                                         char            numtype)
/*G_GNUC_NORETURN*/ {
    abort();
}

void g_return_if_fail_warning (const char *log_domain,
                               const char *pretty_function,
                               const char *expression) {
/* empty */
}


// gslice.h function, eliminates extra checks
inline __attribute__((always_inline))
void
g_slice_free_chain_with_offset (gsize    mem_size,
                                gpointer mem_chain,
                                gsize    next_offset) {
 /* TODO: needs better implementation */
    gpointer slice = mem_chain;
    while (slice)
      {
        guint8 *current = slice;
        slice = *(gpointer*) (current + next_offset);
        free (current);
      }
#if 0 // FYI: original code fragment
  else                                  /* delegate to system malloc */
    while (slice)
      {
        guint8 *current = slice;
        slice = *(gpointer*) (current + next_offset);
        if (G_UNLIKELY (allocator->config.debug_blocks) &&
            !smc_notify_free (current, mem_size))
          abort();
        if (G_UNLIKELY (g_mem_gc_friendly))
          memset (current, 0, mem_size);
        g_free (current);
      }
#endif

}

