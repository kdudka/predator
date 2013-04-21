/**
* @file test-0028.c
*
* @brief Initialized 1D array that contains a structure which contains 2D array.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b[2][2];
	};

	struct my_struct array[2] = { {1, { {3.5, 3.6}, {3.7, 3.8}}},
								  {2, { {4.5, 4.6}, {18.9, 18.95}}} };

	return 0;
}

