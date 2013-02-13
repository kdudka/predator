/*
 * Doubly linked list construction and deletion. The construction is done in two
 * ways (forward and backward).
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

	// first DLL
	while (__nondet()) {
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

	// pointer to the end of y
	struct T* yLast = y;

	// second DLL
	while (__nondet()) {
		z = malloc(sizeof(struct T));
		z->next = y;
		y->prev = z;
		z->prev = NULL;
		y = z;
	}

	// concat 'x' and 'y'
	yLast->next = x;
	x->prev = yLast;

	y = NULL;
	z = NULL;

	__VERIFIER_plot("dll-concat-lists-0001");

	y = x;
	while (y) {
		y = y->next;
	}

	__VERIFIER_plot("dll-concat-lists-0002");

	// delete
	while (x->next) {
		y = x->next;
		x->next = x->next->next;
		free(y);
	}

	while (x->prev) {
		y = x->prev;
		x->prev = x->prev->prev;
		free(y);
	}

	free(x);

	return 0;
}
