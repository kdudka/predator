/**
* @file test-0007.c
*
* @brief Assignment of initialized structure to structure one by one item.
*/

int main(int argc, const char *argv[]) {

	struct my_struct{
		int a;
		float b;
		double c;
	} x;

	struct my_struct y = {.a = 1, .b = -78.9, .c = 7.9};

	x.a = y.a;
	x.b = y.b;
	x.c = y.c;

	return 0;
}

