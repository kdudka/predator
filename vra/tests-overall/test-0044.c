/**
* @file test-0044.c
*
* @brief Unary-/abs/float/min/max.
*/

#include <stdbool.h>

int main(int argc, const char *argv[]) {
	// Unary minus.
	int a = 25;
	int b = -a;

	// Absolute value.
	int c = abs(a);
	int d = abs(b);

	// Float.
	float e = d;

	// Min.
	int f = (a < b) ? a : b;

	// Max.
	int g = (a > b) ? a : b;

	return 0;
}


