/**
 * Procedures implementing a singly linked list
 */

typedef int Data;

typedef struct SLLType
{
	struct SLLType*  next;
	Data             data;
} SLL;

inline void push_front(SLL** list, Data data)
{
	// Preconditions
	__VERIFIER_assert(NULL != list);

	SLL* elem = malloc(sizeof(SLL));
	__VERIFIER_assert(NULL != elem);

	elem->next = NULL;
	elem->data = data;

	elem->next = *list;
	*list = elem;
}

inline void pop_front(SLL** list)
{
	// Preconditions
	__VERIFIER_assert(NULL != list);

	SLL* tmp = *list;
	*list = (*list)->next;
	free(tmp);
}

inline int empty(const SLL* list)
{
	if (NULL == list)
		return 1;

	return 0;
}
