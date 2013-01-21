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
	List* tmp;
	Tree* tree;
	Tree* pred;

	while (__VERIFIER_nondet_int())
	{	// list of arbitrary length
		tmp = malloc(sizeof(*tmp));
		tmp->next = list;
		tmp->tree = malloc(sizeof(*tmp->tree));
		tmp->tree->left  = NULL;
		tmp->tree->right = NULL;
		tmp->tree->root  = tmp->tree;

		while (__VERIFIER_nondet_int())
		{	// create arbitrary tree
			tree = tmp->tree;

			while ((NULL != tree->left) && (NULL != tree->right))
			{
				if (__VERIFIER_nondet_int())
				{
					tree = tree->left;
				}
				else
				{
					tree = tree->right;
				}
			}

			if ((NULL == tree->left) && __VERIFIER_nondet_int())
			{
				tree->left = malloc(sizeof(*tree->left));
				tree->left->left  = NULL;
				tree->left->right = NULL;
				tree->left->root  = tmp->tree;
			}
			if ((NULL == tree->right) && __VERIFIER_nondet_int())
			{
				tree->right = malloc(sizeof(*tree->right));
				tree->right->left  = NULL;
				tree->right->right = NULL;
				tree->right->root  = tmp->tree;
			}
		}

		list = tmp;
	}

	while (NULL != list)
	{	// delete the list
		while (NULL != list->tree)
		{	// while there are still some remains of the tree
			tree = list->tree;
			pred = NULL;

			while ((NULL != tree->left) || (NULL != tree->right))
			{
				pred = tree;
				if (NULL != tree->left)
				{
					tree = tree->left;
				}
				else
				{
					tree = tree->right;
				}
			}

			if (NULL != pred)
			{
				if (tree == pred->left)
				{
					pred->left = NULL;
				}
				else
				{
					pred->right = NULL;
				}
			}
			else
			{
				list->tree = NULL;
			}

			free(tree);
		}

		tmp = list->next;
		free(list);
		list = tmp;
	}

	return 0;
}
