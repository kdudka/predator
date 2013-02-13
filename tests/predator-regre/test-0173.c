#include <verifier-builtins.h>
#include <stdlib.h>

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL }

static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
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

struct my_node {
    void (*data)(void);
    struct hlist_node head;
};

static void append(struct hlist_head *list)
{
    struct my_node *node = malloc(sizeof *node);
    if (!node)
        abort();

    hlist_add_head(&node->head, list);
}

int main()
{
    HLIST_HEAD(head);

    append(&head);
    append(&head);

    __VERIFIER_plot(NULL, &head);
    return 0;
}
