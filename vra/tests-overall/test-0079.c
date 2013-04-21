/**
* @file test-0079.c
*
* @brief Converts lower case letter to upper case one.
*/

#include <stdio.h>

#define NUM 10

int main(){
	char str[NUM] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
	char strNew[NUM];
	int i;

	printf("The string is: %s",str);

	for(i = 0; i < NUM; i++) {
		if ( str[i] >= 97 && str[i] <= 122) {
			strNew[i]= str[i] - 32;
		}
	}

	printf("\nThe string in uppercase is: %s",str);

	return 0;
}

