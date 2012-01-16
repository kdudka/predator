/*
 * Singly linked list of even length
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

	while (__nondet()) {
		y = malloc(sizeof(*y));
		y->next = x;
		x = y;
		y = malloc(sizeof(*y));
		y->next = x;
		x = y;
	}

	while (y != NULL) {
		x = y;
		y = y->next;
		free(x);
		x = y;
		y = y->next;
		free(x);
	}

	return 0;
}
