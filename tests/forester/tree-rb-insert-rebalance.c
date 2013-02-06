#include <stdlib.h>

int __nondet();
//#define __nondet() 1

typedef enum {RED,BLACK} Colour;

//rotate(X,left,right) gives rotate LEFT
//rotate(X,right,left) gives rotate RIGHT
//
#define ROTATE(X,LEFT,RIGHT)                                    \
    r = X->RIGHT;                                               \
    X->RIGHT = r->LEFT;                                         \
    r->LEFT->parent = X;                                        \
    r->parent = X->parent;                                      \
    if (X->parent == &null) {root = r;}                         \
    else {                                                      \
            if (X == X->parent->LEFT) {X->parent->LEFT = r;}    \
            else {X->parent->RIGHT = r;}                        \
    }                                                           \
    r->LEFT = X;                                                \
    X->parent = r;                                              \
    r = NULL;

#define LEFT_ROTATE(X) ROTATE(X,left,right)
#define RIGHT_ROTATE(X) ROTATE(X,right,left)


int main() {

	struct T {
		struct T* left;
		struct T* right;
		struct T* parent;
		Colour colour;
	};

	struct T* r; //an auxliary var used in the ROTATE macro

	//the sentinell node used instead of NULL
	struct T null = {.left=NULL,.right=NULL,.parent=NULL,.colour=BLACK};

	//start with an empty tree
	struct T* root = &null;

	//insert a random number of random nodes
	while (__nondet()) {
		if (root->colour != BLACK)
			*(int*)(NULL) = 0;
		//create a new node z
		struct T* z = root;
		struct T* y = &null;
		//insert z as a random leaf
		int lastChoice;
		while (z != &null) {
			y = z;
			if (__nondet()) {
				z = z->left;
				lastChoice = 0;
				//			    if (x == &null) {y->left = z;}
			}
			else {
				z = z->right;
				lastChoice = 1;
				//			    if (x == &null) {y->right = z;}
			}
		}
		z = malloc(sizeof(struct T));
		z->parent = y;
		//make z root if the tree is empty
		if (y == &null) {root = z;}
		else {
			if (lastChoice == 0) {y->left = z;}
			else {y->right = z; lastChoice = 0;}
		}
		y = NULL;
		//make z RED
		z->left = &null;
		z->right = &null;
		z->colour = RED;


		//rebalance the tree
		while (z->parent->colour == RED) {
			if (z->parent == z->parent->parent->left) {
				y = z->parent->parent->right;
				if (y->colour == RED) {
					z->parent->colour = BLACK;
					y->colour = BLACK;
					y = NULL;
					z->parent->parent->colour = RED;
					z = z->parent->parent;
				}
				else {
					y = NULL;
					if (z == z->parent->right) {
						z = z->parent;
						LEFT_ROTATE(z)
					}
					z->parent->colour = BLACK;
					z->parent->parent->colour = RED;
					y = z->parent->parent;
					//RIGHT_ROTATE(z->parent->parent)
					RIGHT_ROTATE(y)
						y = NULL;
				}
			}
			else {
				y = z->parent->parent->left;
				if (y->colour == RED) {
					z->parent->colour = BLACK;
					y->colour = BLACK;
					y = NULL;
					z->parent->parent->colour = RED;
					z = z->parent->parent;
				}
				else {
					y = NULL;
					if (z == z->parent->left) {
						z = z->parent;
						RIGHT_ROTATE(z)
					}
					z->parent->colour = BLACK;
					z->parent->parent->colour = RED;
					y = z->parent->parent;
					//LEFT_ROTATE(z->parent->parent)
					LEFT_ROTATE(y)
						y = NULL;
				}
			}
		}
		z = NULL;
		root->colour=BLACK;
	}

	//Test that:
	//there are no two succesive random nodes


	//kill the tree (using the aux var r)
	while (root != &null) {
		r = root;
		while (r->left != &null || r->right != &null) {
			if (r->left != &null)
				r = r->left;
			else
				r = r->right;
		}
		if (r->parent != &null) {
			if (r == r->parent->left)
				r->parent->left = &null;
			else
				r->parent->right = &null;
		} else
			root = &null;
		free(r);
	}

	return 42;
}
