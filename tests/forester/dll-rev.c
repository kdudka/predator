/*
 * Doubly linked list reversal
 *
 * boxes: genericdll.boxes
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

	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(struct T));
		y->next = x;
		y->prev = NULL;
		y->data = 0;
		if (x)
			x->prev = y;
		x = y;
	}

	y = x;
	x = NULL;

	while (y != NULL) {
		struct T* z = y;
		y = y->next;
		if (y)
			y->prev = NULL;
		z->next = x;
		z->prev = NULL;
		if (x)
			x->prev = z;
		x = z;
	}

	while (x != NULL) {
		y = x;
		x = x->next;
		free(y);
	}

	return 0;

}
