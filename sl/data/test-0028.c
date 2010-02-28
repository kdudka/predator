#include <stdlib.h>

typedef struct list { struct list *head; struct list *next; } list_t;

static void chain_item(list_t *list, list_t item) {
    item = *list;
    list->next = &item;
}

static void safe_free_list(list_t list) {
    while (list.head && (list.head = list.next)) {
        list_t *next = list.head->next;
        free(list.head);
        list.head = next;
    }
}

int main() {
    list_t list = { .head = &list, .next = NULL };
    list_t *item = (list_t *) malloc(sizeof item);
    chain_item(&list, *item);
    safe_free_list(list);
    safe_free_list(list);
    return 0;
}
