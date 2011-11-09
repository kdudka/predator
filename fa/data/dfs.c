/*
 * Labelling a depth-first search path in a tree
 *
 * boxes:
 */

#include <stdlib.h>

int __nondet();

int main() {

	struct TreeNode {
		struct TreeNode* left;
		struct TreeNode* right;
		struct TreeNode* parent;
		int data;
	};

	struct StackItem {
		struct StackItem* next;
		struct TreeNode* node;
	};

	struct TreeNode* root = malloc(sizeof(*root)), *n, *z;
	root->left = NULL;
	root->right = NULL;
	root->parent = NULL;
	root->data = 0;

	while (__nondet()) {
		n = root;
		while (n->left && n->right) {
			if (__nondet())
				n = n->left;
			else
				n = n->right;
		}

		if (!n->left && __nondet()) {
			z = malloc(sizeof(struct TreeNode));
			z->left = NULL;
			z->right = NULL;
			z->parent = n;
			z->data = 0;
			n->left = z;
		}
		if (!n->right && __nondet()) {
			z = malloc(sizeof(struct TreeNode));
			z->left = NULL;
			z->right = NULL;
			z->parent = n;
			z->data = 0;
			n->right = z;
		}
//		z = NULL;
	}

	n = root;
	while (n) {
		z = n->left;
		if (z && (z->data == 0)) {
			n = z;
		}
		else {
			z = n->right;
			if (z && (z->data == 0)) {
				n = z;
			}
			else {
				n->data = 1;
				n = n->parent;
			}
		}
//		z = NULL;
	}
//	struct TreeNode* pred;
/*
	while (root) {
		n = root;
		while (n->left || n->right) {
			if (n->left)
				n = n->left;
			else
				n = n->right;
		}
		z = n->parent;
		if (z) {
			if (n == z->left)
				z->left = NULL;
			else
				z->right = NULL;
			z = NULL;
		} else
			root = NULL;
		free(n);
	}
*/

	struct StackItem* s = malloc(sizeof(*s)), *st;
	s->next = NULL;
	s->node = root;

	while (s) {
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
