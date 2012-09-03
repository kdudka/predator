/*
 * Global variables
 */

#include <stdlib.h>

struct T
{
	struct T* next;
	struct T* prev;
};

struct T x;

int main()
{
	if (x.next)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	if (x.prev)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	return 0;
}
