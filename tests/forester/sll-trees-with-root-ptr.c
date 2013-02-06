/**
 * Singly linked list of trees with root pointers
 */

#include <stdlib.h>
#include <verifier-builtins.h>

typedef struct TTree
{
	struct TTree* left;
	struct TTree* right;
	struct TTree* root;
} Tree;

typedef struct TList
{
	Tree* tree;
	struct TList* next;
} List;

int main()
{
	List* list = NULL;
	Tree** tr;
	List* tmp;

	while (__VERIFIER_nondet_int())
	{
		tmp = malloc(sizeof(*tmp));
		tmp->next = list;
		tmp->tree = NULL;

		while (__VERIFIER_nondet_int())
		{	// create arbitrary tree
			tr = &tmp->tree;

			while (NULL != *tr)
			{	// find any leaf
				if (__VERIFIER_nondet_int())
				{
					tr = &(*tr)->left;
				}
				else
				{
					tr = &(*tr)->right;
				}
			}

			*tr = malloc(sizeof(**tr));
			(*tr)->left = NULL;
			(*tr)->right = NULL;
			(*tr)->root = tmp->tree;
		}

		list = tmp;
	}

	while (NULL != list)
	{	// delete the list
		while (NULL != list->tree)
		{	// while there are still some remains of the tree
			tr = &list->tree;

			while ((NULL != (*tr)->left) || (NULL != (*tr)->right))
			{
				if (NULL != (*tr)->left)
				{
					tr = &(*tr)->left;
				}
				else
				{
					tr = &(*tr)->right;
				}
			}

			free(*tr);
			*tr = NULL;
		}

		tmp = list->next;
		free(list);
		list = tmp;
	}

	return 0;
}
