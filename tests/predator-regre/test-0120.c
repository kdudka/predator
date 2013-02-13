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

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }

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

struct my_hlist {
    struct list_head nested;
    struct hlist_node node;
};

int main() {
    HLIST_HEAD(my_hlist_head);

    struct my_hlist *item = malloc(sizeof *item);
    if (!item)
        return EXIT_FAILURE;

    INIT_LIST_HEAD(&item->nested);
    INIT_HLIST_NODE(&item->node);
    hlist_add_head(&item->node, &my_hlist_head);
    if (hlist_empty(&my_hlist_head))
        abort();

    __VERIFIER_plot(NULL);
    free(item);
    return EXIT_SUCCESS;
}

/**
 * @file test-0120.c
 *
 * @brief Linux "list" of length one, based on hlist_head/hlist_node
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
