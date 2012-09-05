/*
 * Attempt to allocate negative memory size
 */

#include <stdlib.h>

int main()
{
	void* x = malloc(-1);

	return 0;
}
