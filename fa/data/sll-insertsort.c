/*
 * Singly linked list insert-sort
 *
 * boxes:
 */
#include <stdlib.h>

int __nondet();

int main() {

	struct T {
		struct T* next;
	};

	struct T* x = NULL;
	struct T* y = NULL;

	while (__nondet()) {
		y = malloc(sizeof(*y));
		y->next = x;
		x = y;
	}

	struct T* sorted = NULL;
	struct T* pred = NULL;
	struct T* z = NULL;

	while (x) {
		y = x;
		x = x->next;
		pred = NULL;
		z = sorted;

		while (z && __nondet()) {
			pred = z;
			z = z->next;
		}

		y->next = z;
		if (pred) pred->next = y;
		else sorted = y;
	}

	while (sorted != NULL) {
		x = sorted;
		sorted = sorted->next;
		free(x);
	}

	return 0;

}
