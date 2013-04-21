/**
* @file test-0059.c
*
* @brief Power of two.
*/

#define N 5

int main(void) {
  int i;
  int value = 1;

  for (i = 0; i < N; i++) {
    value = 2 * value;
  }

  return 0;
}

