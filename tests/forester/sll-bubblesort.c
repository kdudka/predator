/*
 * Singly linked list bubble-sort
 *
 * boxes:
 */

#include <stdlib.h>
#include <stdbool.h>

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

	if (!x)
		return 0;

	struct T* pred, * succ;

	bool sorted = false;

	while (!sorted) {
		sorted = true;
		y = x;
		pred = NULL;
		while (y && y->next) {
			if (__nondet()) {
				succ = y->next;
				if (pred) pred->next = succ;
				else x = succ;
				y->next = succ->next;
				succ->next = y;
				sorted = false;
			}
			pred = y;
			y = y->next;
		}
	}

	while (x != NULL) {
		y = x;
		x = x->next;
		free(y);
	}

	return 0;

}
