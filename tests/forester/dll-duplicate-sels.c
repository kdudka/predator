/*
 * Doubly linked list construction and deletion.
 */

#include <stdlib.h>
#include <verifier-builtins.h>

int main() {

	struct T {
		struct T* next1;
		struct T* next2;
		struct T* prev1;
		struct T* prev2;
		int data;
	};

	struct T* x = NULL;
	struct T* y = NULL;

	x = malloc(sizeof(struct T));
	x->next1 = NULL;
	x->next2 = NULL;
	x->prev1 = NULL;
	x->prev2 = NULL;

	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(struct T));
		y->next1 = x;
		y->next2 = x;
		x->prev1 = y;
		x->prev2 = y;
		y->prev1 = NULL;
		y->prev2 = NULL;
		x = y;
	}

	while (x) {
		y = x->next1;
		free(x);
		x = y;
	}

	return 0;
}
