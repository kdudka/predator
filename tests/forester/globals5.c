/*
 * Global variables
 */

#include <stdlib.h>

struct T
{
	struct T* next;
};

struct T y;

struct T x = {.next = &y};
struct T y = {.next = &x};

int main()
{
	if (!x.next)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	if (!y.next)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	x.next = NULL;

	if (x.next)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	if (!y.next)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	if (y.next->next)
	{
		// the analyzer should not report a bug here
		*(int*)NULL = 0;
	}

	return 0;
}
