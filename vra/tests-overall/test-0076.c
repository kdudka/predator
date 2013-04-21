/**
* @file test-0076.c
*
* @brief Prints a table for Fahrenheit to Celsius from 0F to 300F.
*        From http://nob.cs.ucdavis.edu/classes/ecs030-2002-02/handouts/samprog.html
*/

#define LOWER 0
#define UPPER 300
#define STEP 20

#include <stdio.h>

int main(int argc, const char *argv[]) {
	int fahr;
	int celsius;

	fahr = 0;
	while(fahr <= UPPER){
		celsius = 5 * (fahr - 32) / 9;
		printf("%d\t%d\n", fahr, celsius);
		fahr += STEP;
	}

	return 0;
}

