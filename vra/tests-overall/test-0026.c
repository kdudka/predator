/**
* @file test-0026.c
*
* @brief Uninitialized 1D array that contains a structure which contains 1D array that contains
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

	int a = array[0].a;
	int c = array[0].b[0].c;
	signed char d = array[1].b[1].d;

	return 0;
}

