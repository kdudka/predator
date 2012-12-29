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

	while (y != NULL) {
		if (__VERIFIER_nondet_int()) {
	    struct T* z = malloc(sizeof(struct T));
	    z->next = y->next;
	    z->prev = y;
	    y->next = z;
	    if (z->next)
		z->next->prev = z;
	    break;
	}
	y = y->next;
    }

    while (x != NULL) {
	y = x;
        x = x->next;
	free(y);
    }

    return 0;

}
