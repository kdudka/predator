/**
* @file test-0029.c
*
* @brief Uninitialized 1D array that contains a structure which contains 2D array.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b[2][2];
	};

	struct my_struct array[2];

	int a = array[0].a + array[1].a;
	int b1 = array[0].b[0][0] + array[0].b[0][1] + array[0].b[0][0] + array[0].b[0][1];
	int b2 = array[1].b[0][0] + array[1].b[0][1] + array[1].b[0][0] + array[1].b[0][1];

	return 0;
}

