/*
 * A DLL with extends.
 */

#include <stdlib.h>
#include <verifier-builtins.h>

typedef struct TSmallData
{
	char x;
} SmallData;

typedef struct TBigData
{
	int x;
} BigData;

typedef struct TNode
{
	struct TNode* next;
	struct TNode* prev;
	BigData* pBigData;
	SmallData smallData;
} Node;

int main()
{
	Node* list = NULL;
	Node* y = NULL;

	y = malloc(sizeof(*y));
	y->next = NULL;
	y->prev = NULL;
	y->pBigData = NULL;
	list = y;

	while (__VERIFIER_nondet_int())
	{
		y = malloc(sizeof(*y));
		y->next = list;
		list->prev = y;

		if (__VERIFIER_nondet_int())
		{
			y->pBigData = malloc(sizeof(*y->pBigData));
		}
		else
		{
			y->pBigData = NULL;
		}

		list = y;
	}

	while (NULL != list)
	{
		y = list;
		list = list->next;

		if (NULL != y->pBigData)
		{
			free(y->pBigData);
		}

		free(y);
	}

	return 0;
}
