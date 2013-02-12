#include <verifier-builtins.h>

#include <stdbool.h>
#include <stdlib.h>

#define BREAK_IMMATURE_VERSION_OF_PREDATOR 1

struct data {
#if !BREAK_IMMATURE_VERSION_OF_PREDATOR
    void *etc;
#endif
    int  len;
    char *text;
};

static struct data* alloc_char(char c)
{
    struct data *d = malloc(sizeof *d);
    if (!d)
        abort();

#if !BREAK_IMMATURE_VERSION_OF_PREDATOR
    d->etc = NULL;
#endif

    d->len = 2;
    d->text = calloc(2, sizeof(char));
    if (!d->text)
        abort();

    d->text[0] = c;
    return d;
}

struct node {
    struct data *data;
    struct node *next;
};

static void insert_node(struct node **pn, char c)
{
    struct node *n = malloc(sizeof *n);
    if (!n)
        abort();

    n->data = (c) ? alloc_char(c) : NULL;
    n->next = *pn;
    *pn = n;
}

static struct node* create_shape(void)
{
    struct node *list = NULL;

    insert_node(&list, 'A');
    insert_node(&list, '\0');
    insert_node(&list, 'A');

    ___sl_enable_debugging_of(___SL_SYMJOIN, /* enable */ true);
    return list;
}

int main() {
    struct node *shape = create_shape();
    __VERIFIER_plot(NULL, &shape);

    while (shape) {
        struct node *next = shape->next;
        struct data *data = shape->data;

        if (data) {
            free(data->text);
            free(data);
        }

        free(shape);
        shape = next;
    }

    return 0;
}

/**
 * @file test-0193.c
 *
 * @brief a trivial example on use of joinData(), once used as a demo
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
