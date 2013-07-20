/**
* @file test-0014.c
*
* @brief Filled 1D array. Element of an array assigned to a simple variable.
*/

int main(int argc, const char *argv[]) {
	double a[10];

	a[0] = 0;
	a[1] = 0.1;
	a[2] = 0.2;
	a[3] = 0.3;
	a[4] = 0.4;
	a[5] = 5.0;
	a[6] = 5.1;
	a[7] = 5.2;
	a[8] = 8;
	a[9] = 9;

	double b = a[0];

	return 0;
}

