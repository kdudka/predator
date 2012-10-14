/*
 * Executing abort() before freeing memory.
 */

#include <stdlib.h>

int main()
{
	struct
	{
		int x;
	}* p = malloc(sizeof(*p));

	abort();

	return 0;
}
