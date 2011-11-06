#include <stdlib.h>

typedef void *TItem;

static void dispose_list_using_while(TItem *list) {
    while (list) {
        TItem *item = list;
        list = (TItem *) *list;
        free(item);
    }
}

typedef void (*TDisposeList) (TItem *);

void test(TDisposeList dispose_list) {
    dispose_list(NULL);
}

int main() {
    test(dispose_list_using_while);
    return 0;
}
