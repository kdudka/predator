/* Test case: #37 */

int main(void)
{
  unsigned i, j = 0;

  for (i = 0; i < 15; i++)
    j++;

  for ( ; i < j; ) {
    i++;
    j--;
  }

  for (i = 0; i < 15; ) {
    i++;
    j++;
  }

  for ( ; i < j; i++) {
    j--;
  }

  return 0;
}
