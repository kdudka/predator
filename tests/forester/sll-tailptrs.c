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

int main()
{
	ListTail* tail = malloc(sizeof(ListTail));

	tail->next = NULL;
	tail->tail = tail;

	ListTail* list = tail;
	ListTail* tmp = NULL;

	while (__VERIFIER_nondet_int())
	{
		ListTail* tmp = malloc(sizeof(ListTail));
		tmp->next = list;
		tmp->tail = tail;
		list = tmp;
	}

	tail = NULL;

	tmp = list;

	while (tmp != NULL)
		tmp = tmp->next;

	while (list != NULL)
	{
		tmp = list;
		list = list->next;
		free(tmp);
	}

	return 0;
}

