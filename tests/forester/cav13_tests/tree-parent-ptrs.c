/*
 * Tree with parent pointers destruction using a stack
 *
 * boxes: treewpp.boxes
 */

#include <stdlib.h>
#include <verifier-builtins.h>

int main() {

	struct TreeNode {
		struct TreeNode* left;
		struct TreeNode* right;
		struct TreeNode* parent;
	};

	struct StackItem {
		struct StackItem* next;
		struct TreeNode* node;
	};

	struct TreeNode* root = malloc(sizeof(*root)), *n;
	root->left = NULL;
	root->right = NULL;
	root->parent = NULL;

	while (__VERIFIER_nondet_int()) {
		n = root;
		while (n->left && n->right) {
			if (__VERIFIER_nondet_int())
				n = n->left;
			else
				n = n->right;
		}
		if (!n->left && __VERIFIER_nondet_int()) {
			n->left = malloc(sizeof(*n));
			n->left->left = NULL;
			n->left->right = NULL;
			n->left->parent = n;
		}
		if (!n->right && __VERIFIER_nondet_int()) {
			n->right = malloc(sizeof(*n));
			n->right->left = NULL;
			n->right->right = NULL;
			n->right->parent = n;
		}
	}

	n = NULL;
	/*
		 struct TreeNode* parent = root, * right = NULL, * curr;
		 while (parent != NULL) {
		 curr = parent->left;
		 if (curr != NULL) {
		 while (curr != right && curr->right != NULL)
		 curr = curr->right;

		 if (curr != right) {
		 curr->right = parent;
		 parent = parent->left;
		 continue;
		 } else
		 curr->right = NULL;
		 } else
		 right = parent;
		 parent = parent->right;
		 }
		 */
	/*
		 if (!root)
		 return;

		 struct TreeNode* current = root, * from = NULL;
		 while (current != root || from == NULL) {
		 while (current->left)
		 current = current->left;

		 if (current->right) {
		 current = current->right;
		 continue;
		 }

		 from = current;
		 current = current->parent;

		 if (from == current->left) {
		 current = current->right;
		 } else {
		 while (from != current->left && current != root) {
		 from = current;
		 current = current->parent;
		 }
		 if (current == root && from == current->left && current->right)
		 current = current->right;
		 }

		 }
		 */
	//    struct TreeNode* pred;
	/*
		 while (root) {
	//	pred = NULL;
	n = root;
	while (n->left || n->right) {
	//	    pred = n;
	if (n->left)
	n = n->left;
	else
	n = n->right;
	}
	if (n->parent) {
	if (n == n->parent->left)
	n->parent->left = NULL;
	else
	n->parent->right = NULL;
	} else
	root = NULL;
	free(n);
	}
	*/

	struct StackItem* s = malloc(sizeof(*s)), *st;
	s->next = NULL;
	s->node = root;

	while (s != NULL) {
		st = s;
		s = s->next;
		n = st->node;
		free(st);
		if (n->left) {
			st = malloc(sizeof(*st));
			st->next = s;
			st->node = n->left;
			s = st;
		}
		if (n->right) {
			st = malloc(sizeof(*st));
			st->next = s;
			st->node = n->right;
			s = st;
		}
		free(n);
	}

	return 0;

}

/*
void BinarySearchTree::iterate_constant_space() {
  BinarySearchTree *current = this, *from = NULL;
  current->visit();
  while (current != this || from == NULL) {
    while (current->left) {
      current = current->left;
      current->visit();
    }
    if (current->right) {
      current = current->right;
      current->visit();
      continue;
    }
    from = current;
    current = current->parent;
    if (from == current->left) {
      current = current->right;
      current->visit();
    } else {
      while (from != current->left && current != this) {
        from = current;
        current = current->parent;
      }
      if (current == this && from == current->left && current->right) {
        current = current->right;
        current->visit();
      }
    }
  }
}
*/

