/**
* @file test-0080.c
*
* @brief Converts upper case letter to lower case one.
*/

#include <stdio.h>

#define NUM 10

int main(){
	char str[NUM] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
	char strNew[NUM];
	int i;

	printf("The string is: %s",str);

	for(i = 0; i < NUM; i++) {
		if ( str[i] >= 65 && str[i] <= 90) {
			strNew[i]= str[i] + 32;
		}
	}

	printf("\nThe string in uppercase is: %s",str);

	return 0;
}


