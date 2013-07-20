/**
* @file test-0081.c
*
* @brief Finds the minimum element of the array.
*/

#include <stdio.h>

#define NUM 10

int main(int argc, const char *argv[]) {
    int array[NUM] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	int i;
	int min = array[0];
	for (i = 1; i < NUM; ++i) {
		if (array[i] < min) {
			min = array[i];
		}
	}

    return 0;
}

