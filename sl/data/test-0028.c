#include <stdlib.h>

typedef struct list { struct list *head; struct list *next; } list_t;

static void chain_item(list_t *l, list_t item) {
    item = *l;
    l->next = &item;
}

static void safe_free_list(list_t l) {
    while (l.head && (l.head = l.next)) {
        list_t *next = l.head->next;
        free(l.head);
        l.head = next;
    }
}

int main() {
    list_t l = { .head = &l, .next = NULL };
    list_t *item = (list_t *) malloc(sizeof item);
    chain_item(&l, *item);
    safe_free_list(l);
    safe_free_list(l);
    return 0;
}
