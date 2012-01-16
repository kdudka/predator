/*
 * Circular doubly linked list insertion and deletion.
 */

#include <stdlib.h>

int main() {

	struct T {
		struct T* next;
		struct T* prev;
		int data;
	};

	struct T* x = NULL;
	struct T* y = NULL;

	x = malloc(sizeof(struct T));
	x->next = x;
	x->prev = x;
	x->data = 0;

	while (__nondet()) {
		y = malloc(sizeof(struct T));
		y->next = x->next;
		y->next->prev = y;
		y->prev = x;
		y->data = 0;
		x->next = y;
		y = NULL;
	}

	y = x->next;

	while (y != x) {
		struct T* z = y;
		y = y->next;
		free(z);
	}

	free(x);

	return 0;

}
