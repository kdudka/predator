/**
* @file test-0008.c
*
* @brief Assignment of initialized structure to structure.
*/

int main(int argc, const char *argv[]) {

	struct my_struct{
		int a;
		float b;
		double c;
	} x;

	struct my_struct y = {.a = 1, .b = -78.9, .c = 7.9};
	x = y;

	return 0;
}

