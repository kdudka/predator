/**
 * A singly linked list with a random insertion constructor.
 */

#include <stdlib.h>
#include <verifier-builtins.h>

typedef struct TSLL
{
	struct TSLL* next;
} SLL;

int main()
{
	SLL* list = NULL;
	SLL** ptr;
	SLL* tmp;

	while (__VERIFIER_nondet_int())
	{	// create randomly a list
		ptr = &list;

		while ((NULL != *ptr) && __VERIFIER_nondet_int())
		{	// find a place in the list
			ptr = &(*ptr)->next;
		}

		tmp = *ptr;
		*ptr = malloc(sizeof(**ptr));
		(*ptr)->next = tmp;
	}

	while (NULL != list)
	{
		tmp = list;
		list = list->next;
		free(tmp);
	}

	return 0;
}
