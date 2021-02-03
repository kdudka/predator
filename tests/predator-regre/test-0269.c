#include <stdlib.h>
#include <stddef.h>

int main(void) {

	int *array = malloc(sizeof(int)*10);
	int *array2 = malloc(sizeof(int)*10);
	int* p = array;
	int* q = array2 + 5;

	ptrdiff_t qp = q - p;  // Invalid. diff1

	ptrdiff_t pq = p - q;  // Invalid. diff2

	if (qp == 5 && pq == -5) {
		free(array);
		free(array2);
	}

	return 0;

}

/**
 * @file test-0269.c
 *
 * @brief a regression test for pointer subtraction
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
