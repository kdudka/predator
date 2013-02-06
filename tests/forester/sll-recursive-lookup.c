/*
 * Simple tail recursion
 *
 * boxes:
 */

#include <stdlib.h>
#include <verifier-builtins.h>

struct list_type {

	struct list_type* next;
	int data;

};

// creates a list of arbitrary length
struct list_type* build_list() {

	struct list_type* x = NULL, *y;

	while (__VERIFIER_nondet_int()) {
		y = malloc(sizeof(*y));
		y->next = x;
		x = y;
	}

	return x;

}

// frees a list
void free_list(struct list_type* list) {

	struct list_type* x;

	while (list) {
		x = list;
		list = list->next;
		free(x);
	}

}

// non-deterministically finds an element in a list
struct list_type* lookup_list(struct list_type* list) {

	if (!list)
		return NULL;

	return (__VERIFIER_nondet_int())?(list):(lookup_list(list->next));

}

int main(int argc, char* argv[]) {

	struct list_type* list = build_list();

	struct list_type* el = lookup_list(list);

	free_list(list);

	return 0;

}
