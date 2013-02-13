#include <verifier-builtins.h>
#include <stdbool.h>
#include <stdlib.h>

struct node {
    struct node *l;
    struct node *r;
};

void seek_lmost(struct node ***pn)
{
    while (**pn)
        *pn = &(**pn)->l;
}

void seek_rmost(struct node ***pn)
{
    while (**pn)
        *pn = &(**pn)->r;
}

void* alloc_node(void)
{
    void *ptr = malloc(sizeof(struct node));
    if (ptr)
        return ptr;
    else
        abort();
}

void insert(struct node **pn, bool left)
{
    if (left)
        seek_lmost(&pn);
    else
        seek_rmost(&pn);

    *pn = alloc_node();
    (*pn)->l = NULL;
    (*pn)->r = NULL;
}

void *create_pseudo_tree(void)
{
#if 1
    // oops, we forgot to initialize the root pointer!
    struct node *pt;
#else
    struct node *pt = NULL;
#endif

    unsigned i;
    for (i = 0; i < 1024; ++i)
        insert(&pt, i % 2);

    return pt;
}

void destroy(struct node *n)
{
    if (!n)
        return;

    // we have no support for unbounded recursion yet
#if 0
    destroy(n->l);
    destroy(n->r);
    free(n);
#endif
}

int main()
{
    void *pt = create_pseudo_tree();
    __VERIFIER_plot(NULL);

    destroy(pt);

    return 0;
}

/**
 * @file test-0106.c
 *
 * @brief pseudo tree consisting of two SLLs
 *
 * - known to work with SE_COST1_LEN_THR == 4
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
