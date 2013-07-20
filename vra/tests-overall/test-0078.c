/**
* @file test-0078.c
*
* @brief Prints the multiplication table.
*/

#include <stdio.h>

#define NUM 10

int main(int argc, const char *argv[]) {
	int i, j;
	int mult;

	for(i = 1; i <= NUM; i++){
		for(j = 1; j<= NUM; j++) {
			mult = i * j;
			printf("%d * %d = %d ", i, j, i*j);
		}
		printf("\n");
	}

	return 0;
}
