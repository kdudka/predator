/**
* @file test-0052.c
*
* @brief Simple loop: Infinite loop.
*/

int main(int argc, const char *argv[]) {
	signed char i = 0;
	int a = 0;

	while (i < 10) {
		if (argc) {
			++a;
		} else {
			--a;
		}
		++i;
	}

	return 0;
}

