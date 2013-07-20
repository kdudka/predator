/**
* @file test-0020.c
*
* @brief Uninitialized 1D array that contains a structure.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b;
	};

	struct my_struct array[2];

	int x = array[0].a + array[1].a;
	double y = array[0].b + array[1].b;

	return 0;
}


