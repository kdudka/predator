#include <stdlib.h>

extern int __VERIFIER_nondet_int(void);

struct node {
    struct node *next;
    void        *data;
};

#define NEW(type) (type *) malloc(sizeof(type))

void append_node(struct node **list, void *data)
{
    struct node *node = NEW(struct node);
    node->next = *list;
    node->data = data;
    *list = node;
}

void create_list(struct node **list, void *data)
{
    append_node(list, data);
    append_node(list, data);
    while (__VERIFIER_nondet_int())
        append_node(list, data);
}

void create_shape(struct node **px, struct node **py)
{
    struct node *x = NULL;
    struct node *y = NULL;

    if (__VERIFIER_nondet_int()) {
        create_list(&x, NULL);
        create_list(&y, x);
    }
    else {
        create_list(&x, NULL);
    }

    *px = x;
    *py = y;
}

int main()
{
    struct node *x;
    struct node *y;

    create_shape(&x, &y);

    __VERIFIER_plot(NULL, &x, &y);
}

/**
 * @file test-0240.c
 *
 * @brief test-case once used for debugging of rejoinObj()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
