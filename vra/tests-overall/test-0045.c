/**
* @file test-0045.c
*
* @brief trunc_mod/trunc_div/exact_div/rdiv
*/

#include <limits.h>

int main(int argc, const char *argv[]) {
	// Trunc div.
	unsigned char a = 10 / 0; // Undefined behaviour.
	int b = INT_MIN / -1;     // Undefined behaviour.
	int c = -10;
	int d = 3;
	int e = c / d;
	unsigned f = 10;
	unsigned g = 3;
	unsigned h = f / g;
	unsigned char i = 10;
	signed char j = -3;
	int k = i / j;

	// Exact div.
	int l = -10;
	int m = 2;
	int n = l / m;
	unsigned o = 10;
	unsigned p = 2;
	unsigned r = o / p;
	unsigned char s = 10;
	signed char t = -2;
	int u = s / t;

	return 0;
}

