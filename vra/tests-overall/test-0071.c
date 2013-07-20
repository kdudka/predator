/**
* @file test-0071.c
*
* @brief Nested if-else statement with double type.
*/

int main(int argc, const char *argv[]) {
	long double f = 10;

	if (argc < 100) {
		f = f + argc;
		if (f > 50.125) {
			printf("f = %Lf", f);
		} else {
			f = 50.125;
		}
	} else {
		f = 40.225;
	}

	return 0;
}

