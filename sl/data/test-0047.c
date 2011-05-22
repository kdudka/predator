// A basic test of creating a SLL, starting with a possibly undefined list pointer.

#include "../sl.h"
#include <stdlib.h>

struct item {
    int         nth;
    struct item *next;
    void        *data;
};

void insert_item(struct item **ppi)
{
    struct item *pi = malloc(sizeof(*pi));
    if (!pi)
        abort();

    if (*ppi) {
        pi->nth     = (*ppi)->nth + 1;
        pi->next    = (*ppi);
    }
    else {
        pi->nth     = 0;
        pi->next    = NULL;
    }
    *ppi = pi;
}

int main()
{
    struct item *list/* = NULL*/;

    int i;
    for(i = 1; i; ++i) {
        insert_item(&list);
        ___sl_plot("snapshot-01");
    }

    return 0;
}
