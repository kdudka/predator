/**
* @file test-0031.c
*
* @brief Initialized 1D array that contains a structure which contains 2D array that contains
*        a structure.
*/

int main(int argc, const char *argv[]) {
	struct nested_struct {
		double c;
	};


	struct my_struct {
		int a;
		struct nested_struct b[2][2];
	};

	struct my_struct array[2] = { {1, { { {3.5} , {3.6}}, {{3.7}, {3.8}}}},
								  {2, { {{4.5}, {4.6}}, {{18.9}, {18.95}}}} };

	return 0;
}

