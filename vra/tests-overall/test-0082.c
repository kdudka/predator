/**
* @file test-0082.c
*
* @brief Power of two for an array.
*/

#include<stdio.h>
#include<math.h>

#define NUM 10

int main(){
    int array[NUM] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int result[NUM];

	int i;
	for (i = 0; i < NUM; ++i) {
		int j = array[i];
		result[i] = j * j;
	}

    return 0;
}
