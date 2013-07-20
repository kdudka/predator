/**
* @file test-0018.c
*
* @brief Unitialized 2D array. Element of an array assigned to a simple variable.
*/

int main(int argc, const char *argv[]) {
	int a[10][10];
	double b = a[0][9];

	return 0;
}


