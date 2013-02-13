#include <verifier-builtins.h>
#include <stdbool.h>
#include <stdlib.h>

struct data {
    struct data* (*allocator)();

    union {
        const char      *leaf_name;
        int             nearest_leaf_distance;
    } /* anonymous union */;

    union {
        bool do_not_free_name;

        struct {
            struct data *lptr;
            struct data *rptr;
        } /* anonymous struct */;

    } named_union;

};

#define NEW(type) ({                    \
    void *ptr = malloc(sizeof(type));   \
    if (!ptr)                           \
        abort();                        \
    (type *) ptr;                       \
});

static struct data* alloc_leaf_node(const char *name)
{
    struct data *d = NEW(struct data);
    d->allocator = alloc_leaf_node;
    d->leaf_name = name;
    d->named_union.do_not_free_name = true;
    return d;
}

static struct data* alloc_non_leaf(struct data *lptr, struct data *rptr)
{
    struct data *d = NEW(struct data);
    d->allocator = alloc_non_leaf;
    d->nearest_leaf_distance =
        (alloc_leaf_node == lptr->allocator) &&
        (alloc_leaf_node == rptr->allocator);

    d->named_union.lptr = lptr;
    d->named_union.rptr = rptr;
    return d;
}

static struct data* alloc_tripple(void)
{
    static const char term = '\0';
    struct data *lptr = alloc_leaf_node(&term);
    struct data *rptr = alloc_leaf_node(&term);

    struct data *root = alloc_non_leaf(lptr, rptr);
    return root;
}

static void destroy_tree(struct data *tree)
{
    if (alloc_leaf_node == tree->allocator) {
        if (!tree->named_union.do_not_free_name)
            free((void *) tree->leaf_name);

        free(tree);
        return;
    }

    if (!tree->nearest_leaf_distance)
        // internal error
        abort();

    destroy_tree(tree->named_union.lptr);
    destroy_tree(tree->named_union.rptr);
    free(tree);
}

int main()
{
    struct data *tree = alloc_tripple();
    
    // wrong use of union
    const char *name = tree->leaf_name;
    const int rv = *name;
    __VERIFIER_plot(NULL);

    destroy_tree(tree);
    return rv;
}

/**
 * @file test-0094.c
 *
 * @brief regression test focused on named/anonymous struct/unions
 *
 *   and handling of function pointers (heap graph available)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
