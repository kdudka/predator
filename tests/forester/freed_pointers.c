/*
 * Equality of freed pointers
 */

#include <stdlib.h>

int main()
{
	void* x = malloc(1);

	void* y = x;
	free(x);
	if (x != y)
	{	// should not get here, the value of x and y is still the same
		*(void*)0;
	}

	return 0;
}
