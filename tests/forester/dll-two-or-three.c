/*
 * Doubly linked list with 2 or 3 elements construction and deletion.
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

	// the first element
	x = malloc(sizeof(struct T));
	x->next = NULL;
	x->prev = NULL;

	// the second element
	y = malloc(sizeof(struct T));
	y->next = x;
	x->prev = y;
	y->prev = NULL;
	x = y;

	if (__VERIFIER_nondet_int())
	{	// the third element
		y = malloc(sizeof(struct T));
		y->next = x;
		x->prev = y;
		y->prev = NULL;
		x = y;
	}

	while (x) {
		y = x->next;
		free(x);
		x = y;
	}

	return 0;
}
