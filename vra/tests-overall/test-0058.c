/**
* @file test-0058.c
*
* @brief Fibonacci computation II.
*/

#define N 10

int main(int argc, const char *argv[]) {
	int fibonacci[N];
	fibonacci[0] = fibonacci[1] = 1;

    int i;
	for (i = 2; i < 10; i++) {
		fibonacci[i] = fibonacci[i-1] + fibonacci[i-2];
	}

	return 0;
}


