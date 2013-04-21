/**
* @file test-0005.c
*
* @brief Assignment of structure to structure.
*/

int main(int argc, const char *argv[]) {

	struct {
		int a;
		float b;
		double c;
	} x, y;

	x.a = 2;
	x.b = 4.0;
	x.c = 8.45;

	y = x;

	return 0;
}

