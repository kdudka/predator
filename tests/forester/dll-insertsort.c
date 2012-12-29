/*
 * Doubly linked list insert-sort
 *
 * boxes: genericdll.boxes
 */

#include <stdlib.h>
#include <verifier-builtins.h>

int main() {

	struct T {
		struct T* next;
		struct T* prev;
	};

	struct T* x = NULL;
	struct T* y = NULL;

	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(struct T));
		y->next = x;
		y->prev = NULL;
		if (x)
			x->prev = y;
		x = y;
	}

	struct T* sorted = NULL;
	struct T* pred = NULL;
	struct T* z = NULL;

	while (x) {
		y = x;
		x = x->next;
		z = sorted;
		pred = NULL;

		while (z && __VERIFIER_nondet_int()) {
			pred = z;
			z = z->next;
		}

		y->next = z;
		if (z) z->prev = y;
		y->prev = pred;
		if (pred) pred->next = y;
		else sorted = y;
	}

	while (sorted != NULL) {
		y = sorted;
		sorted = sorted->next;
		free(y);
	}

	return 0;

}
