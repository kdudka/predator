#include <stdlib.h>

//int __nondet();
#define __nondet() 1

typedef enum {red,black} Colour;

//rotate(x,left,right) gives rotate left
//rotate(x,right,left) gives rotate right
#define ROTATE(x,right,left)							\
    r = x->right;                                                               \
    x->right = r->left;                                                         \
    r->left->parent = x;                                                        \
    r->parent = x->parent;                                                      \
    if (x->parent == &null) {        	                                        \
	    root = r;                                                           \
	    if (x == x->parent->left) {x->parent->left = r;}                    \
	    else {x->parent->right = r;}                                        \
    }                                                                           \
    r->left = x;                                                                \
    x->parent = r;                                                              \
    r = &null;


int main() {

    struct T {
        struct T* left;
	struct T* right;
	struct T* parent;
	Colour colour;
    };

    struct T* r; //an auxliary var used in the ROTATE macro

    //the sentinell node used instead of NULL
    struct T null = {.left=NULL,.right=NULL,.parent=NULL,.colour=black};
    
    //start with an empty tree
    struct T* root = &null;

    //insert a random number of random nodes 
    while (__nondet()) {
	    //create a new node z 
	    struct T* z = malloc(sizeof(struct T));
	    struct T* y = &null;
	    struct T* x = root;
	    //insert z as a random leaf
	    while (x != &null) {
		    y = x;   
		    if (__nondet()) {x = x->left;}
		    else {x = x->right;}
	    }
	    z->parent = y;
	    //make z root if the tree is empty
	    if (y == &null) {root = z;}
            else {
		    if (__nondet()) {y->left = z;}
		    else {y->right = z;}
	    }
	    //make z red
	    z->left = &null;
	    z->right = &null;
	    z->colour = red;


	    //rebalance the tree
	    while (z->parent->colour == red) {
		    if (z->parent == z->parent->parent->left) {
			    y = z->parent->parent->right;
			    if (y->colour == red) {
				    z->parent->colour = black;
				    z->colour = black;
				    z->parent->parent->colour = red;
				    z = z->parent->parent;
			    }
			    else {
				    if (z == z->parent->right) {
					    z = z->parent;
					    //rotate left (z)
					    ROTATE(z,left,right)
				    }
				    z->parent->colour = black;
				    z->parent->parent->colour = red;
				    //rotate right
				    ROTATE(z->parent->parent,right,left)
			    }
		    } 
		    else {
			    y = z->parent->parent->left;
			    if (y->colour == red) {
				    z->parent->colour = black;
				    z->colour = black;
				    z->parent->parent->colour = red;
				    z = z->parent->parent;
			    }
			    else {
				    if (z == z->parent->left) {
					    z = z->parent;
					    //rotate right (z)
					    ROTATE(z,right,left)
				    }
				    z->parent->colour = black;
				    z->parent->parent->colour = red;
				    //rotate left
				    ROTATE(z->parent->parent,left,right)
			    }
		    }
	    }
	    root->colour=black;
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


	

