/**
* @file test-0054.c
*
* @brief Simple loop: Uninitialized loop variable.
*/

int main(int argc, const char *argv[]) {

	int i;
	while (i < 10) {
		++i;
	}

	return 0;
}


