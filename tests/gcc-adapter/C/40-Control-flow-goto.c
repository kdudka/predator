/* Test case: #40 */

int main(void)
{
  unsigned i, j, k, result;

start:
  for (i = 0; i < 10; i++) {
    for (j = 0; j < 20; j++) {
      for (k = 0; k < 30; k++) {
        result = i * j * k;

        if ((result % 13) == 0)
          goto end_of_fors;
      }
    }
  }
  

end_of_fors:
  if ((i + j + k) % 2 == 1)
    goto start;

  return 0;
}
