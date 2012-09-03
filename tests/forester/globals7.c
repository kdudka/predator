/*
 * Global variables
 */

#include <stdlib.h>

struct T
{
	struct T* next;
};

struct T* x = 0;
struct T* y = NULL + 1;

int main()
{
	if (x)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	if (y)
	{
		// the analyzer should find a bug here
		*(int*)NULL = 0;
	}

	return 0;
}
