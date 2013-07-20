/**
* @file test-0063.c
*
* @brief Factorial with double type.
*/

int main(int argc, const char *argv[]) {
	int i;
	double fact[6];

	fact[0] = 1.0;
	for (i = 1; i < 6; ++i){
		fact[i] = fact[i-1] * i;
	}

	return 0;
}
