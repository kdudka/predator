/**
 * A tree with subroot pointers
 */

#include <stdbool.h>
#include <stdlib.h>
#include <verifier-builtins.h>

typedef struct TTree
{
	struct TTree* left;
	struct TTree* right;
	struct TTree* root;
	bool isSubRoot;
} Tree;

int main()
{
	Tree* tree = NULL;
	Tree** tr;

	// create the root
	tree = malloc(sizeof(*tree));
	tree->left  = NULL;
	tree->right = NULL;
	tree->isSubRoot = true;
	tree->root = tree;

	while (__VERIFIER_nondet_int())
	{	// create arbitrary tree
		tr = &tree;

		Tree* prevRoot = NULL;

		while (NULL != *tr)
		{	// find any leaf

			if ((*tr)->isSubRoot)
			{
				prevRoot = *tr;
			}

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
		(*tr)->isSubRoot = __VERIFIER_nondet_int();
		(*tr)->root = prevRoot;
	}

	while (NULL != tree)
	{	// while there are still some remains of the tree
		tr = &tree;

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

	return 0;
}

