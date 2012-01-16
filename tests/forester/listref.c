/*
 * Singly linked list of concrete length
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

	y = malloc(sizeof(*y));
	y->next = x;
	x = y;


	y = malloc(sizeof(*y));
	y->next = x;
	x = y;

	y = x;
	while (y != NULL)
		y = y->next;

	y = x;

	x = y;
	y = y->next;
	free(x);

	if (y) {
		x = y;
		y = y->next;
		free(x);
	}

	return 0;
}
