/**
* @file test-0062.c
*
* @brief Nested if-else statement.
*/

int main(int argc, const char *argv[]) {
	int i = 10;

	if (argc < 100) {
		i = i + argc;
		if (i > 50) {
			printf("i = %d", i);
		} else {
			i = 50;
		}
	} else {
		i = 40;
	}

	return 0;
}
