/**
* @file test-0023.c
*
* @brief Uninitialized 1D array that contains a structure which contains 1D array.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b[2];
	};

	struct my_struct array[2];

	int a = array[0].a;
	double b = array[1].b[0];

	return 0;
}

