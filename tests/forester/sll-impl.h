/**
 * Procedures implementing a singly linked list
 */

typedef int Data;

typedef struct SLLType
{
	struct SLLType*  next;
	Data             data;
} SLL;

#if 0
#define push_front(list, dt) do     \
{                                   \
	__VERIFIER_assert(NULL != list);  \
	                                  \
	SLL* elem = malloc(sizeof(SLL));  \
	__VERIFIER_assert(NULL != elem);  \
	                                  \
	elem->next = NULL;                \
	elem->data = dt;                  \
	                                  \
	elem->next = *list;               \
	*list = elem;                     \
} while (0)

#define pop_front(list) do          \
{                                   \
	__VERIFIER_assert(NULL != list);  \
	                                  \
	SLL* tmp = *list;                 \
	*list = (*list)->next;            \
	free(tmp);                        \
} while (0)
#endif

#define empty(list) (NULL == (list))

// commented out due to Forester bug in processing function calls and performing
// too rough abstraction
#if 0
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
#endif
