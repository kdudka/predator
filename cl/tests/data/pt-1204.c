/* taken from tests/predator-regre/test-0221.c */

#include <stdlib.h>

int ___sl_get_nondet_int(void);
void __VERIFIER_plot(const char *name, ...);

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

struct L2 {
    struct list_head    head;
    void               *proto;
};

struct L1 {
    struct list_head    head;
    struct list_head    l2;
};

static void l2_insert(struct list_head *list)
{
    struct L2 *item = malloc(sizeof *item);
    if (!item)
        abort();

    item->proto = malloc(119U);
    if (!item->proto)
        abort();

    list_add(&item->head, list);
}

static void l1_insert(struct list_head *list)
{
    struct L1 *item = malloc(sizeof *item);
    if (!item)
        abort();

    INIT_LIST_HEAD(&item->l2);
    l2_insert(&item->l2);

    list_add(&item->head, list);

    do
        l2_insert(&item->l2);
    while (___sl_get_nondet_int());
}

static void create_all_levels(struct list_head *list)
{
    l1_insert(list);
    do
        l1_insert(list);
    while (___sl_get_nondet_int());
}

int main()
{
    LIST_HEAD(list);
    create_all_levels(&list);
    __VERIFIER_plot("01-ready", &list);
}
