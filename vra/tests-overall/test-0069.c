/**
* @file test-0069.c
*
* @brief Using global variables.
*/

#define N 2

int modif[N] = {0, 1};
int nonModif[N] = {2, 3};

int main(int argc, const char *argv[]) {

	int arrModif[N];
	int i;
	for (i = 0; i < N; ++i) {
		arrModif[i] = modif[i];
	}

	int arrNonModif[N];
	for (i = 0; i < N; ++i) {
		arrNonModif[i] = nonModif[i];
	}

	modif[0] = 5;

	return 0;
}
