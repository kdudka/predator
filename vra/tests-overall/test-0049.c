/**
* @file test-0049.c
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
		a = a + 2;
	}

	return 0;
}


