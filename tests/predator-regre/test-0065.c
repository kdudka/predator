// Creating an SLL with two marked successor nodes (via non-null data elements),
// and then reversing the list.

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
    struct item *data1, *data2; // abusing the structure to be able to model some external data
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (pi)
        return pi;
    else
        abort();
}

struct item* create_sll_item(struct item *next) {
    struct item *pi = alloc_or_die();
    pi->next = next;
    pi->data1 = pi->data2 = NULL;
    return pi;
}

struct item* create_sll(void *end)
{
    struct item *sll = create_sll_item(end);
    sll = create_sll_item(sll);
    sll = create_sll_item(sll);

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i)
        sll = create_sll_item(sll);

    // the return will trigger further abstraction (stack frame destruction)
    return sll;
}

struct item* create_slseg(void *end)
{
    struct item *list = create_sll(end);
    struct item *sls = list->next;
    free(list);
    return sls;
}

struct item* reverse_sll(struct item *list)
{
    struct item *prev, *next;

    if (list) 
      prev = NULL;
    else 
      return list;

    while (list->next) {
        next = list->next;
        list->next = prev;
        prev = list;
        list = next;
    }
    list->next = prev;

    return list;
}

void destroy_sll(struct item *list)
{
    while (list) {
        struct item *next = list->next;
        free(list);
        list = next;
    }
}

int main()
{
    struct item *sll = create_slseg(NULL);
    sll = create_sll_item(sll);
    sll->data2 = sll; // mark the second selected node (we create the list from the end)
    sll = create_sll_item(sll);
    sll->data1 = sll; // mark the first selected node
    sll = create_slseg(sll);

    __VERIFIER_plot("main1");

    sll = reverse_sll(sll);

    __VERIFIER_plot("main2");

    destroy_sll(sll);

    return 0;
}

/**
 * @file test-0065.c
 *
 * @brief SLL reversion with an ordering check
 *
 * - contributed by Tomas Vojnar
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
