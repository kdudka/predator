/**
* @file test-0053.c
*
* @brief Simple loop.
*/

int main(int argc, const char *argv[]) {

	char i, a;
	a = i = 0;
	int c = 1;

	while (i < 10) {
		a = a + 2;
		++i;
		c = c * 2;
	}

	return 0;
}

