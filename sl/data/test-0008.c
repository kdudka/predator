#include "../sl.h"
#include <stdlib.h>

typedef void *TItem;

static void dispose_list_using_while(TItem *list) {
    while (list) {
        TItem *item = list;
        list = (TItem *) *list;
        free(item);
    }
}

static void dispose_list_recursively(TItem *list) {
    ___sl_plot(NULL);
    if (list) {
        dispose_list_recursively((TItem *) *list);
        free(list);
    }
}

typedef void (*TDisposeList) (TItem *);

void test(TDisposeList dispose_list) {
    dispose_list(NULL);

    TItem *list = malloc(sizeof(TItem));
    *list = NULL;
    dispose_list(list);

    list = malloc(sizeof(TItem));
    *list = malloc(sizeof(TItem));
    *((TItem *) *list) = NULL;
    dispose_list(list);
}

int main() {
    test(dispose_list_using_while);
    test(dispose_list_recursively);

    return 0;
}
