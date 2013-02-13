/*
 * Singly linked list insert-sort
 *  - limited length
 *
 * boxes:
 */
#include <stdlib.h>

int main() {

	struct T {
		struct T* next;
	};

	struct T* x = NULL;
	struct T* y = NULL;

	// allocate SLL of length 2
	y = malloc(sizeof(*y));
	y->next = x;
	x = y;

	y = malloc(sizeof(*y));
	y->next = x;
	x = y;

	struct T* sorted = NULL;
	struct T* pred = NULL;
	struct T* z = NULL;

//	__VERIFIER_plot("sll-insertsort-limited-0001");

	while (x) {
		y = x;
		x = x->next;
		pred = NULL;
		z = sorted;

		while (z) {
			pred = z;
			z = z->next;
		}

//		__VERIFIER_plot(NULL);

		y->next = z;
//		__VERIFIER_plot("sll-insertsort-limited-0003");
		if (pred) pred->next = y;
		else sorted = y;
	}

//	__VERIFIER_plot("sll-insertsort-limited-0002");

	while (sorted != NULL) {
		x = sorted;
		sorted = sorted->next;
		free(x);
	}

	return 0;

}
