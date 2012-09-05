/*
 * Attempt to allocate zero memory size
 */

#include <stdlib.h>

int main()
{
	void* x = malloc(0);

	return 0;
}
