#include <verifier-builtins.h>
#include <stdlib.h>

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

struct list_head {
	struct list_head *next, *prev;
};

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

struct my_nested_list {
    struct list_head head;
    int x;
};

struct my_list {
    struct my_list *next;
    struct list_head nested;
};

void add_item(struct my_list **my_list) {
    struct my_list *item = malloc(sizeof *item);
    if (!item)
        abort();

    INIT_LIST_HEAD(&item->nested);

    while (__VERIFIER_nondet_int())
    {
        struct my_nested_list *nested = malloc(sizeof *nested);
        if (!nested)
            abort();

        list_add_tail(&nested->head, &item->nested);
    }

    item->next = *my_list;
    *my_list = item;
}

int main() {
    struct my_list *my_list = NULL;

    while (__VERIFIER_nondet_int()) {
        add_item(&my_list);
    }

    __VERIFIER_plot(NULL);

    while (my_list) {
        struct list_head *head = my_list->nested.next;
        while (&my_list->nested != head) {
            struct list_head *next = head->next;
            free(/* FIXME: should use list_entry() */ head);
            head = next;
        }

        struct my_list *next = my_list->next;
        free(my_list);
        my_list = next;
    }

    return 0;
}
