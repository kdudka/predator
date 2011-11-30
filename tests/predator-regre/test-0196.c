#include <stdbool.h>
#include <stdlib.h>
#include <verifier-builtins.h>

static void *zalloc(size_t nelem, size_t elsize)
{
    void *addr = calloc(nelem, elsize);
    if (addr)
        return addr;

    // OOM
    abort();
}

#define ZNEW_ARRAY(nelem, type)         ((type *) zalloc((nelem), sizeof(type)))
#define ADVANCE_PTR_BY_OFF(ptr, off)    ((typeof(ptr)) ((char *)(ptr) + (off)))

static void* create_shape(bool simple)
{
    void **root = NULL;
    do {
        void **node = ZNEW_ARRAY(2, void *);
        void **next = ADVANCE_PTR_BY_OFF(node, sizeof(void *));
        *next = root;
        root  = node;

        *node = (simple || ___sl_get_nondet_int())
            ? next
            : node;
    }
    while (___sl_get_nondet_int());

    return root;
}

int main()
{
    void *simple = create_shape(/* simple */ true);
    ___sl_plot("01-simple", &simple);

    void *tricky = create_shape(/* simple */ false);
    ___sl_plot("02-tricky", &tricky);

    return 0;
}
