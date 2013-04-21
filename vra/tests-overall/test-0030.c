/**
* @file test-0030.c
*
* @brief Filled 1D array that contains a structure which contains 2D array.
*/

int main(int argc, const char *argv[]) {
	struct my_struct {
		int a;
		double b[2][2];
	};

	struct my_struct array[2];

	array[0].a = 1;
	array[0].b[0][0] = 3.5;
	array[0].b[0][1] = 3.6;
	array[0].b[0][0] = 3.7;
	array[0].b[0][1] = 3.8;

	array[1].a = 2;
	array[1].b[0][0] = 4.5;
	array[1].b[0][1] = 4.6;
	array[1].b[0][0] = 18.9;
	array[1].b[0][1] = 18.95;

	return 0;
}

