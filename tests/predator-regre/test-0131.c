#define TRIGGER_INVALID_WRITE       1
#define GIVE_CHANCE_TO_INVADER      1

#include <verifier-builtins.h>

/* #include <linux/stddef.h> */
#include <stddef.h>
#include <stdlib.h>

#define prefetch(x) ((void) 0)
#define typeof(x) __typeof__(x)

struct list_head {
	struct list_head *next, *prev;
};

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head *head)
{
    //__VERIFIER_plot(NULL);
	return head->next == head;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head); 	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

struct my_item {
    void                *data;
    struct list_head    link;
    struct list_head    aux_link;
};

struct my_item* my_alloc(void)
{
    struct my_item *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    return ptr;
}

void append_one(struct list_head *head)
{
    struct my_item *ptr = my_alloc();
    list_add_tail(&ptr->link, head);
}

void traverse(struct list_head *head)
{
    struct my_item *now;
#if TRIGGER_INVALID_WRITE
    list_for_each_entry_reverse(now, head, aux_link)
#else
    list_for_each_entry_reverse(now, head, link)
#endif
    {
        now->data = NULL;
        __VERIFIER_plot("now");
    }
}

int main()
{
    LIST_HEAD(my_list);
    if (!list_empty(&my_list))
        return 1;

    append_one(&my_list);
    append_one(&my_list);

#if !GIVE_CHANCE_TO_INVADER
    // Predator supports arbitrarily long Linux lists...
    int i;
    for (i = 0; i < 1024; ++i)
        append_one(&my_list);
#endif

    traverse(&my_list);

    return 0;
}

/**
 * @file test-0131.c
 *
 * @brief combination of test-0037 and test-0073
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
