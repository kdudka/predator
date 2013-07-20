/**
* @file test-0048.c
*
* @brief Simple loop.
*/

int main(int argc, const char *argv[]) {
	int buffer[10];
	int i = 0;
	int a = 0;

	while (i < 10) {
		buffer[i] = i;
		++i;
		++a;
	}

	return 0;
}

