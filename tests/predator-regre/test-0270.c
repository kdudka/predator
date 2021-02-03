#include <stdlib.h>
#include <stddef.h>

int main(void) {

	ptrdiff_t qp, pq;
	int *p;
	{
		int array[10];
		p = &array[0];
		int* q = &array[5];

		qp = q - p;  // Valid. diff1 is 5

		pq = p - q;  // Valid. diff2 is -5
	}

	if (qp == 5 && pq == -5) {
		return 5;
	}

	return *p;

}

/**
 * @file test-0270.c
 *
 * @brief a regression test for pointer subtraction
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
