/**
* @file test-0019.c
*
* @brief Initialized 1D array that contains a structure.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b;
	};

	struct my_struct array[2] = { {1, 1.35}, {2, 2.35}};

	return 0;
}

