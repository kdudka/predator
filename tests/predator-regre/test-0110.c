#include <verifier-builtins.h>
#include <stdlib.h>

struct node {
    struct node *next;
    struct node *data;
};

struct node* create(void)
{
    struct node *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->next = NULL;
    ptr->data = NULL;

    return ptr;
}

struct node* alloc1(void)
{
    struct node *pi = create();
    pi->data = create();
    return pi;
}

struct node* alloc2(void)
{
    struct node *pi = create();
#ifdef PREDATOR
    pi->data = create();
    pi->data->data = create();
#else
    // a special quirk for tools that are not ready for real-world code sources
    struct node *help = create();
    pi->data = help;
    help->data = create();
#endif

    return pi;
}

struct node* create_sll(void)
{
    struct node *sll = alloc1();
    struct node *now = sll;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        now->next = alloc2();
        now = now->next;
        now->next = alloc1();
        now = now->next;
    }

    return sll;
}

int main()
{
    struct node *sll = create_sll();
    __VERIFIER_plot(NULL);

    return 0;
}

/**
 * @file test-0110.c
 *
 * @brief top level SLL
 *
 * - odd nodes contain only one item of nested SLL (not a SLL)
 * - even nodes contain two items of nested SLL
 * - the same type is used at both levels, which helps negligibly
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
