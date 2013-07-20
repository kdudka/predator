/**
* @file test-0032.c
*
* @brief Uninitialized 1D array that contains a structure which contains 2D array that contains
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

	int a = array[0].a;
	double c = array[0].b[0][0].c;
	c = array[0].b[0][1].c;
	c = array[0].b[1][0].c;
	c = array[0].b[1][1].c;

	a = array[0].a;
	c = array[0].b[0][0].c;
	c = array[0].b[0][1].c;
	c = array[0].b[1][0].c;
	c = array[0].b[1][1].c;

	return 0;
}

