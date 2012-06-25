/*
 * Global variables
 */

#include <stdlib.h>

struct T
{
	struct T* next;
};

struct T x = {.next = NULL};

int main()
{
	if (!x.next)
	{
		// the analyzer should report a bug here
		*(int*)NULL = 0;
	}

	return 0;
}

