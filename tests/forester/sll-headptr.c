/*
 * Singly linked list with head pointers
 *
 * boxes:
 */

#include <stdlib.h>
#include <verifier-builtins.h>

struct T {
	struct T* next;
	struct T* head;
};

int main() {

	struct T* head = malloc(sizeof(struct T));

	head->next = NULL;
	head->head = head;

	struct T* x = head;

	while (__VERIFIER_nondet_int()) {
		x->next = malloc(sizeof(struct T));
		x->next->next = NULL;
		x->next->head = head;
		x = x->next;
	}

	x = head;

	while (x != NULL)
		x = x->next;

	x = head;

	while (x != NULL) {
		struct T* y = x;
		x = x->next;
		free(y);
	}

	return 0;
}
