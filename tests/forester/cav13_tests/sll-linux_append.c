/*
 * Linux kernel singly linked lists
 *
 * boxes:
 */

#include <stdlib.h>
#include <verifier-builtins.h>

struct list_head {
	struct list_head *next;
};

struct my_item {
	void *data;
	struct list_head link;
};

int main()
{
	struct list_head my_list = { &(my_list) };
	int i;

	struct my_item *ptr = malloc(sizeof *ptr);
	ptr->data = ((void *)0);
	ptr->link.next = my_list.next;
	my_list.next = &ptr->link;

	ptr = malloc(sizeof *ptr);
	ptr->data = ((void *)0);
	ptr->link.next = my_list.next;
	my_list.next = &ptr->link;

	ptr = malloc(sizeof *ptr);
	ptr->data = ((void *)0);
	ptr->link.next = my_list.next;
	my_list.next = &ptr->link;

	ptr = malloc(sizeof *ptr);
	ptr->data = ((void *)0);
	ptr->link.next = my_list.next;
	my_list.next = &ptr->link;

	ptr = malloc(sizeof *ptr);
	ptr->data = ((void *)0);
	ptr->link.next = my_list.next;
	my_list.next = &ptr->link;

	ptr = malloc(sizeof *ptr);
	ptr->data = ((void *)0);
	ptr->link.next = my_list.next;
	my_list.next = &ptr->link;

	while (__VERIFIER_nondet_int()) {
		ptr = malloc(sizeof *ptr);
		ptr->data = ((void *)0);
		ptr->link.next = my_list.next;
		my_list.next = &ptr->link;
	}

	struct my_item *now = (struct my_item *)(
			(char *)my_list.next - __builtin_offsetof (struct my_item, link)
			);

	while (&now->link != (&my_list)) {
		ptr = now;
		now = (struct my_item *)(
				(char *)now->link.next - __builtin_offsetof (struct my_item, link)
				);
		free(ptr);
	}

	return 0;
}
