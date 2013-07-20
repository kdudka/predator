/**
* @file test-0041.c
*
* @brief Comparisons: ==, !=, >, <, >=, <=.
*/

#include <stdbool.h>

int main(int argc, const char *argv[]) {
	int x = 12;
	float y = 13.0;

	// <
	bool a = x < y;
	bool b = y < x;

	// >
	bool c = x > y;
	bool d = y > x;
	bool e = x > x;

	// >=
	bool f = y >= x;
	bool g = x >= y;
	bool h = y >= y;

	// <=
	bool i = x <= y;
	bool j = y <= x;
	bool k = x <= x;

	// ==
	bool l = x == x;
	bool m = x == y;

	// !=
	bool n = x != x;
	bool o = x != y;

	return 0;
}
