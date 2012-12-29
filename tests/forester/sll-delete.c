/*
 * Singly linked list deleting
 *
 * boxes:
 */

#include <stdlib.h>
#include <verifier-builtins.h>

int main() {

	struct T {
		struct T* next;
	};

	struct T* x = NULL;
	struct T* y = NULL;

	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(*y));
		y->next = x;
		x = y;
	}

	struct T* z = NULL;

	while (x != NULL) {
		if (__VERIFIER_nondet_int()) {
			if (z)
				z->next = x->next;
			else
				y = y->next;
			free(x);
			break;
		}
		z = x;
		x = x->next;
	}

	while (y != NULL) {
		x = y;
		y = y->next;
		free(x);
	}

	return 0;

}
