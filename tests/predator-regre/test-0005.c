#include <stdlib.h>

typedef void *TItem;

int main() {
    TItem *list = malloc(sizeof(TItem));
    *list = malloc(sizeof(TItem));
    *((TItem *) *list) = NULL;

    while (list) {
        TItem *item = list;
        list = (TItem *) *list;
        free(item);
    }

    return 0;
}
