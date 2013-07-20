/**
* @file test-0025.c
*
* @brief Initialized 1D array that contains a structure which contains 1D array that contains
*        a structure.
*/

int main(int argc, const char *argv[]) {
	struct nested_struct {
		int c;
		signed char d;
	};

	struct my_struct {
		int a;
		struct nested_struct b[2];
	};

	struct my_struct array[2] = { {1, {{4, 1}, {6, -1}}},
								  {2, {{8, 3}, {9, -7}}} };

	return 0;
}

