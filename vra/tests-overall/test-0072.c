/**
* @file test-0071.c
*
* @brief More functions.
*/

#include <limits.h>
#include <stdbool.h>

bool isMax(signed char c);

int main(int argc, const char *argv[]) {
	int c = isMax(CHAR_MAX);
	return 0;
}

bool isMax(signed char c)
{
	bool a;
	if (c == CHAR_MAX) {
		a = true;
	} else {
		a = false;
	}

	return a;
}
