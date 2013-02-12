// Creating a SLL with one data pointer being all the time NULL---to be remembered by the abstraction.

#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    void        *undef; // test conversion from UV_UNINITIALIZED -> UN_UNKNOWN
    void        *null;  // test detection of shared data
    struct item *next;
};

void insert_item(struct item **ppi)
{
    struct item *pi = malloc(sizeof(*pi));
    if (!pi)
        abort();

    pi->null = NULL;
    pi->next = (*ppi) ?: NULL;
    *ppi = pi;
}

int main()
{
    struct item *list = NULL;

    int i;
    for(i = 1; i; ++i) {
        insert_item(&list);
        __VERIFIER_plot("snapshot-01");
    }

    return 0;
}

/**
 * @file test-0048.c
 *
 * @brief handling of shared/existential values during the abstraction
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
