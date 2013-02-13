/**
 * Observer testing insertion into a singly linked list
 */

#include <stdbool.h>
#include <stdlib.h>
#include <verifier-builtins.h>

#include "sll-impl.h"

int main()
{
	SLL* x = NULL;
	SLL* y = NULL;

	// create a list with a single marked element
	while (__VERIFIER_nondet_int())
	{
		// inlined push_front
		y = malloc(sizeof(SLL));
		y->next = x;
		y->data = 0;
		x = y;
	}

	// inlined push_front
	y = malloc(sizeof(SLL));
	y->next = x;
	y->data = 1;
	x = y;

	while (__VERIFIER_nondet_int())
	{
		// inlined push_front
		y = malloc(sizeof(SLL));
		y->next = x;
		y->data = 0;
		x = y;
	}

	__VERIFIER_plot("observer-0001");

	y = x;

	bool found = false;

	// now check that the element is there in the list exactly once
	while (NULL != y)
	{
		if (1 == y->data)
		{
			__VERIFIER_assert(!found);

			found = true;
		}

		y = y->next;
	}

	__VERIFIER_assert(found);

	// delete the list
	while (!empty(x))
	{
		// inlined pop_front
		y = x;
		x = x->next;
		free(y);
	}

	return 0;
}
