/*
 * Global variables
 */

#include <stdlib.h>

struct T
{
	struct T* next;
};

struct T* x = NULL;

int main()
{
	if (x)
	{
		*(int*)NULL = 0;
	}

	return 0;
}
