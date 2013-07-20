/**
* @file test-0056.c
*
* @brief Factorial computation.
*/

int main(int argc, const char *argv[]) {
	int i;
	int fact[6];

	fact[0] = 1;
	for (i = 1; i < 6; ++i){
		fact[i] = fact[i-1] * i;
	}

	return 0;
}

