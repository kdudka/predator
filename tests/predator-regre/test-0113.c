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
    struct node *ptr = malloc(sizeof(struct node));
    if (!ptr)
        abort();

    ptr->l = NULL;
    ptr->r = NULL;
    return ptr;
}

void insert(struct node **pn, bool left)
{
    if (left)
        seek_lmost(&pn);
    else
        seek_rmost(&pn);

    *pn = alloc_node();
}

void *create_pseudo_tree(void)
{
    struct node *pt = alloc_node();
    unsigned i;
    for (i = 0; i < 1024; ++i)
        insert(&pt, i % 2);

    return pt;
}

void destroy(struct node *node)
{
    if (!node)
        return;

    while (node->l) {
        struct node *next = node->l->l;
        free(node->l);
        node->l = next;
    }

    while (node->r) {
        struct node *next = node->r->r;
        free(node->r);
        node->r = next;
    }

    free(node);
}

int main()
{
    void *pt = create_pseudo_tree();
    __VERIFIER_plot(NULL);

    destroy(pt);

    return 0;
}

/**
 * @file test-0113.c
 *
 * @brief error-free variant of test-0106
 *
 * - known to work with SE_COST1_LEN_THR == 4
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
