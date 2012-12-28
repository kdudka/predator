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

	// create a list with a single marked element
	while (__VERIFIER_nondet_int())
	{
		push_front(&x, 0);
	}

	push_front(&x, 1);

	while (__VERIFIER_nondet_int())
	{
		push_front(&x, 0);
	}

	SLL* y = x;

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
		pop_front(&x);
	}

	return 0;
}
