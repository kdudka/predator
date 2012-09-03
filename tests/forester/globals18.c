/*
 * Global variables
 */

#include <stdlib.h>

struct T {
	struct T* next;
};

struct T* x = NULL;

int main()
{
	x = malloc(sizeof(*x));

	// the analyser should report garbage here

	return 0;
}
