/**
* @file test-0033.c
*
* @brief Filled 1D array that contains a structure which contains 2D array that contains
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

	struct my_struct array[2];

	array[0].a = 1;
	array[0].b[0][0].c = 3.5;
	array[0].b[0][1].c = 3.6;
	array[0].b[1][0].c = 3.7;
	array[0].b[1][1].c = 3.8;

	array[1].a = 2;
	array[1].b[0][0].c = 4.5;
	array[1].b[0][1].c = 4.6;
	array[1].b[1][0].c = 18.9;
	array[1].b[1][1].c = 18.95;

	return 0;
}

