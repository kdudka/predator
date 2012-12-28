/*
 * Calling a few functions with abstraction inside
 */

#include <stdbool.h>
#include <verifier-builtins.h>

void bar()
{
	while (__VERIFIER_nondet_int())
	{ }
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
