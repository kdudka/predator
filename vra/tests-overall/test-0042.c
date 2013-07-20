/**
* @file test-0042.c
*
* @brief Logical and/or/xor/not.
*/

#include <stdbool.h>

int main(int argc, const char *argv[]) {
	int x = 12;
	float y = 0.0;

	// &&
	bool a = x && x;
	bool b = x && y;
	bool c = y && x;
	bool d = y && y;

	// ||
	bool e = x || x;
	bool f = x || y;
	bool g = y || x;
	bool h = y || y;

	// !
	bool i = !x;
	bool j = !y;

	return 0;
}
