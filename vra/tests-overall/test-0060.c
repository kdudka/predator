/**
* @file test-0060.c
*
* @brief Adding of all numbers stored in an array.
*/

#define N 10

int main(void) {
  int array[N] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int sum = 0;
  int i = 0;

  while (i < N) {
      sum = sum + array[i];
      ++i;
  }

	return 0;
}

