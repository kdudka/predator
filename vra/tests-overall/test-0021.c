/**
* @file test-0021.c
*
* @brief Filled 1D array that contains a structure.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b;
	};

	struct my_struct array[2];

	array[0].a = 1;
	array[0].b = 3.5;
	array[1].a = 2;
	array[1].b = 5.5;

	return 0;
}

