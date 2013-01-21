/**
 * A tree with pointer-to-pointer constructor.
 *
 * @note: currently fails because of rough Forester abstraction: during
 * deallocation of the tree, it makes a mistake and generalizes 'tr' that is
 * a pointer known to point to a non-NULL location into a pointer which may
 * point to a non-NULL location.
 */

#include <stdlib.h>
#include <verifier-builtins.h>

typedef struct TTree
{
	struct TTree* left;
	struct TTree* right;
} Tree;

int main()
{
	Tree* tree = NULL;
	Tree** tr;

	while (__VERIFIER_nondet_int())
	{	// create arbitrary tree
		tr = &tree;

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
