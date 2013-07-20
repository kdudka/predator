/**
* @file test-0017.c
*
* @brief Filled 2D array. Element of an array assigned to a simple variable.
*/

int main(int argc, const char *argv[]) {
	double a[2][2];

	a[0][0] = 0;
	a[0][1] = 0.1;
	a[1][0] = 0.2;
	a[1][1] = 0.3;

	int b = a[0][1];

	return 0;
}

