/**
* @file test-0022.c
*
* @brief Initialized 1D array that contains a structure which contains 1D array.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b[2];
	};

	struct my_struct array[2] = { {1, {3.5, 3.6} }, {2, {4.5, 4.6}} };

	return 0;
}

