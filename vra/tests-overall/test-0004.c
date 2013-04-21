/**
* @file test-0004.c
*
* @brief Assignment of structure one by one item.
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

	y.a = x.a;
	y.b = x.b;
	y.c = x.c;

	return 0;
}

