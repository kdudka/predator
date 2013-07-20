/**
* @file test-0075.c
*
* @brief Motivation program from the master thesis with buffer overflow.
*/

#include <stdio.h>

int main(int argc, const char *argv[])
{
	int importantData = 1;
	int buffer[10];

	int i;
	for (i = 0; i <= 10; i++) {
		buffer[i] = 9999;
	}

	printf("importantData = %d\n", importantData);

	return 0;
}
