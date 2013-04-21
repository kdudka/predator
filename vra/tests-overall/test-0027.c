/**
* @file test-0027.c
*
* @brief Filled 1D array that contains a structure which contains 1D array that contains
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

	struct my_struct array[2];

	array[0].a = 14;
	array[0].b[0].c = 20;
	array[0].b[1].c = 21;
	array[0].b[0].d = 126;
	array[0].b[1].d = 127;

	array[1].a = 15;
	array[1].b[0].c = 23;
	array[1].b[1].c = 24;
	array[1].b[0].d = 100;
	array[1].b[1].d = 99;

	return 0;
}

