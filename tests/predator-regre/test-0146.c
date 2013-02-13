#include <verifier-builtins.h>
#include <stdlib.h>

struct list_head {
	struct list_head *next, *prev;
};

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

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

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

struct my_nested_hlist {
    int x;
    struct hlist_node node;
};

struct my_list {
    struct hlist_head nested;
    struct list_head head;
};

void add_nested(struct hlist_head *hhead)
{
    struct my_nested_hlist *nested = malloc(sizeof *nested);
    if (!nested)
        abort();

    INIT_HLIST_NODE(&nested->node);
    hlist_add_head(&nested->node, hhead);
}

void add_item(struct list_head *head) {
    struct my_list *item = malloc(sizeof *item);
    if (!item)
        abort();

    list_add_tail(&item->head, head);
    INIT_HLIST_HEAD(&item->nested);

    // TODO: support for 0+ nested lists
    add_nested(&item->nested);
    add_nested(&item->nested);
    while (__VERIFIER_nondet_int())
        add_nested(&item->nested);
}

int main() {
    LIST_HEAD(my_list_head);

    while (__VERIFIER_nondet_int())
        add_item(&my_list_head);

    __VERIFIER_plot(NULL);

    return 0;
}
