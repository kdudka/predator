
/*
 * Test of an potentially unsound behaviour for setjmp/longjmp
 */

#include <setjmp.h>
#include <stdlib.h>

static jmp_buf buf;

int main()
{
	int x = 1;
	int* px = &x;

	if (!setjmp(buf))
	{	// executed when set
		px = NULL;

		longjmp(buf, 1);
	}
	else
	{	// executed after return
		*px = 0;    // invalid write here
	}

	return 0;
}
