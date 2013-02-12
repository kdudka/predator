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

        *node = (simple || __VERIFIER_nondet_int())
            ? next
            : node;
    }
    while (__VERIFIER_nondet_int());

    return root;
}

int main()
{
    void *simple = create_shape(/* simple */ true);
    __VERIFIER_plot("01-simple", &simple);

    void *tricky = create_shape(/* simple */ false);
    __VERIFIER_plot("02-tricky", &tricky);

    return 0;
}

/**
 * @file test-0196.c
 *
 * @brief a training example for the introduction of VT_RANGE
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
