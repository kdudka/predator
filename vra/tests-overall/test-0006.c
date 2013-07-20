/**
* @file test-0006.c
*
* @brief Assignment of uninitialized structure to structure.
*/

int main(int argc, const char *argv[]) {

	struct {
		int a;
		float b;
		double c;
	} x, y;

	y = x;

	return 0;
}

