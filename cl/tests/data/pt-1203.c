/* taken from tests/forester-regre/test-f0019.c */
#include "include/pt.h"

/*
 * Simple tail recursion
 *
 * boxes:
 */

#include <stdlib.h>

struct list_type {

	struct list_type* next;
	int data;

};

// creates a list of arbitrary length
struct list_type* build_list() {

	struct list_type* x = NULL, *y;

	while (__nondet()) {
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

	return (__nondet())?(list):(lookup_list(list->next));

}

int main(int argc, char* argv[]) {

	struct list_type* list = build_list();

	struct list_type* el = lookup_list(list);

	free_list(list);

    ___cl_pt_build_fail();

	return 0;
}
