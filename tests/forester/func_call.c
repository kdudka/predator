/*
 * Calling a few functions
 */

#include <stdbool.h>
#include <verifier-builtins.h>

void bar()
{

}

void foo()
{
	bar();
}

int main()
{
	bool flag = false;

	foo();

	flag = true;

	foo();

	__VERIFIER_assert(flag);

	return 0;
}
