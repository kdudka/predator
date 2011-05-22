// Creating a SLL with one data pointer being all the time NULL---to be remembered by the abstraction.

#include "../sl.h"
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
        ___sl_plot("snapshot-01");
    }

    return 0;
}
