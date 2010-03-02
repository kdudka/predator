#include <stdlib.h>

typedef struct list {
    struct list *head;
    struct list *next;
} list_t;

static void chain_item(list_t *list, list_t item) {
    item = *list;
    list->next = &item;
}

static void free_list(list_t *list) {
    list = list->next;
    while (list) {
        list_t *next = list->next;
        free(list);
        list = next;
    }
}

int main() {
    list_t list = { .head = &list, .next = NULL };
    list_t *item = (list_t *) malloc(sizeof item);
    chain_item(&list, *item);
    free_list(list.head);
    return 0;
}
