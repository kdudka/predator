/*
 * Singly linked list with tail pointers
 *
 * boxes:
 */

#include <stdlib.h>
#include <verifier-builtins.h>

typedef struct TListTail
{
	struct TListTail* next;
	struct TListTail* tail;
} ListTail;

typedef struct TList
{
	struct TList* next;
	ListTail* lower;
} List;

int main()
{
	List* x = NULL;
	List* y = NULL;

	ListTail* tail = NULL;
	ListTail* tmp  = NULL;

	while (__VERIFIER_nondet_int())
	{
		y = malloc(sizeof(List));
		y->next = x;
		x = y;

		tail = malloc(sizeof(ListTail));

		tail->next = NULL;
		tail->tail = tail;

		x->lower = tail;
		tmp = NULL;

		while (__VERIFIER_nondet_int())
		{
			tmp = malloc(sizeof(ListTail));
			tmp->next = x->lower;
			tmp->tail = tail;
			x->lower = tmp;
		}

		tail = NULL;
	}

	while (x != NULL)
	{
		while (x->lower != NULL)
		{
			tmp = x->lower;
			x->lower = x->lower->next;
			free(tmp);
		}

		y = x;
		x = x->next;
		free(y);
	}

	return 0;
}

