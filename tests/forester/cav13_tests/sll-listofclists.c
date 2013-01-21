/*
 * Singly linked list of circular singly linked lists
 *
 * boxes: listofclists.sll.boxes
 */

#include <stdlib.h>
#include <verifier-builtins.h>

int main() {

	struct T {
		struct T* next;
		int data;
	};

	struct T2 {
		struct T head;
		struct T2* next;
	};

	struct T2* first = NULL;
	struct T2* last = NULL;

	while (__VERIFIER_nondet_int()) {

		struct T2* x = malloc(sizeof(struct T2));
		x->next = NULL;
		x->head.next = &x->head;
		x->head.data = 0;

		struct T* y = NULL;

		while (__VERIFIER_nondet_int()) {
			y = malloc(sizeof(struct T));
			y->next = x->head.next;
			y->data = 0;
			x->head.next = y;
			y = NULL;
		}

		if (!first) {
			first = x;
			last = x;
		} else {
			last->next = x;
			last = x;
		}

	}

	while (first) {

		struct T2* x = first;
		first = first->next;

		struct T* y = x->head.next;
		while (y != &x->head) {
			struct T* z = y;
			y = y->next;
			free(z);
		}

		free(x);

	}

	return 0;

}
