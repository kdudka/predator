/*
 * Doubly linked list construction and deletion. The construction is done in two
 * ways (forward and backward).
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

	// ``normal'' DLL
	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(struct T));
		y->next = x;
		x->prev = y;
		y->prev = NULL;
		x = y;
	}

	struct T* z = NULL;

	y = malloc(sizeof(struct T));
	y->next = NULL;
	y->prev = NULL;

	// ``reversed'' DLL
	while (__VERIFIER_nondet_int()) {
		z = malloc(sizeof(struct T));
		z->prev = y;
		y->next = z;
		z->next = NULL;
		y = z;
	}

	struct T* xEnd = x;

	// go to the end of 'x'
	while (xEnd->next)
	{
		xEnd = xEnd->next;
	}

	// got to the end of 'y'
	while (y->prev)
	{
		y = y->prev;
	}

	// interweave 'x' and 'y'
	xEnd->next = y;
	y->prev = xEnd;

	y = x;
	while (y) {
		y = y->next;
	}

	while (x) {
		y = x->next;
		free(x);
		x = y;
	}

	return 0;
}
