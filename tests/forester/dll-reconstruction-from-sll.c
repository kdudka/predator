/*
 * Doubly linked list reconstruction from a singly linked list
 *
 * boxes: genericdll.boxes
 */

#include <stdlib.h>

struct item {
	struct item *next;
	struct item *prev;
};

int __nondet();

int main()
{
	// create a SLL using the 'next' field
	struct item *list = NULL;
	while (__nondet()) {
		struct item *ptr = malloc(sizeof(*ptr));
		ptr->next = NULL;
		ptr->prev = NULL;
		ptr->next = list;

		list = ptr;
	}

	// convert the SLL to DLL by completing the 'prev' field
	struct item *pos = list;
	while (pos && pos->next) {
		struct item *prev = pos;
		pos = pos->next;
		pos->prev = prev;
	}

	// convert the DLL to SLL by zeroing the 'next' field
	while (list && list->next) {
		struct item *next = list->next;
		list->next = NULL;
		list = next;
	}

	// finally just destroy the list to silence our garbage collector
	while (list) {
		struct item *prev = list->prev;
		free(list);
		list = prev;
	}

	return 0;
}
