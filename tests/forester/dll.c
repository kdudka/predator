/*
 * Doubly linked list construction and deletion.
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
	x->next = NULL;
	x->prev = NULL;

	while (__nondet()) {
		y = malloc(sizeof(struct T));
		y->next = x;
		x->prev = y;
		y->prev = NULL;
		x = y;
	}

	___fa_plot("test-f0028-fixpoint");

	while (x) {
		y = x->next;
		free(x);
		x = y;
	}

	return 0;
}
