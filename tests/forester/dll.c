/*
 * Doubly linked list construction and deletion.
 */

#include <stdlib.h>
#include <verifier-builtins.h>

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

	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(struct T));
		y->next = x;
		x->prev = y;
		y->prev = NULL;
		x = y;
	}

	__VERIFIER_plot("test-f0028-fixpoint");

	while (x) {
		y = x->next;
		free(x);
		x = y;
	}

	return 0;
}
