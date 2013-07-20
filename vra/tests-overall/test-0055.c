/**
* @file test-0055.c
*
* @brief Buffer overflow.
*/

int main(int argc, const char *argv[]) {
	int buffer[10];

	int i = 0;
	while (i <= 10) {
		buffer[i] = 9999;
		++i;
	}

	return 0;
}

