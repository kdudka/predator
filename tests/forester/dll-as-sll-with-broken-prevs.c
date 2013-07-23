/*
 * A doubly linked list that is manipulated in such a way that only keeps the
 * SLL shape ok (i.e. prev links are broken)
 */

#include <stdlib.h>
#include <verifier-builtins.h>

int main()
{
	struct T
	{
		struct T* next;
		struct T* prev;
	};

	struct T* x = NULL;
	struct T* y = NULL;
	struct T* z = NULL;

	x = malloc(sizeof(struct T));
	x->next = NULL;
	x->prev = NULL;

	// create the DLL
	while (__VERIFIER_nondet_int())
	{
		y = malloc(sizeof(struct T));
		y->next = x;
		x->prev = y;
		y->prev = NULL;
		x = y;
	}

	// 'x' is now the head

	// now we randomly erase a few elements, keeping only the 'next' pointers
	// consistent
	while (__VERIFIER_nondet_int())
	{
		y = x;

		while (y->next && __VERIFIER_nondet_int())
		{
			y = y->next;
		}

		if (y->next)
		{
			z = y->next->next;
			free(y->next);
			y->next = z;
		}
	}

	while (x->next)
	{
		y = x->next;
		x->next = x->next->next;
		free(y);
	}

	free(x);

	return 0;
}
